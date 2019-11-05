/***************************************************************************//**
* \file cy_jwt_bnu_policy.c
* \version 1.0
*
* \brief
*  This is the source code for functions to extract Boot and Upgrade policy
*  from the Provisioning packet.
*
*****************************************?***************************************
* \copyright
* Copyright 2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include <stdint.h>
#include <stdlib.h>

#include "base64/base64.h"
#include "cy_jwt_bnu_policy.h"

/*******************************************************************************
* Function Name: Cy_JWT_ParseProvisioningPacket
****************************************************************************//**
* Parses provisioning packet (JWT) and looks for provisioning request JWT packet.
*
* \param provPacket    JWT provisioning packet (0 terminated).
* \param bnuPolicy     Boot and Upgrade policy structure.
* \param masterImageId ID of the current image.
* \return              Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ParseProvisioningPacket(char *provPacket, bnu_policy_t *bnuPolicy,
        debug_policy_t *debugPolicy, uint8_t masterImageId)
{
    int rc = CY_JWT_SUCCESS;
    char *bodyStart = NULL;
    uint32_t bodySize = 0;
    char *jsonStr = NULL;
    uint32_t jsonLen = 0;
    cJSON *json = NULL;
    cJSON *provReq = NULL;

    rc = Cy_JWT_GetStrDataBody(provPacket, &bodyStart, &bodySize);
    if(CY_JWT_SUCCESS == rc)
    {
        jsonLen = Cy_JWT_GetB64DecodeLen(bodySize);
        jsonStr = malloc(jsonLen);
        if(jsonStr != NULL)
        {
            rc = base64_decode((uint8_t*)bodyStart, bodySize,
                    (uint8_t*)jsonStr, jsonLen, BASE64_URL_SAFE_CHARSET);
            if(rc > 0)
            {
                json = cJSON_Parse(jsonStr);
                if(NULL != json)
                {
                    provReq = Cy_JWT_FindJsonItemByKey("prov_req", json);
                    if((NULL != provReq) && (NULL != provReq->valuestring))
                    {
                        rc = Cy_JWT_ParseProvisioningRequest(
                                provReq->valuestring, jsonStr,
                                jsonLen, bnuPolicy, debugPolicy,
                                masterImageId);
                    }
                    else
                    {
                        rc = CY_JWT_ERR_JKEY_NOT_FOUND;
                    }
                    cJSON_Delete(json);
                }
                else
                {
                    rc = CY_JWT_ERR_JSN_PARSE_FAIL;
                }
            }
            else
            {
                rc = CY_JWT_ERR_B64DECODE_FAIL;
            }

            free(jsonStr);
        }
        else
        {
            rc = CY_JWT_ERR_JWT_MALLOC_FAIL;
        }
    }

    return rc;
}

/*******************************************************************************
* Function Name: Cy_JWT_ParseProvisioningPacket
****************************************************************************//**
* Parses provisioning request packet (JWT) and looks for Boot and Upgrade policy.
*
* \param provReq       JWT provisioning request packet.
* \param buff          Buffer for decoded JSON.
* \param buffLen       Length of the buffer for decoded JSON.
* \param bnuPolicy     Boot and Upgrade policy structure.
* \param masterImageId ID of the current image.
* \return              Result of operation: 0 if success, errorcode otherwise.
*******************************************************************************/
int Cy_JWT_ParseProvisioningRequest(char *provReq, char *buff, uint32_t buffLen,
        bnu_policy_t *bnuPolicy, debug_policy_t *debugPolicy,
        uint8_t masterImageId)
{
    int rc = CY_JWT_SUCCESS;
    char *bodyStart = NULL;
    uint32_t bodySize = 0;

    rc = Cy_JWT_GetStrDataBody(provReq, &bodyStart, &bodySize);
    if(CY_JWT_SUCCESS == rc)
    {
        rc = base64_decode((uint8_t*)bodyStart, bodySize,
                (uint8_t*)buff, buffLen, BASE64_URL_SAFE_CHARSET);
        if(rc > 0)
        {
            policy_set_t policy_set;
            char *template = NULL;
            uint32_t templateLen = 0;

            /* Read BnU template and set default values for BnU policy */
            rc = Cy_JWT_GetProvisioningDetails(FB_POLICY_TEMPL_BOOT, &template,
                    &templateLen);
            if(CY_JWT_SUCCESS == rc)
            {
                rc = Cy_JWT_BnUPolicySetDefault(template, bnuPolicy);
            }
            else
            {
                rc = CY_JWT_ERR_OTHER;
            }

            /* Read debug template and set default values for debug policy */
            rc = Cy_JWT_GetProvisioningDetails(FB_POLICY_TEMPL_DEBUG, &template,
                    &templateLen);
            if(CY_JWT_SUCCESS == rc)
            {
                rc = Cy_JWT_DebugPolicySetDefault(template, debugPolicy);
            }
            else
            {
                rc = CY_JWT_ERR_OTHER;
            }

            if(CY_JWT_SUCCESS == rc)
            {
                rc = Cy_JWT_PolicyPackageSplit(buff, &policy_set);
                if(CY_JWT_SUCCESS == rc)
                {
                    rc = Cy_JWT_DebugPolicyParse(policy_set.boot, debugPolicy);
                    if(CY_JWT_SUCCESS == rc)
                    {
                        /* Look for "launch" inside Bootloader's policy image blob
                        *  load policy corresponding to "launch" ID */
                        rc = Cy_JWT_BnUPolicyParse(policy_set.boot, bnuPolicy,
                                masterImageId);
                        cJSON_Delete(policy_set.root);
                    }
                }
            }
        }
        else
        {
            rc = CY_JWT_ERR_B64DECODE_FAIL;
        }
    }

    return rc;
}
