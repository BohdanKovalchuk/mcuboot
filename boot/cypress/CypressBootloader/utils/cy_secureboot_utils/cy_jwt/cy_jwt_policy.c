/***************************************************************************//**
* \file cy_jwt_policy.c
* \version 1.0
*
* \brief
*  This is the source code for the JWT policy parsing and processing.
*
*****************************************?***************************************
* \copyright
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

#include "cy_syslib.h"
#include "cy_ipc_drv.h"

#include "mbedtls/ecdsa.h"
#include "mbedtls/oid.h"
#include "mbedtls/asn1.h"
#include "mbedtls/pk_internal.h"

#include "base64/base64.h"

#include "cy_jwt_policy.h"

/***************************************
* Macro definitions
***************************************/
#define CY_JWT_HASH_SIZE 32

#define CY_JWT_EC_KEY_CRV_FOUND     (1)
#define CY_JWT_EC_KEY_X_FOUND       (2)
#define CY_JWT_EC_KEY_Y_FOUND       (4)

#define JSON_MAX_DEPTH  (15)

/** Provisioning details SysCall opcode */
#define PROVDETAIL_SYSCALL_OPCODE       (0x37UL << 24UL)

/** PSA crypto SysCall return codes */
#define CY_FB_SYSCALL_SUCCESS           (0xA0000000UL)

/** SysCall timeout value */
#define SYSCALL_TIMEOUT                 (15000UL)

/* Uncomment to print validation debug messages */
/* #define JSON_SCHEMA_DBG */

/***************************************
* Data Structure definitions
***************************************/

typedef enum cy_jwt_validation_error_codes
{
    CY_SCHEMA_VALID = 0,
    CY_SCHEMA_WRONG_TYPE = -1,
    CY_SCHEMA_MORE_THAN_MAX = -2,
    CY_SCHEMA_LESS_THAN_MIN = -3,
    CY_SCHEMA_WRONG_FORMAT = -4
} cy_jwt_validation_error_codes_t;

/** Region names used in policy, must be in conformity with cy_jwt_smpu_reg_t enum */
const char *mem_region_names[POLICY_MAX_N_OF_PROT_REGIONS] =
{
    "FLASH_PC1_SPM", "FLASH_PC2", "FLASH_PC3", "FLASH_PC4",
    "FLASH_MAIN_CODE", "FLASH_MAIN_XO","FLASH_MAIN_DATA", "FLASH_SUPERVISORY",
    "FLASH_WORK_SECURE", "FLASH_WORK", "SRAM_PC0_PRIV", "SRAM_PC0_PUB",
    "SRAM_SPM_PRIV", "SRAM_SPM_PUB", "SRAM_PC1_PRIV", "SRAM_PC2_PRIV",
    "SRAM_PC3_PRIV", "SRAM_PC4_PRIV", "SRAM_MAIN", "SRAM_DAP", "SMIF_CODE",
    "SMIF_XO", "SMIF_DATA"
};

/*******************************************************************************
* Function Name: get_str_key_crv
****************************************************************************//**
* Converts string with EC key curve name to enum value.
*
* \param curve    The pointer to string defining curve type.
* \return Integer value corresponding to curve defined.
*******************************************************************************/
ec_curve_type Cy_JWT_GetStrKeyCrv(char *curve)
{
    ec_curve_type rc = EC_KEY_CRV_INV;

    if (curve != NULL)
    {
        if (0 == strcasecmp(curve, "P-256"))
        {
            rc = EC_CRV_P_256;
        }
/*        else
        if (!strcasecmp(alg, "another_name"))
        {
            rc = ANOTHER_ENUM_VALUE;
        }
*/
    }
    return rc;
}

/*******************************************************************************
* Function Name: get_b64_decode_len
****************************************************************************//**
* Function Calculates the buffer size required for decoded base64 string storage.
*
* \param base64_size Size of base64 string to be decoded.
* \return Returns required buffer size to store decoded Base64 string.
*******************************************************************************/
uint32_t Cy_JWT_GetB64DecodeLen(uint32_t base64_size)
{
    uint32_t out_len, rem;

    out_len = (base64_size*3);
    out_len /= 4;

    rem = (4-out_len%4);

    out_len = out_len + rem;

    return out_len;
}

/*******************************************************************************
* Function Name: get_b64_encode_len
****************************************************************************//**
* Calculates the buffer size required for string encoded into base64.
*
* \param raw_size Size of string to be encoded.
* \return Returns required buffer size to store encoded Base64 string.
*******************************************************************************/
uint32_t Cy_JWT_GetB64EncodeLen(uint32_t raw_size)
{
    uint32_t out_len;

    out_len = 4uL*raw_size;
    out_len = out_len/3uL;
    out_len = out_len+3uL;
    out_len = out_len & ~3uL;

    return out_len;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseEcKey
****************************************************************************//**
* Parses JWK which represents EC key.
*
* \param json   JSON object with a key.
* \param key    Composite type return variable where key will be stored.
* \return       Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ParseEcKey(cJSON *json, ec_key_t *key)
{
    int rc = CY_JWT_ERR_JKEY_NOT_FOUND;
    uint32_t flags = 0;

    if(cJSON_Object == json->type)
    {
        cJSON *item = json->child;

        /* reset d-parameter in case we have private key */
        memset(key->d, 0, sizeof(key->d));

        while(NULL != item)
        {
            if((NULL != item->string) && (cJSON_String == item->type))
            {
                if(0 == strcmp("crv", item->string))
                {
                    key->crv = Cy_JWT_GetStrKeyCrv(item->valuestring);
                    if(EC_KEY_CRV_INV != key->crv)
                    {
                        flags |= CY_JWT_EC_KEY_CRV_FOUND;
                    }
                    else
                    {   /* report unsupported curve early */
                        rc = CY_JWT_ERR_EC_KEY_CRV_INV;
                        break;
                    }
                }
                else
                if(0 == strcmp("d", item->string))
                {
                    rc = base64_decode((uint8_t*)item->valuestring, -1, (uint8_t*)key->d, KEY_XY_LENGTH, BASE64_URL_SAFE_CHARSET);
                    if(rc <= 0)
                    {   /* report Decode64 failure early */
                        break;
                    }
                    else
                    {
                        rc = CY_JWT_SUCCESS;
                    }
                }
                else
                if(0 == strcmp("x", item->string))
                {
                    rc = base64_decode((uint8_t*)item->valuestring, -1, (uint8_t*)key->x, KEY_XY_LENGTH, BASE64_URL_SAFE_CHARSET);
                    if(rc > 0)
                    {
                        flags |= CY_JWT_EC_KEY_X_FOUND;
                        rc = CY_JWT_SUCCESS;
                    }
                    else
                    {   /* report Decode64 failure early */
                        break;
                    }
                }
                else
                if(0 == strcmp("y", item->string))
                {
                    rc = base64_decode((uint8_t*)item->valuestring, -1, (uint8_t*)key->y, KEY_XY_LENGTH, BASE64_URL_SAFE_CHARSET);
                    if(rc > 0)
                    {
                        flags |= CY_JWT_EC_KEY_Y_FOUND;
                        rc = CY_JWT_SUCCESS;
                    }
                    else
                    {   /* report Decode64 failure early */
                        break;
                    }
                }
            }
            item = item->next;
        }

        if((CY_JWT_EC_KEY_CRV_FOUND|CY_JWT_EC_KEY_X_FOUND|CY_JWT_EC_KEY_Y_FOUND) != flags)
        {
            rc = CY_JWT_ERR_EC_KEY_NON_COMPLT;
        }
        else
        {
            rc = CY_JWT_SUCCESS;
        }
    }
    else
    {
        rc = CY_JWT_ERR_JSN_NONOBJ;
    }
    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseStrByKey
****************************************************************************//**
* Looks for JSON item by key string. Operates only on one level of nesting.
* Expects input item to be object.
*
* \param json       JSON object for lookup.
* \param val_str    String-type return variable result of lookup will be stored.
* \param key_str    Key string to lookup value by it.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ParseStrByKey(cJSON *json, char *val_str, char *key_str)
{
    int rc = CY_JWT_ERR_JKEY_NOT_FOUND;

    if(cJSON_Object == json->type)
    {
        cJSON *item = json->child;
        while(NULL != item)
        {
            if((NULL != item->string) && (cJSON_String == item->type))
            {
                if(0 == strcmp(key_str, item->string))
                {
                    if((cJSON_String == item->type) && (NULL != item->valuestring))
                    {
                        strcpy(val_str, item->valuestring);
                        rc = CY_JWT_SUCCESS;
                        break;
                    }
                }
            }
            item = item->next;
        }
    }
    else
    {
        rc = CY_JWT_ERR_JSN_NONOBJ;
    }
    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseHeaderAlg
****************************************************************************//**
* Looks for "alg" value inside given JSON (JWT packet header).
*
* \param json       JSON object for lookup.
* \param alg_str    Key string to lookup crypto key type by it.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ParseHeaderAlg(cJSON *json, char *alg_str)
{
    int rc = CY_JWT_ERR_JKEY_NOT_FOUND;

    rc = Cy_JWT_ParseStrByKey(json, alg_str, "alg");

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseKeyKty
****************************************************************************//**
* Looks for "kty" value inside given JSON (JWT packet header).
*
* \param json       JSON object for lookup.
* \param kty_str    Key string to lookup by it.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ParseKeyKty(cJSON *json, char *kty_str)
{
    int rc = CY_JWT_ERR_JKEY_NOT_FOUND;

    rc = Cy_JWT_ParseStrByKey(json, kty_str, "kty");

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_SplitParsedPolicy
****************************************************************************//**
* Searches for 3 sub-policies inside already parsed JSON-policy and
* saves pointers to them.
*
* \param policy       Policy JSON object for lookup.
* \param policy_set   Composite variable where pointers of corresponing sub-polices
                      will be stored.
* \return             Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_PolicyParsedSplit(cJSON *policy, policy_set_t *policy_set)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *item;

    policy_set->root = policy;

    if(cJSON_Object == policy->type)
    {
        item = policy->child;
        while(NULL != item)
        {
            if(NULL != item->string)
            {
                if(0 == strcmp("debug", item->string))
                {
                    policy_set->debug = item;
                }
                else if(0 == strcmp("wounding", item->string))
                {
                    policy_set->wounding = item;
                }
                else if(0 == strcmp("boot_upgrade", item->string))
                {
                    policy_set->boot = item;
                }
            }

            item = item->next;
        }
    }
    else
    {
        rc = CY_JWT_ERR_JSN_NONOBJ;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_IsJSONItemSimple
****************************************************************************//**
* Checks if JSON item has a simple type - number, boolean, string.
*
* \param item     JSON object to check.
* \return Trivial TRUE/FALSE.
*******************************************************************************/
bool Cy_JWT_IsJsonItemSimple(cJSON *item)
{
    return ((cJSON_False == item->type) || (cJSON_True == item->type)
            || (cJSON_Number == item->type) || (cJSON_String == item->type)
            || (cJSON_NULL == item->type));
}

/*******************************************************************************
* Function Name: Cy_JWT_FindJSONItemByKey
****************************************************************************//**
* Finds item by key (name) in whole JSON.
* Key can be not unique so function returns first item with this key.
*
* \param key      String key to lookup after.
* \param json     JSON object to check.
* \return         Returns pointer to JSON object found.
*******************************************************************************/
cJSON *Cy_JWT_FindJsonItemByKey(const char* key, cJSON *json)
{
    cJSON *item = NULL;
    cJSON *parent[JSON_MAX_DEPTH];
    uint32_t depth = 0;

    while(NULL != json)
    {
        if(NULL != json->string && 0 == strcmp(key, json->string))
        {
            item = json;
            break;
        }

        if(NULL != json->child)
        {
            if(depth < JSON_MAX_DEPTH)
            {
                parent[depth++] = json;
                json = json->child;
            }
            else
            {
                break;
            }
        }
        else if(NULL != json->next)
        {
            json = json->next;
        }
        else if(0 == depth)
        {
            break;
        }
        else
        {
            while(depth > 0)
            {
                json = parent[--depth]->next;
                if(NULL != json)
                {
                    break;
                }
            }
        }
    }

    return item;
}

/*******************************************************************************
* Function Name: Cy_JWT_ValidateItemBySchema
****************************************************************************//**
* Validates single JSON item by looking for its constraints in schema.
*
* \param item       Pointer to policy item in JSON format.
* \param parent     Pointer to parent of policy item in JSON format.
* \param schema     Pointer to policy template in JSON format.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ValidateItemBySchema(cJSON *item, cJSON *parent, cJSON *schema)
{
    int rc = CY_SCHEMA_VALID;
    cJSON *parent_sch = NULL;
    cJSON *item_sch = NULL;
    cJSON *val_kwd = NULL;

    if(NULL != parent)
    {
        parent_sch = Cy_JWT_FindJsonItemByKey(parent->string, schema);
    }
    else
    {
        parent_sch = schema;
    }

    if(NULL != item->string)
    {
        item_sch = Cy_JWT_FindJsonItemByKey(item->string, parent_sch);
    }
    else
    {
        item_sch = parent_sch;
    }

    if((NULL != item_sch) && (cJSON_Object == item_sch->type))
    {
        val_kwd = item_sch->child;
        while(NULL != val_kwd)
        {
            if(0 == strcmp("type", val_kwd->string))
            {
                if(0 == strcmp("array", val_kwd->valuestring))
                {
                    val_kwd = Cy_JWT_FindJsonItemByKey("items", item_sch);
                    val_kwd = val_kwd->child;
                }
                else
                {
                    if(0 == strcmp("number", val_kwd->valuestring) &&
                            cJSON_Number != item->type)
                    {
                        rc = CY_SCHEMA_WRONG_TYPE;
                    }
                    else if(0 == strcmp("integer", val_kwd->valuestring) &&
                            cJSON_Number != item->type)
                    {
                        rc = CY_SCHEMA_WRONG_TYPE;
                    }
                    else if(0 == strcmp("boolean", val_kwd->valuestring) &&
                            (cJSON_False != item->type && cJSON_True != item->type))
                    {
                        rc = CY_SCHEMA_WRONG_TYPE;
                    }
                    else if(0 == strcmp("string", val_kwd->valuestring) &&
                            cJSON_String != item->type)
                    {
                        rc = CY_SCHEMA_WRONG_TYPE;
                    }

                    val_kwd = val_kwd->next;
                }
            }
            else if(0 == strcmp("maximum", val_kwd->string))
            {
                if(item->valueint > val_kwd->valueint)
                {
                    rc = CY_SCHEMA_MORE_THAN_MAX;
                }

                val_kwd = val_kwd->next;
            }
            else if(0 == strcmp("minimum", val_kwd->string))
            {
                if(item->valueint < val_kwd->valueint)
                {
                    rc = CY_SCHEMA_LESS_THAN_MIN;
                }

                val_kwd = val_kwd->next;
            }
            else if(0 == strcmp("oneOf", val_kwd->string))
            {
                val_kwd = val_kwd->next;
            }
            else if(0 == strcmp("allOf", val_kwd->string))
            {
                val_kwd = val_kwd->next;
            }
            else
            {
                val_kwd = val_kwd->next;
            }

            if(CY_SCHEMA_VALID != rc)
            {
                break;
            }
        }
    }
    else
    {
        rc = CY_SCHEMA_WRONG_FORMAT;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_ValidateSinglePolicyBySchema
****************************************************************************//**
* Validates JSON-policy using correspondent JSON-schema.
*
* \param json       JSON object to be validated.
* \param schema     Schema JSON to as a validation template.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_PolicySingleValidateBySchema(cJSON *json, cJSON *schema)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *item;
    cJSON *parents[JSON_MAX_DEPTH];
    uint32_t depth = 0;

    if(cJSON_Object == json->type)
    {
        item = json->child;
        while(NULL != item)
        {
            if(Cy_JWT_IsJsonItemSimple(item))
            {
                /* Validate item */
                cJSON *parent;
                if(0 == depth)
                {
                    parent = NULL;
                }
                else if(cJSON_Object == parents[depth-1]->type && NULL == parents[depth-1]->string)
                {
                    parent = parents[depth-2];
                }
                else
                {
                    parent = parents[depth-1];
                }

                rc = Cy_JWT_ValidateItemBySchema(item, parent, schema);
                if(CY_SCHEMA_VALID != rc)
                {
#if !defined(JSON_SCHEMA_DBG)
                    rc = CY_JWT_ERR_TEMPL_VALID_FAIL;
                    break;
#endif
                }

#if defined(JSON_SCHEMA_DBG)
                char *value = cJSON_Print(item);
                printf("%s : %s = %i\r\n", 0 == item->string ? "*" : item->string, value, rc);
                free(value);
#endif
            }

            if(NULL != item->child)
            {
                if(depth < JSON_MAX_DEPTH)
                {
                    parents[depth++] = item;
                    item = item->child;
                }
                else
                {
                    rc = CY_JWT_ERR_JSN_BIG_NESTING;
                    break;
                }
            }
            else if(NULL != item->next)
            {
                item = item->next;
            }
            else if(0 == depth)
            {
                break;
            }
            else
            {
                while(depth > 0)
                {
                    item = parents[--depth]->next;
                    if(NULL != item)
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        rc = CY_JWT_ERR_JSN_NONOBJ;
    }

#if defined(JSON_SCHEMA_DBG)
    printf("\r\n");
#endif

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetHeadStrData
****************************************************************************//**
* Looks over given JWT packet, find where JWT header is located
* and what length of it is.
*
* \param jwt_str    JWT packet to analyze.
* \param header_ptr Pointer to JWT header section.
* \param header_len Length of JWT header section.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_GetStrDataHead(char *jwt_str, char **header_ptr, uint32_t *header_len)
{
    int ret = CY_JWT_SUCCESS;
    char *ptr;

    ptr = strchr(jwt_str, '.');

    if(NULL != ptr)
    {
        *header_len = (uint32_t)(ptr-jwt_str);
        *header_ptr = jwt_str;
    }
    else
    {
        ret = CY_JWT_ERR_JWT_BROKEN_FORMAT;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetBodyStrData
****************************************************************************//**
* Looks over given JWT packet, find where JWT body is located
* and what length of it is.
*
* \param jwt_str  JWT packet to analyze.
* \param body_ptr Pointer to JWT body section.
* \param body_len Length of JWT header section.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_GetStrDataBody(char *jwt_str, char **body_ptr, uint32_t *body_len)
{
    int ret = CY_JWT_SUCCESS;
    char *ptrStart;
    char *ptrEnd;

    ptrStart = strchr(jwt_str, '.');
    if(NULL != ptrStart)
    {
        ptrEnd = strchr(ptrStart+1, '.');
        if(NULL != ptrEnd)
        {
            *body_len = (uint32_t)(ptrEnd-ptrStart)-1U;
            *body_ptr = ptrStart+1;
        }
        else
        {
            ret = CY_JWT_ERR_JWT_BROKEN_FORMAT;
        }
    }
    else
    {
        ret = CY_JWT_ERR_JWT_BROKEN_FORMAT;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetSigStrData
****************************************************************************//**
* Looks over given JWT packet, find where JWT signature is located
* and what length of it is.
*
* \param jwt_str  JWT packet to analyze.
* \param sig_ptr Pointer to JWT signature section.
* \param sig_len Length of JWT signature section.
* \return           Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_GetStrDataSig(char *jwt_str, char **sig_ptr, uint32_t *sig_len)
{
    int ret = CY_JWT_SUCCESS;
    char *ptrStart;

    ptrStart = strchr(jwt_str, '.');
    if(NULL != ptrStart)
    {
        ptrStart = strchr(ptrStart+1, '.');
        if(NULL != ptrStart)
        {
            *sig_len = strlen(ptrStart+1);
            *sig_ptr = ptrStart+1;
        }
        else
        {
            ret = CY_JWT_ERR_JWT_BROKEN_FORMAT;
        }
    }
    else
    {
        ret = CY_JWT_ERR_JWT_BROKEN_FORMAT;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_JWT_CheckHeadValid
****************************************************************************//**
* Checks if given JWT packet has valid header.
*
* \param jwt_packet   JWT packet where header has to be validated.
* \param alg          Expected algorithm.
* \return             Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_IsHeadValid(char *jwt_packet, char *alg)
{
    int ret = CY_JWT_SUCCESS;

    char *json_payload;
    char *ptr;

    uint32_t payload_len;
    uint32_t json_len;

    /* look for <head.body> */
    ret = Cy_JWT_GetStrDataHead(jwt_packet, &ptr, &payload_len);

    if (CY_JWT_SUCCESS == ret)
    {
        /* estimate decode64 output buffer size */
        json_len = Cy_JWT_GetB64DecodeLen(payload_len);
        json_payload = malloc(json_len);

        if(NULL != json_payload)
        {
            ret = base64_decode((uint8_t*)ptr, payload_len, (uint8_t*)json_payload, json_len, BASE64_URL_SAFE_CHARSET);
            /* if head decoded-64 successfully */
            if(ret > 0)
            {
                cJSON_InitHooks(NULL);
                cJSON *json_data = cJSON_Parse(json_payload);

                ret = Cy_JWT_ParseHeaderAlg(json_data, alg);

                cJSON_Delete(json_data);
            }
            else
            {
                ret = CY_JWT_ERR_B64DECODE_FAIL;
            }
            free(json_payload);
        }
        else
        {
            ret = CY_JWT_ERR_JWT_MALLOC_FAIL;
        }
    }
    else
    {
        ret = CY_JWT_ERR_JWT_PACKET_PARSE;
    }
    return ret;
}

/*******************************************************************************
* Function Name: Cy_JWT_ValidateBySchema
****************************************************************************//**
* Validates JSON-policy which consist of 3 sub-policies using set of 3 JSON-schemas.
*
* \param policy_str  Pointer to string that contains policy.
* \param templates   Pointer to structure that contains policy template strings.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_PolicyValidateBySchema(char *policy_str, schema_set_t *templates)
{
    int rc = CY_JWT_SUCCESS;
    policy_set_t policy_set;
    cJSON *policy = NULL;
    cJSON *schema = NULL;

    cJSON_InitHooks(NULL);

    policy = cJSON_Parse(policy_str);
    if(NULL != policy)
    {
        rc = Cy_JWT_PolicyParsedSplit(policy, &policy_set);

        if(CY_JWT_SUCCESS == rc)
        {
            schema = cJSON_Parse(templates->debug);
            if(NULL != schema)
            {
                rc = Cy_JWT_PolicySingleValidateBySchema(policy_set.debug, schema);
                cJSON_Delete(schema);
            }
            else
            {
                rc = CY_JWT_ERR_JSN_PARSE_FAIL;
            }
        }

        if(CY_JWT_SUCCESS == rc)
        {
            schema = cJSON_Parse(templates->wounding);
            if(NULL != schema)
            {
                rc = Cy_JWT_PolicySingleValidateBySchema(policy_set.wounding, schema);
                cJSON_Delete(schema);
            }
            else
            {
                rc = CY_JWT_ERR_JSN_PARSE_FAIL;
            }
        }

        if(CY_JWT_SUCCESS == rc)
        {
            schema = cJSON_Parse(templates->boot);
            if(NULL != schema)
            {
                rc = Cy_JWT_PolicySingleValidateBySchema(policy_set.boot, schema);
                cJSON_Delete(schema);
            }
            else
            {
                rc = CY_JWT_ERR_JSN_PARSE_FAIL;
            }
        }

        cJSON_Delete(policy);
    }
    else
    {
        rc = CY_JWT_ERR_JSN_PARSE_FAIL;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_SplitPolicy
****************************************************************************//**
* Takes whole JSON-policy string and returns pointers to 3 parsed sub-policies.
* Note: policy_set.root must be freed after.
*
* USAGE MODEL:
*
* Cy_JWT_SplitPolicy(&policy_set);
* ...
* some_useful_code(&policy_set);
* ...
* cJSON_Delete(policy_set.root);
*
* \param policy_str  Pointer to string that contains policy.
* \param policy_set  Pointer to structure that contains parsed policy.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_PolicyPackageSplit(char *policy_str, policy_set_t *policy_set)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *policy = NULL;

    cJSON_InitHooks(NULL);

    policy = cJSON_Parse(policy_str);
    if(NULL != policy)
    {
        rc = Cy_JWT_PolicyParsedSplit(policy, policy_set);
    }
    else
    {
        rc = CY_JWT_ERR_JSN_PARSE_FAIL;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetProtRegionCode
****************************************************************************//**
* Helper function, analyzes protection region name and returns its numerical
* representation.
*
* \param name         Name of protected region.
* \param code         Numerical representation of protected region.
* \return             Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
static int Cy_JWT_GetProtRegionCode(const char* name, int *code)
{
    int rc = CY_JWT_SUCCESS;
    int i;

    if((NULL == name) || (NULL == code))
    {
        rc = CY_JWT_ERR_JSN_PARSE_FAIL;
    }
    else
    {
        for(i = 0; i < POLICY_MAX_N_OF_PROT_REGIONS; i++)
        {
            if(0 == strcmp(name, mem_region_names[i]))
            {
                *code = i;
                break;
            }
        }

        if(POLICY_MAX_N_OF_PROT_REGIONS == i)
        {
            rc = CY_JWT_ERR_JSN_PARSE_FAIL;
        }
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetProtRegionSize
****************************************************************************//**
* Helper function, analyzes protection region size in bytes and returns
* its code suitable to use with Cypress protection unit structure.
*
* \param  size        Protection region size in bytes.
* \return code        Size code for protection unit structure.
*******************************************************************************/
int Cy_JWT_GetProtRegionSize(uint64_t size)
{
    int code = -1;
    uint32_t limit = 4; /* 4 bytes is lower possible memory size to protect */
    int i;
    const int max_shift = 29; /* 4 << 29 = 0x80000000 */

    for(i = 0; i < max_shift; i++)
    {
        if(size > limit)
        {
            limit <<= 1;
        }
        else
        {
            code = i+1;
            break;
        }
    }

    if(max_shift == i)
    {
        code = i+2;
    }

    return code;
}

/*******************************************************************************
* Function Name: Cy_JWT_ParseBnUPolicyResources
****************************************************************************//**
* Takes list of images configuration from parsed BnU policy and
* looks for used resources to configure protections.
*
* \param bnu_json    Pointer to parsed policy.
* \param bnu_policy  Pointer to structure that contains policy in binary
*                    representation.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
static int Cy_JWT_ParseBnUPolicyResources(cJSON *bnu_json, bnu_policy_t *bnu_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *image = bnu_json;
    cJSON *item = NULL;
    cJSON *res = NULL;
    int region = 0;
    int size = 0;
    uint32_t address = 0;

    while(NULL != image)
    {
        item = Cy_JWT_FindJsonItemByKey("resources", image);
        if(NULL != item)
        {
            res = item->child;
            while(NULL != res)
            {
                item = Cy_JWT_FindJsonItemByKey("type", res);
                if(NULL != item)
                {
                    /* if resources are NOT flashmap-related */
                    if((0 != strcmp(item->valuestring, "BOOT"))&&
                        (0 != strcmp(item->valuestring, "UPGRADE")))
                    {
                        rc = Cy_JWT_GetProtRegionCode(item->valuestring, &region);
                        if(0 != rc)
                        {
                            region = -1;
                        }

                        item = Cy_JWT_FindJsonItemByKey("address", res);
                        if(NULL != item)
                        {
                            address = (uint32_t)item->valueint;
                        }

                        item = Cy_JWT_FindJsonItemByKey("size", res);
                        if(NULL != item)
                        {
                            size = Cy_JWT_GetProtRegionSize((uint64_t)item->valueint);
                        }

                        if((region >= 0) && (size > 0))
                        {
                            if(1U == bnu_policy->present.prot_regions[region].start)
                            {
                                bnu_policy->prot_regions[region].start = (uint32_t)address;
                            }

                            if(1U == bnu_policy->present.prot_regions[region].size)
                            {
                                bnu_policy->prot_regions[region].size = (uint32_t)size;
                            }
                        }
                    }
                }
                res = res->next;
            }
        }
        image = image->next;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_ParseBnUPolicyFlashMap
****************************************************************************//**
* Takes list of images configuration from parsed BnU policy and
* looks for flash map configuration.
*
* \param bnu_json    Pointer to parsed policy.
* \param bnu_policy  Pointer to structure that contains policy in binary
*                    representation.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
static int Cy_JWT_ParseBnUPolicyFlashMap(cJSON *bnu_json, bnu_policy_t *bnu_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *image = bnu_json;
    cJSON *item = NULL;
    cJSON *res = NULL;

    if(NULL != image)
    {
        item = Cy_JWT_FindJsonItemByKey("resources", image);
        if(NULL != item)
        {
            res = item->child;
            while(NULL != res)
            {
                item = Cy_JWT_FindJsonItemByKey("type", res);
                if(NULL != item)
                {
                    /* if resource is FlashMap-related */
                    if(0 == strcmp(item->valuestring, "BOOT"))
                    {
                        item = Cy_JWT_FindJsonItemByKey("address", res);
                        if(NULL != item)
                        {
                            if(1U == bnu_policy->present.bnu_img_policy.boot_area.start)
                            {
                                bnu_policy->bnu_img_policy.boot_area.start = (uint32_t)item->valueint;
                            }
                        }

                        item = Cy_JWT_FindJsonItemByKey("size", res);
                        if(NULL != item)
                        {
                            if(1U == bnu_policy->present.bnu_img_policy.boot_area.size)
                            {
                                bnu_policy->bnu_img_policy.boot_area.size = (uint32_t)item->valueint;
                            }
                        }
                    }
                    else /* if resource is FlashMap-related */
                    if(0 == strcmp(item->valuestring, "UPGRADE"))
                    {
                        item = Cy_JWT_FindJsonItemByKey("address", res);
                        if(NULL != item)
                        {
                            if(1U == bnu_policy->present.bnu_img_policy.upgrade_area.start)
                            {
                                bnu_policy->bnu_img_policy.upgrade_area.start = (uint32_t)item->valueint;
                            }
                        }

                        item = Cy_JWT_FindJsonItemByKey("size", res);
                        if(NULL != item)
                        {
                            if(1U == bnu_policy->present.bnu_img_policy.upgrade_area.start)
                            {
                                bnu_policy->bnu_img_policy.upgrade_area.size = (uint32_t)item->valueint;
                            }
                        }
                    }
                }
                res = res->next;
            }
        }
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseBnUPolicy
****************************************************************************//**
* Takes parsed BnU policy, looks for items and stores them into structure.
* Assumes that image ID is already in BnU structure.
*
* \param bnu_json    Pointer to parsed policy.
* \param bnu_policy  Pointer to structure that contains policy in binary
*                    representation.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_BnUPolicyParse(cJSON *bnu_json, bnu_policy_t *bnu_policy, uint32_t masterImgID)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *policy = NULL;
    cJSON *policyRec = NULL;
    cJSON *item = NULL;
    uint8_t id = 0;

    /* Analyze 'image' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("firmware", bnu_json);
    if(NULL != policy)
    {
        policy = policy->child;

        rc = Cy_JWT_ParseBnUPolicyResources(policy, bnu_policy);

        policyRec = policy;
        /* Looking for master image ID */
        if(cJSON_Object == policy->type)
        {
            while(NULL != policy)
            {
                item = Cy_JWT_FindJsonItemByKey("id", policy);
                if(NULL != item)
                {
                    id = (uint8_t)item->valueint;
                    if(id == masterImgID)
                    {
                        item = Cy_JWT_FindJsonItemByKey("launch", policy);
                        bnu_policy->bnu_img_policy.id = (uint8_t)item->valueint;
                        break;
                    }
                }
                policy = policy->next;
            }
        }
        /* return to the beginning */
        policy = policyRec;
        if(cJSON_Object == policy->type)
        {
            while(NULL != policy)
            {
                item = Cy_JWT_FindJsonItemByKey("id", policy);
                if(NULL != item)
                {
                    id = (uint8_t)item->valueint;
                    if(id == bnu_policy->bnu_img_policy.id)
                    {
                        break;
                    }
                }
                policy = policy->next;
            }

            rc = Cy_JWT_ParseBnUPolicyFlashMap(policy, bnu_policy);

            item = Cy_JWT_FindJsonItemByKey("monotonic", policy);
            if(NULL != item)
            {
                if(1U == bnu_policy->present.bnu_img_policy.monotonic)
                {
                    bnu_policy->bnu_img_policy.monotonic = (uint8_t)item->valueint;
                }
            }

            item = Cy_JWT_FindJsonItemByKey("upgrade", policy);
            if(NULL != item)
            {
                if(1U == bnu_policy->present.bnu_img_policy.upgrade)
                {
                    bnu_policy->bnu_img_policy.upgrade = (bool)item->type;
                }
            }

            item = Cy_JWT_FindJsonItemByKey("encrypt", policy);
            if(NULL != item)
            {
                if(1U == bnu_policy->present.bnu_img_policy.encrypt)
                {
                    bnu_policy->bnu_img_policy.encrypt = (bool)item->type;
                }
            }

            item = Cy_JWT_FindJsonItemByKey("encrypt_key_id", policy);
            if(NULL != item)
            {
                if(1U == bnu_policy->present.bnu_img_policy.encrypt)
                {
                    bnu_policy->bnu_img_policy.encrypt_key_id = (uint8_t)item->valueint;
                }
            }

            item = Cy_JWT_FindJsonItemByKey("backup", policy);
            if(NULL != item)
            {
                if(1U == bnu_policy->present.bnu_img_policy.backup)
                {
                    bnu_policy->bnu_img_policy.backup = (bool)item->type;
                }
            }

            item = Cy_JWT_FindJsonItemByKey("boot_auth", policy);
            if(NULL != item && cJSON_Array == item->type)
            {
                item = item->child;
                for(int i = 0; i < POLICY_MAX_N_OF_KEYS; i++)
                {
                    if(NULL != item)
                    {
                        if(1U == bnu_policy->present.bnu_img_policy.boot_auth[i])
                        {
                            bnu_policy->bnu_img_policy.boot_auth[i] = (uint8_t)item->valueint;
                        }
                        item = item->next;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            item = Cy_JWT_FindJsonItemByKey("upgrade_auth", policy);
            if(NULL != item && cJSON_Array == item->type)
            {
                item = item->child;
                for(int i = 0; i < POLICY_MAX_N_OF_KEYS; i++)
                {
                    if(NULL != item)
                    {
                        if(1U == bnu_policy->present.bnu_img_policy.upgrade_auth[i])
                        {
                            bnu_policy->bnu_img_policy.upgrade_auth[i] = (uint8_t)item->valueint;
                        }
                        item = item->next;
                    }
                    else
                    {
                        break;
                    }
                }
            }

            item = Cy_JWT_FindJsonItemByKey("smif_id", policy);
            if(NULL != item)
            {
                bnu_policy->bnu_img_policy.smif_id = (uint8_t)((uint8_t)item->valueint & 0xffu);
            }
            else
            {
                bnu_policy->bnu_img_policy.smif_id = 0u;
            }
        }
    }

    /* Analyze 'reprogramming' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("reprogram", bnu_json);
    if(NULL != policy)
    {
        policy = policy->child;
        for(int i = 0; i < POLICY_MAX_N_OF_REPROG_REGIONS; i++)
        {
            if(NULL != policy)
            {
                item = Cy_JWT_FindJsonItemByKey("size", policy);
                if(NULL != item)
                {
                    if(1U == bnu_policy->present.bnu_reprog_policy[i].size)
                    {
                        bnu_policy->bnu_reprog_policy[i].size = (uint32_t)item->valueint;
                    }
                }

                item = Cy_JWT_FindJsonItemByKey("start", policy);
                if(NULL != item)
                {
                    if(1U == bnu_policy->present.bnu_reprog_policy[i].start)
                    {
                        bnu_policy->bnu_reprog_policy[i].start = (uint32_t)item->valueint;
                    }
                }
                policy = policy->next;
            }
            else
            {
                break;
            }
        }
    }

    /* Analyze 'reprovisioning' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("reprovision", bnu_json);
    if(NULL != policy)
    {
        item = Cy_JWT_FindJsonItemByKey("boot_loader", policy);
        if(NULL != item)
        {
            if(1U == bnu_policy->present.bnu_reprov_policy.boot_loader)
            {
                bnu_policy->bnu_reprov_policy.boot_loader = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("keys_and_policies", policy);
        if(NULL != item)
        {
            if(1U == bnu_policy->present.bnu_reprov_policy.keys_n_pol)
            {
                bnu_policy->bnu_reprov_policy.keys_n_pol = (bool)item->type;
            }
        }
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseWoundPolicy
****************************************************************************//**
* Takes parsed Wounding policy, looks for items and stores them into structure.
*
* \param wound_json     Pointer to parsed policy.
* \param wound_policy   Pointer to structure that contains policy in binary
*                       representation.
* \return               Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_WoundPolicyParse(cJSON *wound_json, wound_policy_t *wound_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *policy = NULL;
    cJSON *item = NULL;

    /* Analyze 'peripherals' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("peripherals", wound_json);
    if(NULL != policy)
    {
        policy = policy->child;
        for(int i = 0; i < POLICY_MAX_N_OF_PERIPH; i++)
        {
            if(NULL != policy)
            {
                item = Cy_JWT_FindJsonItemByKey("id", policy);
                if(NULL != item)
                {
                    if(1U == wound_policy->present.periph_policy[i].id)
                    {
                        wound_policy->periph_policy[i].id = (uint8_t)item->valueint;
                    }
                }

                item = Cy_JWT_FindJsonItemByKey("base_address", policy);
                if(NULL != item)
                {
                    if(1U == wound_policy->present.periph_policy[i].base_address)
                    {
                        wound_policy->periph_policy[i].base_address = (uint32_t)item->valueint;
                    }
                }
                policy = policy->next;
            }
            else
            {
                break;
            }
        }
    }

    /* Analyze 'memories' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("memories", wound_json);
    if(NULL != policy)
    {
        item = Cy_JWT_FindJsonItemByKey("flash", policy);
        if(NULL != item)
        {
            if(1U == wound_policy->present.mem_policy.flash)
            {
                wound_policy->mem_policy.flash = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("workflash", policy);
        if(NULL != item)
        {
            if(1U == wound_policy->present.mem_policy.workflash)
            {
                wound_policy->mem_policy.workflash = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("sflash", policy);
        if(NULL != item)
        {
            if(1U == wound_policy->present.mem_policy.sflash)
            {
                wound_policy->mem_policy.sflash = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("sram", policy);
        if(NULL != item)
        {
            if(1U == wound_policy->present.mem_policy.sram)
            {
                wound_policy->mem_policy.sram = (bool)item->type;
            }
        }
    }

    /* Analyze 'software' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("software", wound_json);
    if(NULL != policy)
    {
        policy = policy->child;
        for(int i = 0; i < POLICY_MAX_N_OF_SW; i++)
        {
            if(NULL != policy)
            {
                item = Cy_JWT_FindJsonItemByKey("id", policy);
                if(NULL != item)
                {
                    if(1U == wound_policy->present.soft_policy[i].id)
                    {
                        wound_policy->soft_policy[i].id = (uint8_t)item->valueint;
                    }
                }

                item = Cy_JWT_FindJsonItemByKey("capability", policy);
                if(NULL != item)
                {
                    if(1U == wound_policy->present.soft_policy[i].capability)
                    {
                        wound_policy->soft_policy[i].capability = (uint8_t)item->valueint;
                    }
                }
                policy = policy->next;
            }
            else
            {
                break;
            }
        }
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetPermissionCode
****************************************************************************//**
* Takes permission policy string and returns corresponding enum value.
*
* \param str  Permission policy string.
* \return     Permission policy enum value.
*******************************************************************************/
static perm_policy_t Cy_JWT_GetPermissionCode(const char *str)
{
    perm_policy_t p = PERM_DISABLED;

    if(0 == strcmp("disabled", str))
    {
        p = PERM_DISABLED;
    }
    else if(0 == strcmp("enabled", str))
    {
        p = PERM_ENABLED;
    }
    else if(0 == strcmp("allowed", str))
    {
        p = PERM_ALLOWED;
    }

    return p;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetControlCode
****************************************************************************//**
* Takes control policy string and returns corresponding enum value.
*
* \param str  Control policy string.
* \return     Control policy enum value.
*******************************************************************************/
static contrl_policy_t Cy_JWT_GetControlCode(const char *str)
{
    contrl_policy_t c = CONTRL_FIRMWARE;

    if(0 == strcmp("firmware", str))
    {
        c = CONTRL_FIRMWARE;
    }
    else if(0 == strcmp("certificate", str))
    {
        c = CONTRL_CERT;
    }
    else if(0 == strcmp("open", str))
    {
        c = CONTRL_OPEN;
    }

    return c;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetParseDebugPolicy
****************************************************************************//**
* Takes parsed Debug policy, looks for items and stores them into structure.
*
* \param debug_json     Pointer to parsed policy.
* \param debug_policy   Pointer to structure that contains policy in binary
*                       representation.
* \return               Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_DebugPolicyParse(cJSON *debug_json, debug_policy_t *debug_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *policy = NULL;
    cJSON *item = NULL;
    cJSON *array = NULL;

    /* Analyze 'm0p' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("m0p", debug_json);
    if(NULL != policy)
    {
        item = Cy_JWT_FindJsonItemByKey("permission", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.m0p_policy.permission)
            {
                debug_policy->m0p_policy.permission =
                    Cy_JWT_GetPermissionCode((const char*)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("control", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.m0p_policy.control)
            {
                debug_policy->m0p_policy.control =
                    Cy_JWT_GetControlCode((const char *)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("key", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.m0p_policy.key)
            {
                debug_policy->m0p_policy.key = (uint8_t)item->valueint;
            }
        }
    }

    /* Analyze 'm4' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("m4", debug_json);
    if(NULL != policy)
    {
        item = Cy_JWT_FindJsonItemByKey("permission", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.m4_policy.permission)
            {
                debug_policy->m4_policy.permission =
                    Cy_JWT_GetPermissionCode((const char*)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("control", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.m4_policy.control)
            {
                debug_policy->m4_policy.control =
                    Cy_JWT_GetControlCode((const char *)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("key", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.m4_policy.key)
            {
                debug_policy->m4_policy.key = (uint8_t)item->valueint;
            }
        }
    }

    /* Analyze 'system' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("system", debug_json);
    if(NULL != policy)
    {
        item = Cy_JWT_FindJsonItemByKey("permission", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.permission)
            {
                debug_policy->sys_policy.permission =
                    Cy_JWT_GetPermissionCode((const char*)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("control", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.control)
            {
                debug_policy->sys_policy.control =
                    Cy_JWT_GetControlCode((const char *)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("key", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.key)
            {
                debug_policy->sys_policy.key = (uint8_t)item->valueint;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("syscall", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.syscall)
            {
                debug_policy->sys_policy.syscall = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("mmio", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.mmio)
            {
                debug_policy->sys_policy.mmio = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("flash", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.flash)
            {
                debug_policy->sys_policy.flash = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("workflash", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.workflash)
            {
                debug_policy->sys_policy.workflash = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("sflash", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.sflash)
            {
                debug_policy->sys_policy.sflash = (bool)item->type;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("sram", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.sys_policy.sram)
            {
                debug_policy->sys_policy.sram = (bool)item->type;
            }
        }
    }

    /* Analyze 'rma' part of policy */
    policy = Cy_JWT_FindJsonItemByKey("rma", debug_json);
    {
        item = Cy_JWT_FindJsonItemByKey("permission", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.rma_policy.permission)
            {
                debug_policy->rma_policy.permission =
                    Cy_JWT_GetPermissionCode((const char*)item->valuestring);
            }
        }

        item = Cy_JWT_FindJsonItemByKey("key", policy);
        if(NULL != item)
        {
            if(1U == debug_policy->present.rma_policy.key)
            {
                debug_policy->rma_policy.key = (uint8_t)item->valueint;
            }
        }

        item = Cy_JWT_FindJsonItemByKey("destroy_fuses", policy);
        if(NULL != item)
        {
            array = item->child;
            for(int i = 0; i < POLICY_MAX_N_OF_DESTR_REGIONS; i++)
            {
                if(NULL != array)
                {
                    item = Cy_JWT_FindJsonItemByKey("start", array);
                    if(NULL != item)
                    {
                        if(1U == debug_policy->present.rma_policy.destroy_fuses[i].start)
                        {
                            debug_policy->rma_policy.destroy_fuses[i].start = (uint32_t)item->valueint;
                        }
                    }

                    item = Cy_JWT_FindJsonItemByKey("size", array);
                    if(NULL != item)
                    {
                        if(1U == debug_policy->present.rma_policy.destroy_fuses[i].size)
                        {
                            debug_policy->rma_policy.destroy_fuses[i].size = (uint32_t)item->valueint;
                        }
                    }
                    array = array->next;
                }
                else
                {
                    break;
                }
            }
        }

        item = Cy_JWT_FindJsonItemByKey("destroy_flash", policy);
        if(NULL != item)
        {
            array = item->child;
            for(int i = 0; i < POLICY_MAX_N_OF_DESTR_REGIONS; i++)
            {
                if(NULL != array)
                {
                    item = Cy_JWT_FindJsonItemByKey("start", array);
                    if(NULL != item)
                    {
                        if(1U == debug_policy->present.rma_policy.destroy_flash[i].start)
                        {
                            debug_policy->rma_policy.destroy_flash[i].start = (uint32_t)item->valueint;
                        }
                    }

                    item = Cy_JWT_FindJsonItemByKey("size", array);
                    if(NULL != item)
                    {
                        if(1U == debug_policy->present.rma_policy.destroy_flash[i].size)
                        {
                            debug_policy->rma_policy.destroy_flash[i].size = (uint32_t)item->valueint;
                        }
                    }
                    array = array->next;
                }
                else
                {
                    break;
                }
            }
        }
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_FindDefaultValueInTemplate
****************************************************************************//**
* Find default value for single item in policy template
*
* \param key  Name of the policy item which default value is looked for.
* \param json Pointer to JSON item which default value is looked for.
* \return     Pointer to JSON item that contains default value of given policy
*             item.
*******************************************************************************/
cJSON *Cy_JWT_FindDefaultValueInTemplate(const char *key, cJSON *json)
{
    cJSON *def = NULL;
    cJSON *item = NULL;
    cJSON *type = NULL;
    char def_key[20];

    memset(def_key, 0, sizeof(def_key));

    item = Cy_JWT_FindJsonItemByKey(key, json);
    while(NULL != item)
    {
        type = Cy_JWT_FindJsonItemByKey("type", item);
        if(NULL != type)
        {
            if((NULL != type->valuestring) && (0 == strcmp("string", type->valuestring)))
            {
                strcpy(def_key, "string");
            }
            else if(NULL != type->valuestring && 0 == strcmp("number", type->valuestring))
            {
                strcpy(def_key, "number");
            }
            else if(NULL != type->valuestring && 0 == strcmp("integer", type->valuestring))
            {
                strcpy(def_key, "number");
            }
            else if(NULL != type->valuestring && 0 == strcmp("boolean", type->valuestring))
            {
                strcpy(def_key, "boolean");
            }
            else if(NULL != type->valuestring && 0 == strcmp("array", type->valuestring))
            {
                item = Cy_JWT_FindJsonItemByKey("items", item);
                continue;
            }

            def = Cy_JWT_FindJsonItemByKey((const char*)def_key, item);
            item = NULL;
        }
    }

    return def;
}

/*******************************************************************************
* Function Name: Cy_JWT_FindPresentValueInTemplate
****************************************************************************//**
* Find 'present' property value for single item in policy template
*
* \param key  Name of the policy item which 'present' value is looked for.
* \param json Pointer to JSON item which 'present' value is looked for.
* \return     Value of the 'present' property.
*******************************************************************************/
uint8_t Cy_JWT_FindPresentValueInTemplate(const char *key, cJSON *json)
{
    uint8_t present_val = 1U;
    cJSON *item = NULL;
    cJSON *present = NULL;
    char def_key[20];

    memset(def_key, 0, sizeof(def_key));

    item = Cy_JWT_FindJsonItemByKey(key, json);
    if((NULL != item) && (NULL != item->child))
    {
        present = Cy_JWT_FindJsonItemByKey("present", item->child);
        if(NULL != present)
        {
            present_val = (uint8_t)present->type;
        }
    }

    return present_val;
}

/*******************************************************************************
* Function Name: Cy_JWT_DefaultDebugPolicy
****************************************************************************//**
* Set Debug policy structure to default values using corresponding template
*
* \param schema_str  Schema policy template with default values.
* \param debug_policy  Debug policy to be initialized.
* \return              Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_DebugPolicySetDefault(const char *schema_str, debug_policy_t *debug_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *schema = NULL;
    cJSON *item = NULL;
    cJSON *array = NULL;
    cJSON *def = NULL;
    uint8_t present;

    cJSON_InitHooks(NULL);

    schema = cJSON_Parse(schema_str);
    if(NULL != schema)
    {
        item = Cy_JWT_FindJsonItemByKey("m0p", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("permission", item);
            present = Cy_JWT_FindPresentValueInTemplate("permission", item);
            if(NULL != def)
            {
                debug_policy->m0p_policy.permission =
                        Cy_JWT_GetPermissionCode((const char *)def->valuestring);
                debug_policy->present.m0p_policy.permission = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("control", item);
            present = Cy_JWT_FindPresentValueInTemplate("control", item);
            if(NULL != def)
            {
                debug_policy->m0p_policy.control =
                        Cy_JWT_GetControlCode((const char *)def->valuestring);
                debug_policy->present.m0p_policy.control = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("key", item);
            present = Cy_JWT_FindPresentValueInTemplate("key", item);
            if(NULL != def)
            {
                debug_policy->m0p_policy.key = (uint8_t)def->valueint;
                debug_policy->present.m0p_policy.key = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("m4", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("permission", item);
            present = Cy_JWT_FindPresentValueInTemplate("permission", item);
            if(NULL != def)
            {
                debug_policy->m4_policy.permission =
                        Cy_JWT_GetPermissionCode((const char *)def->valuestring);
                debug_policy->present.m4_policy.permission = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("control", item);
            present = Cy_JWT_FindPresentValueInTemplate("control", item);
            if(NULL != def)
            {
                debug_policy->m4_policy.control =
                        Cy_JWT_GetControlCode((const char *)def->valuestring);
                debug_policy->present.m4_policy.control = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("key", item);
            present = Cy_JWT_FindPresentValueInTemplate("key", item);
            if(NULL != def)
            {
                debug_policy->m4_policy.key = (uint8_t)def->valueint;
                debug_policy->present.m4_policy.key = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("system", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("permission", item);
            present = Cy_JWT_FindPresentValueInTemplate("permission", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.permission =
                        Cy_JWT_GetPermissionCode((const char *)def->valuestring);
                debug_policy->present.sys_policy.permission = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("control", item);
            present = Cy_JWT_FindPresentValueInTemplate("control", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.control =
                        Cy_JWT_GetControlCode((const char *)def->valuestring);
                debug_policy->present.sys_policy.control = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("key", item);
            present = Cy_JWT_FindPresentValueInTemplate("key", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.key = (uint8_t)def->valueint;
                debug_policy->present.sys_policy.key = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("syscall", item);
            present = Cy_JWT_FindPresentValueInTemplate("syscall", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.syscall = (bool)def->type;
                debug_policy->present.sys_policy.syscall = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("mmio", item);
            present = Cy_JWT_FindPresentValueInTemplate("mmio", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.mmio = (bool)def->type;
                debug_policy->present.sys_policy.mmio = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("flash", item);
            present = Cy_JWT_FindPresentValueInTemplate("flash", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.flash = (bool)def->type;
                debug_policy->present.sys_policy.flash = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("workflash", item);
            present = Cy_JWT_FindPresentValueInTemplate("workflash", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.workflash = (bool)def->type;
                debug_policy->present.sys_policy.workflash = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("sflash", item);
            present = Cy_JWT_FindPresentValueInTemplate("sflash", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.sflash = (bool)def->type;
                debug_policy->present.sys_policy.sflash = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("sram", item);
            present = Cy_JWT_FindPresentValueInTemplate("sram", item);
            if(NULL != def)
            {
                debug_policy->sys_policy.sram = (bool)def->type;
                debug_policy->present.sys_policy.sram = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("rma", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("permission", item);
            present = Cy_JWT_FindPresentValueInTemplate("permission", item);
            if(NULL != def)
            {
                debug_policy->rma_policy.permission =
                        Cy_JWT_GetPermissionCode((const char *)def->valuestring);
                debug_policy->present.rma_policy.permission = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("key", item);
            present = Cy_JWT_FindPresentValueInTemplate("key", item);
            if(NULL != def)
            {
                debug_policy->rma_policy.key = (uint8_t)def->valueint;
                debug_policy->present.rma_policy.key = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            array = Cy_JWT_FindJsonItemByKey("destroy_fuses", item);
            if(NULL != array)
            {
                uint32_t start = 0U;
                uint32_t size = 0U;
                uint8_t startPresent;
                uint8_t sizePresent;

                def = Cy_JWT_FindDefaultValueInTemplate("start", array);
                startPresent = Cy_JWT_FindPresentValueInTemplate("start", array);
                if(NULL != def)
                {
                    start = (uint32_t)def->valueint;
                }
                else
                {
                    rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                }

                def = Cy_JWT_FindDefaultValueInTemplate("size", array);
                sizePresent = Cy_JWT_FindPresentValueInTemplate("size", array);
                if(NULL != def)
                {
                    size = (uint32_t)def->valueint;
                }
                else
                {
                    rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                }

                for(int i = 0; i < POLICY_MAX_N_OF_DESTR_REGIONS; i++)
                {
                    debug_policy->rma_policy.destroy_fuses[i].size = size;
                    debug_policy->rma_policy.destroy_fuses[i].start = start;
                    debug_policy->present.rma_policy.destroy_fuses[i].size = sizePresent;
                    debug_policy->present.rma_policy.destroy_fuses[i].start = startPresent;
                }
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            array = Cy_JWT_FindJsonItemByKey("destroy_flash", item);
            if(NULL != array)
            {
                uint32_t start = 0U;
                uint32_t size = 0U;
                uint8_t startPresent;
                uint8_t sizePresent;

                def = Cy_JWT_FindDefaultValueInTemplate("start", array);
                startPresent = Cy_JWT_FindPresentValueInTemplate("start", array);
                if(NULL != def)
                {
                    start = (uint32_t)def->valueint;
                }
                else
                {
                    rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                }

                def = Cy_JWT_FindDefaultValueInTemplate("size", array);
                sizePresent = Cy_JWT_FindPresentValueInTemplate("size", array);
                if(NULL != def)
                {
                    size = (uint32_t)def->valueint;
                }
                else
                {
                    rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                }

                for(int i = 0; i < POLICY_MAX_N_OF_DESTR_REGIONS; i++)
                {
                    debug_policy->rma_policy.destroy_flash[i].size = size;
                    debug_policy->rma_policy.destroy_flash[i].start = start;
                    debug_policy->present.rma_policy.destroy_flash[i].size = sizePresent;
                    debug_policy->present.rma_policy.destroy_flash[i].start = startPresent;
                }
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        cJSON_Delete(schema);
    }
    else
    {
        rc = CY_JWT_ERR_JSN_PARSE_FAIL;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_DefaultWoundingPolicy
****************************************************************************//**
* Set Wounding policy structure to default values using corresponding template
*
* \param schema_str  Schema policy template with default values.
* \param wounding_policy  Wounding policy to be initialized.
* \return                Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_WoundingPolicySetDefault(const char *schema_str, wound_policy_t *wounding_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *schema = NULL;
    cJSON *item = NULL;
    cJSON *def = NULL;
    uint8_t present;

    cJSON_InitHooks(NULL);

    schema = cJSON_Parse(schema_str);
    if(NULL != schema)
    {
        item = Cy_JWT_FindJsonItemByKey("peripherals", schema);
        if(NULL != item)
        {
            uint32_t id = 0U;
            uint32_t base_address = 0U;
            uint8_t idPresent;
            uint8_t addrPresent;

            def = Cy_JWT_FindDefaultValueInTemplate("id", item);
            idPresent = Cy_JWT_FindPresentValueInTemplate("id", item);
            if(NULL != def)
            {
                id = (uint32_t)def->valueint;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("base_address", item);
            addrPresent = Cy_JWT_FindPresentValueInTemplate("base_address", item);
            if(NULL != def)
            {
                base_address = (uint32_t)def->valueint;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            for(int i = 0; i < POLICY_MAX_N_OF_PERIPH; i++)
            {
                wounding_policy->periph_policy[i].id = id;
                wounding_policy->periph_policy[i].base_address = base_address;
                wounding_policy->present.periph_policy[i].id = idPresent;
                wounding_policy->present.periph_policy[i].base_address = addrPresent;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("memories", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("flash", item);
            present = Cy_JWT_FindPresentValueInTemplate("flash", item);
            if(NULL != def)
            {
                wounding_policy->mem_policy.flash = (bool)def->type;
                wounding_policy->present.mem_policy.flash = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("workflash", item);
            present = Cy_JWT_FindPresentValueInTemplate("workflash", item);
            if(NULL != def)
            {
                wounding_policy->mem_policy.workflash = (bool)def->type;
                wounding_policy->present.mem_policy.workflash = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("sflash", item);
            present = Cy_JWT_FindPresentValueInTemplate("sflash", item);
            if(NULL != def)
            {
                wounding_policy->mem_policy.sflash = (bool)def->type;
                wounding_policy->present.mem_policy.sflash = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("sram", item);
            present = Cy_JWT_FindPresentValueInTemplate("sram", item);
            if(NULL != def)
            {
                wounding_policy->mem_policy.sram = (bool)def->valueint;
                wounding_policy->present.mem_policy.sram = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("software", schema);
        if(NULL != item)
        {
            uint32_t id = 0U;
            uint32_t capability = 0U;
            uint8_t id_present;
            uint8_t cap_present;

            def = Cy_JWT_FindDefaultValueInTemplate("id", item);
            id_present = Cy_JWT_FindPresentValueInTemplate("id", item);
            if(NULL != def)
            {
                id = (uint32_t)def->valueint;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("capability", item);
            cap_present = Cy_JWT_FindPresentValueInTemplate("capability", item);
            if(NULL != def)
            {
                capability = (uint32_t)def->valueint;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            for(int i = 0; i < POLICY_MAX_N_OF_SW; i++)
            {
                wounding_policy->soft_policy[i].id = id;
                wounding_policy->soft_policy[i].capability = capability;
                wounding_policy->present.soft_policy[i].id = id_present;
                wounding_policy->present.soft_policy[i].capability = cap_present;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        cJSON_Delete(schema);
    }
    else
    {
        rc = CY_JWT_ERR_JSN_PARSE_FAIL;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_DefaultBnUPolicy
****************************************************************************//**
* Set Boot & Upgrade policy structure to default values using corresponding template
*
* \param schema_str  Schema policy template with default values.
* \param bnu_policy  Boot&Upgrade policy to be initialized.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_BnUPolicySetDefault(const char *schema_str, bnu_policy_t *bnu_policy)
{
    int rc = CY_JWT_SUCCESS;
    cJSON *schema = NULL;
    cJSON *item = NULL;
    cJSON *def = NULL;
    uint8_t present;

    cJSON_InitHooks(NULL);

    schema = cJSON_Parse(schema_str);
    if(NULL != schema)
    {
        item = Cy_JWT_FindJsonItemByKey("firmware", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("id", item);
            present = Cy_JWT_FindPresentValueInTemplate("id", item);
            if(NULL != def)
            {
                bnu_policy->bnu_img_policy.id = (uint8_t)def->valueint;
                bnu_policy->present.bnu_img_policy.id = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("upgrade", item);
            present = Cy_JWT_FindPresentValueInTemplate("upgrade", item);
            if(NULL != def)
            {
                bnu_policy->bnu_img_policy.upgrade = (bool)def->type;
                bnu_policy->present.bnu_img_policy.upgrade = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("encrypt", item);
            present = Cy_JWT_FindPresentValueInTemplate("encrypt", item);
            if(NULL != def)
            {
                bnu_policy->bnu_img_policy.encrypt = (bool)def->type;
                bnu_policy->present.bnu_img_policy.encrypt = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("monotonic", item);
            present = Cy_JWT_FindPresentValueInTemplate("monotonic", item);
            if(NULL != def)
            {
                bnu_policy->bnu_img_policy.monotonic = (uint8_t)def->valueint;
                bnu_policy->present.bnu_img_policy.monotonic = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("boot_auth", item);
            present = Cy_JWT_FindPresentValueInTemplate("boot_auth", item);
            if(NULL != def)
            {
                for(int i = 0; i < POLICY_MAX_N_OF_KEYS; i++)
                {
                    bnu_policy->bnu_img_policy.boot_auth[i] = (uint8_t)def->valueint;
                    bnu_policy->present.bnu_img_policy.boot_auth[i] = present;
                }
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("upgrade_auth", item);
            present = Cy_JWT_FindPresentValueInTemplate("upgrade_auth", item);
            if(NULL != def)
            {
                for(int i = 0; i < POLICY_MAX_N_OF_KEYS; i++)
                {
                    bnu_policy->bnu_img_policy.upgrade_auth[i] = (uint8_t)def->valueint;
                    bnu_policy->present.bnu_img_policy.upgrade_auth[i] = present;
                }
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("backup", item);
            present = Cy_JWT_FindPresentValueInTemplate("backup", item);
            if(NULL != def)
            {
                bnu_policy->bnu_img_policy.backup = (bool)def->type;
                bnu_policy->present.bnu_img_policy.backup = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            item = Cy_JWT_FindJsonItemByKey("resources", schema);
            if(NULL != def)
            {
                uint32_t address = 0U;
                uint32_t size = 0U;
                uint8_t addressPresent;
                uint8_t sizePresent;

                // TODO: need to set it when validating per schema
                bnu_policy->present.bnu_img_policy.boot_area.start = 1U;
                bnu_policy->present.bnu_img_policy.boot_area.size = 1U;

                bnu_policy->present.bnu_img_policy.upgrade_area.start = 1U;
                bnu_policy->present.bnu_img_policy.upgrade_area.size = 1U;

                def = Cy_JWT_FindDefaultValueInTemplate("address", item);
                addressPresent = Cy_JWT_FindPresentValueInTemplate("address", item);
                if(NULL != def)
                {
                    address = (uint32_t)def->valueint;
                }
                else
                {
                    rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                }

                def = Cy_JWT_FindDefaultValueInTemplate("size", item);
                sizePresent = Cy_JWT_FindPresentValueInTemplate("size", item);
                if(NULL != def)
                {
                    size = (uint32_t)def->valueint;
                }
                else
                {
                    rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                }

                for(int i = 0; i < POLICY_MAX_N_OF_PROT_REGIONS; i++)
                {
                    bnu_policy->prot_regions[i].start = address;
                    bnu_policy->prot_regions[i].size = size;
                    bnu_policy->present.prot_regions[i].start = addressPresent;
                    bnu_policy->present.prot_regions[i].size = sizePresent;
                }
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("reprogram", schema);
        if(NULL != item)
        {
            uint32_t start = 0U;
            uint32_t size = 0U;
            uint8_t startPresent;
            uint8_t sizePresent;

            def = Cy_JWT_FindDefaultValueInTemplate("start", item);
            startPresent = Cy_JWT_FindPresentValueInTemplate("start", item);
            if(NULL != def)
            {
                start = (uint32_t)def->valueint;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("size", item);
            sizePresent = Cy_JWT_FindPresentValueInTemplate("size", item);
            if(NULL != def)
            {
                size = (uint32_t)def->valueint;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            for(int i = 0; i < POLICY_MAX_N_OF_REPROG_REGIONS; i++)
            {
                bnu_policy->bnu_reprog_policy[i].size = size;
                bnu_policy->bnu_reprog_policy[i].start = start;
                bnu_policy->present.bnu_reprog_policy[i].size = sizePresent;
                bnu_policy->present.bnu_reprog_policy[i].start = startPresent;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        item = Cy_JWT_FindJsonItemByKey("reprovision", schema);
        if(NULL != item)
        {
            def = Cy_JWT_FindDefaultValueInTemplate("keys_and_policies", item);
            present = Cy_JWT_FindPresentValueInTemplate("keys_and_policies", item);
            if(NULL != def)
            {
                bnu_policy->bnu_reprov_policy.keys_n_pol = (bool)def->type;
                bnu_policy->present.bnu_reprov_policy.keys_n_pol = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }

            def = Cy_JWT_FindDefaultValueInTemplate("boot_loader", item);
            present = Cy_JWT_FindPresentValueInTemplate("boot_loader", item);
            if(NULL != def)
            {
                bnu_policy->bnu_reprov_policy.boot_loader = (bool)def->type;
                bnu_policy->present.bnu_reprov_policy.boot_loader = present;
            }
            else
            {
                rc = CY_JWT_ERR_JKEY_NOT_FOUND;
            }
        }
        else
        {
            rc = CY_JWT_ERR_JKEY_NOT_FOUND;
        }

        cJSON_Delete(schema);
    }
    else
    {
        rc = CY_JWT_ERR_JSN_PARSE_FAIL;
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_GetProvisioningDetails
****************************************************************************//**
* Read provisioning packet (JWT), policy templates or public keys strings in
* JSON format.
*
* \param id          Item id (provisioning packet, templates or public keys).
* \param ptr         Pointer to the response string.
* \param len         Length of the response string.
* \return            Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_GetProvisioningDetails(uint32_t id, char **ptr, uint32_t *len)
{
    int rc = -1;
    volatile uint32_t syscallCmd[2];
    volatile uint32_t syscallParam[2];
    uint32_t timeout = 0U;

    syscallCmd[0] = PROVDETAIL_SYSCALL_OPCODE;
    syscallCmd[1] = (uint32_t)syscallParam;

    syscallParam[0] = id;
    syscallParam[1] = 0U;

    /* Get IPC base register address */
    IPC_STRUCT_Type * ipcStruct = Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_SYSCALL);

    while((CY_IPC_DRV_SUCCESS != Cy_IPC_Drv_LockAcquire(ipcStruct)) &&
            (timeout < SYSCALL_TIMEOUT))
    {
        ++timeout;
    }

    if(timeout < SYSCALL_TIMEOUT)
    {
        timeout = 0U;

        Cy_IPC_Drv_WriteDataValue(ipcStruct, (uint32_t)syscallCmd);
        Cy_IPC_Drv_AcquireNotify(ipcStruct, (1<<CY_IPC_CHAN_SYSCALL));

        while((Cy_IPC_Drv_IsLockAcquired(ipcStruct))&&
                (timeout < SYSCALL_TIMEOUT))
        {
            ++timeout;
        }

        if(timeout < SYSCALL_TIMEOUT)
        {
            if(CY_FB_SYSCALL_SUCCESS != syscallCmd[0])
            {
                rc = syscallCmd[0];
            }
            else
            {
                *len = syscallParam[0];
                *ptr = (char*)syscallParam[1];
                rc = 0;
            }
        }

    }
    return rc;
}
