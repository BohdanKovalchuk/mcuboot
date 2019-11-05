/****************************************?***********************************//**
* \file cy_jwt_policy.h
* \version 1.0
*
* \brief
*  This is the header file for the JWT policy parsing and processing.
*
*****************************************?***************************************
* \copyright
* Copyright 2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*****************************************?**************************************/

#ifndef CY_JWT_POLICY_H
#define CY_JWT_POLICY_H

#include <stdint.h>
#include <stdbool.h>

#include "psa/crypto.h"

#include "cJSON.h"

/** Array sizes for policy types */
#define POLICY_MAX_N_OF_KEYS            (3)
#define POLICY_MAX_N_OF_REPROG_REGIONS  (5)
#define POLICY_MAX_N_OF_PERIPH          (10)
#define POLICY_MAX_N_OF_SW              (5)
#define POLICY_MAX_N_OF_DESTR_REGIONS   (5)
#define POLICY_MAX_N_OF_PROT_REGIONS    (23)

/* The size for composite XY key storage */
#define KEY_XY_LENGTH   (44)

/** IDs of the parts of the provisioning packet */
#define FB_KEY_SLOT_NA              (0u)
#define FB_KEY_SLOT_DEVICE_ECDH     (1u)
#define FB_KEY_SLOT_DEVICE_ECDSA    (2u)
#define FB_KEY_SLOT_CYPRESS         (3u)
#define FB_KEY_SLOT_HSM             (4u)
#define FB_KEY_SLOT_OEM             (5u)
#define FB_KEY_SLOT_STATIC_MAX      (FB_KEY_SLOT_OEM)
#define FB_KEY_SLOT_UDS             (6u)
#define FB_KEY_SLOT_AES             (FB_KEY_SLOT_UDS)
#define FB_POLICY_JWT               (0x100u)
#define FB_POLICY_TEMPL_BOOT        (0x101u)
#define FB_POLICY_TEMPL_DEBUG       (0x102u)
#define FB_POLICY_TEMPL_WOUNDING    (0x103u)

/** Error codes for policy processing functions */
typedef enum cy_jwt_error_codes
{
    CY_JWT_SUCCESS = 0,
    /* generic JSON - [-1, -20) */
    CY_JWT_ERR_JKEY_NOT_FOUND = -1,
    CY_JWT_ERR_JSN_NONOBJ = -2,
    CY_JWT_ERR_JWT_PACKET_PARSE = -3,
    CY_JWT_ERR_JSN_BIG_NESTING = -4,
    CY_JWT_ERR_JSN_PARSE_FAIL = -5,
    /* ECC KEYs [-10, -20) */
    CY_JWT_ERR_EC_KEY_CRV_INV = -10,
    CY_JWT_ERR_EC_KEY_NON_COMPLT = -11,
    CY_JWT_ERR_UNSUPPORTED_ALG = -12,
    /* JWT [-20, -30) */
    CY_JWT_ERR_B64DECODE_FAIL = -20,
    CY_JWT_ERR_JWT_TOO_BIG = -21,
    CY_JWT_ERR_JWT_BROKEN_FORMAT = -22,
    /* Templates [-30, -40) */
    CY_JWT_ERR_TEMPL_VALID_FAIL = -30,
    CY_JWT_ERR_JWT_MALLOC_FAIL = -100,
    CY_JWT_ERR_OTHER = -101
} cy_jwt_error_codes_t;

/** Protection regions names */
typedef enum
{
    CY_PROT_FLASH_PC1_SPM = 0,
    CY_PROT_FLASH_PC2,
    CY_PROT_FLASH_PC3,
    CY_PROT_FLASH_PC4,
    CY_PROT_FLASH_MAIN_CODE,
    CY_PROT_FLASH_MAIN_XO,
    CY_PROT_FLASH_MAIN_DATA,
    CY_PROT_FLASH_SUPERVISORY,
    CY_PROT_FLASH_WORK_SECURE,
    CY_PROT_FLASH_WORK,
    CY_PROT_SRAM_PC0_PRIV,
    CY_PROT_SRAM_PC0_PUB,
    CY_PROT_SRAM_SPM_PRIV,
    CY_PROT_SRAM_SPM_PUB,
    CY_PROT_SRAM_PC1_PRIV,
    CY_PROT_SRAM_PC2_PRIV,
    CY_PROT_SRAM_PC3_PRIV,
    CY_PROT_SRAM_PC4_PRIV,
    CY_PROT_SRAM_MAIN,
    CY_PROT_SRAM_DAP,
    CY_PROT_SMIF_CODE,
    CY_PROT_SMIF_XO,
    CY_PROT_SMIF_DATA
}cy_jwt_smpu_reg_t;

/** EC curves names definition. Only P-256 curve supported */
typedef enum
{
    EC_CRV_P_256 = 0,
    /* add more if scaling */
    EC_KEY_CRV_INV = -1
}ec_curve_type;

typedef struct
{
    char *debug;
    char *wounding;
    char *boot;
}schema_set_t;

/* Type for parsed and split into 3 peaces policy */
typedef struct
{
    cJSON *root; /* We need root of whole policy to free it after */
    cJSON *debug;
    cJSON *wounding;
    cJSON *boot;
}policy_set_t;

/* Reprovision policy type, part of Boot and Upgrade policy */
typedef struct
{
    bool keys_n_pol;
    bool boot_loader;
}bnu_reprov_policy_t;

/* Memory region type, used in Debug, Boot and Upgrade policies */
typedef struct
{
    uint32_t start;
    uint32_t size;
}cy_jwt_mem_region_t;

/* Reprogramming policy type, part of Boot and Upgrade policy */
typedef cy_jwt_mem_region_t bnu_reprog_policy_t;

/* Image policy type, part of Boot and Upgrade policy */
typedef struct
{
    uint8_t id;
    bool upgrade;
    uint8_t smif_id;
    bool encrypt;
    uint8_t encrypt_key_id;
    uint32_t monotonic;
    cy_jwt_mem_region_t boot_area;
    cy_jwt_mem_region_t upgrade_area;
    uint8_t boot_auth[POLICY_MAX_N_OF_KEYS];
    uint8_t upgrade_auth[POLICY_MAX_N_OF_KEYS];
    bool backup;
}bnu_img_policy_t;

/* Boot and Upgrade policy type */
typedef struct
{
    bnu_img_policy_t bnu_img_policy;
    bnu_reprog_policy_t bnu_reprog_policy[POLICY_MAX_N_OF_REPROG_REGIONS];
    bnu_reprov_policy_t bnu_reprov_policy;
    cy_jwt_mem_region_t prot_regions[POLICY_MAX_N_OF_PROT_REGIONS];
    struct
    {
        struct
        {
            uint8_t id;
            uint8_t upgrade;
            uint8_t smif_id;
            uint8_t encrypt;
            uint8_t monotonic;
            struct
			{
            	uint8_t start;
            	uint8_t size;
            }boot_area;
            struct
			{
            	uint8_t start;
            	uint8_t size;
            }upgrade_area;
            uint8_t boot_auth[POLICY_MAX_N_OF_KEYS];
            uint8_t upgrade_auth[POLICY_MAX_N_OF_KEYS];
            uint8_t backup;
        }bnu_img_policy;
        struct
        {
            uint8_t start;
            uint8_t size;
        }bnu_reprog_policy[POLICY_MAX_N_OF_REPROG_REGIONS];
        struct
        {
            uint8_t keys_n_pol;
            uint8_t boot_loader;
        }bnu_reprov_policy;
        struct
        {
            uint8_t start;
            uint8_t size;
        }prot_regions[POLICY_MAX_N_OF_PROT_REGIONS];
    }present;
}bnu_policy_t;

/* Peripherals policy type, part of Wounding policy */
typedef struct
{
    uint32_t id;
    uint32_t base_address;
}periph_policy_t;

/* Memories policy type, part of Wounding policy */
typedef struct
{
    uint32_t flash;
    uint32_t workflash;
    uint32_t sflash;
    uint32_t sram;
}mem_policy_t;

/* Software policy type, part of Wounding policy */
typedef struct
{
    uint32_t id;
    uint32_t capability;
}soft_policy_t;

/* Wounding policy type */
typedef struct
{
    periph_policy_t periph_policy[POLICY_MAX_N_OF_PERIPH];
    mem_policy_t mem_policy;
    soft_policy_t soft_policy[POLICY_MAX_N_OF_SW];
    struct
    {
        struct
        {
            uint8_t id;
            uint8_t base_address;
        }periph_policy[POLICY_MAX_N_OF_PERIPH];
        struct
        {
            uint8_t flash;
            uint8_t workflash;
            uint8_t sflash;
            uint8_t sram;
        }mem_policy;
        struct
        {
            uint8_t id;
            uint8_t capability;
        }soft_policy[POLICY_MAX_N_OF_SW];
    }present;
}wound_policy_t;

/* Permission options enumeration */
typedef enum
{
    PERM_DISABLED,
    PERM_ENABLED,
    PERM_ALLOWED
}perm_policy_t;

/* Control options enumeration */
typedef enum
{
    CONTRL_FIRMWARE,
    CONTRL_CERT,
    CONTRL_OPEN
}contrl_policy_t;

/* CPU policy type, part of Debug policy */
typedef struct
{
    perm_policy_t permission;
    contrl_policy_t control;
    uint8_t key;
}cpu_policy_t;

/* System policy type, part of Debug policy */
typedef struct
{
    perm_policy_t permission;
    contrl_policy_t control;
    uint8_t key;
    bool syscall;
    bool mmio;
    bool flash;
    bool workflash;
    bool sflash;
    bool sram;
}system_policy_t;

/* RMA policy type, part of Debug policy */
typedef struct
{
    perm_policy_t permission;
    cy_jwt_mem_region_t destroy_fuses[POLICY_MAX_N_OF_DESTR_REGIONS];
    cy_jwt_mem_region_t destroy_flash[POLICY_MAX_N_OF_DESTR_REGIONS];
    uint8_t key;
}rma_policy_t;

/* Debug policy type */
typedef struct
{
    cpu_policy_t m0p_policy;
    cpu_policy_t m4_policy;
    system_policy_t sys_policy;
    rma_policy_t rma_policy;
    struct
    {
        struct /* M0p debug access */
        {
            uint8_t permission;
            uint8_t control;
            uint8_t key;
        }m0p_policy;
        struct /* M4 debug access */
        {
            uint8_t permission;
            uint8_t control;
            uint8_t key;
        }m4_policy;
        struct /* SysAP debug access */
        {
            uint8_t permission;
            uint8_t control;
            uint8_t key;
            uint8_t syscall;
            uint8_t mmio;
            uint8_t flash;
            uint8_t workflash;
            uint8_t sflash;
            uint8_t sram;
        }sys_policy;
        struct /* Availability and capabilities of the RMA life cycle stage */
        {
            uint8_t permission;
            struct /* Ranges of fuses to be destroyed before allowing transition to RMA stage */
            {
                uint8_t start;
                uint8_t size;
            }destroy_fuses[POLICY_MAX_N_OF_DESTR_REGIONS];
            struct /* Ranges of flash to be destroyed before allowing transition to RMA stage */
            {
                uint8_t start;
                uint8_t size;
            }destroy_flash[POLICY_MAX_N_OF_DESTR_REGIONS];
            uint8_t key;
        }rma_policy;
    }present;
}debug_policy_t;

/** Format of key to return from key extraction function */
typedef struct
{
    uint8_t use;
    ec_curve_type crv;
    unsigned char d[KEY_XY_LENGTH];
    unsigned char x[KEY_XY_LENGTH];
    unsigned char y[KEY_XY_LENGTH];
}ec_key_t;

/*
 * Checks conformity of policy in JSON format to JSON schema (template).
 * */
int Cy_JWT_PolicyValidateBySchema(char *policy_str, schema_set_t *templates);

/*
 * Takes entire policy in JSON format and looks for sub-policies inside it.
 * Places pointers to subpolicies in struct subpolicies
 * */
int Cy_JWT_PolicyPackageSplit(char *policy_str, policy_set_t *policy_set);

/*
 * Copies requested by name sub-policy to char *policy buffer.
 * It uses struct subpolicies * filled in Cy_JWT_SplitPolicy()
 * function instead of looking through whole JSON-policy.
 * */
int Cy_JWT_PolicyGetSubpolicy(const char *policy_name, char *policy);

/*
 * Searches for 3 sub-policies inside already parsed JSON-policy and
 * saves pointers to them.
 * */
int Cy_JWT_PolicyParsedSplit(cJSON *policy, policy_set_t *policy_set);

/*
 * Validates JSON-policy using correspondent JSON-schema.
 * */
int Cy_JWT_PolicySingleValidateBySchema(cJSON *json, cJSON *schema);

/*
 * Parses Boot and Upgrade policy in JSON format and returns binary representation of them.
 * */
int Cy_JWT_BnUPolicyParse(cJSON *bnu_json, bnu_policy_t *bnu_policy, uint32_t masterImgID);

/*
 * Set Boot & Upgrade policy structure to default values using corresponding template.
 * */
int Cy_JWT_BnUPolicySetDefault(const char *schema_str, bnu_policy_t *bnu_policy);

/*
 * Parses Debug policy in JSON format and returns binary representation of them.
 * */
int Cy_JWT_DebugPolicyParse(cJSON *debug_json, debug_policy_t *debug_policy);

/*
 * Set Debug policy structure to default values using corresponding template.
 * */
int Cy_JWT_DebugPolicySetDefault(const char *schema_str, debug_policy_t *debug_policy);

/*
 * Parses Wounding policy in JSON format and returns binary representation of them.
 * */
int Cy_JWT_WoundPolicyParse(cJSON *wound_json, wound_policy_t *wound_policy);

 /*
  * Set Wounding policy structure to default values using corresponding template
  * */
int Cy_JWT_WoundingPolicySetDefault(const char *schema_str, wound_policy_t *wounding_policy);


/*
 * Checks if given JWT packet has valid header.
 * */
int Cy_JWT_IsHeadValid(char *jwt_packet, char *alg);

/*
 * Looks over given JWT packet, find where JWT header is located
 * and what length of it is.
 * */
int Cy_JWT_GetStrDataHead(char *jwt_str, char **header_ptr, uint32_t *header_len);

/*
 * Looks over given JWT packet, find where JWT body is located
 * and what length of it is.
 * */
int Cy_JWT_GetStrDataBody(char *jwt_str, char **body_ptr, uint32_t *body_len);

/*
 * Looks over given JWT packet, find where JWT signature is located
 * and what length of it is.
 * */
int Cy_JWT_GetStrDataSig(char *jwt_str, char **sig_ptr, uint32_t *sig_len);

/*
 * Converts string with EC key curve name to enum value.
 * */
ec_curve_type Cy_JWT_GetStrKeyCrv(char *curve);

/*
 * Function Calculates the buffer size required for decoded base64 string storage.
 * */
uint32_t Cy_JWT_GetB64DecodeLen(uint32_t base64_size);

/*
 * Calculates the buffer size required for string encoded into base64.
 * */
uint32_t Cy_JWT_GetB64EncodeLen(uint32_t raw_size);

/*
 * Parses JWK which represents EC key.
 * */
int Cy_JWT_ParseEcKey(cJSON *json, ec_key_t *key);

/*
 * Looks for JSON item by key string. Operates only on one level of nesting.
 * Expects input item to be object.
 * */
int Cy_JWT_ParseStrByKey(cJSON *json, char *val_str, char *key_str);

/*
 * Looks for "alg" value inside given JSON (JWT packet header).
 * */
int Cy_JWT_ParseHeaderAlg(cJSON *json, char *alg_str);

/*
 * Looks for "kty" value inside given JSON (JWT packet header).
 * */
int Cy_JWT_ParseKeyKty(cJSON *json, char *kty_str);

/*
 * Checks if JSON item has a simple type - number, boolean, string.
 * */
bool Cy_JWT_IsJsonItemSimple(cJSON *item);

/*
 * Finds item by key (name) in whole JSON.
 * */
cJSON *Cy_JWT_FindJsonItemByKey(const char* key, cJSON *json);

/*
 * Validates single JSON item by looking for its constraints in schema.
 * */
int Cy_JWT_ValidateItemBySchema(cJSON *item, cJSON *parent, cJSON *schema);

/*
 * Read provisioning packet (JWT), policy templates or public keys strings in
 * JSON format.
 * */
int Cy_JWT_GetProvisioningDetails(uint32_t id, char **ptr, uint32_t *len);

/** Boot & Upgrade policy structure */
extern bnu_policy_t cy_bl_bnu_policy;

#endif /* CY_JWT_POLICY_H */
