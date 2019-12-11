/***************************************************************************//**
* \file cyprotection.h
* \version 1.0
*
* \brief
*  This is the header file for the protection configuration structures
*  and functions.
*
********************************************************************************
* \copyright
*
* © 2018, Cypress Semiconductor Corporation
* or a subsidiary of Cypress Semiconductor Corporation. All rights
* reserved.
*
* This software, including source code, documentation and related
* materials ("Software"), is owned by Cypress Semiconductor
* Corporation or one of its subsidiaries ("Cypress") and is protected by
* and subject to worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-
* exclusive, non-transferable license to copy, modify, and compile the
* Software source code solely for use in connection with Cypress�s
* integrated circuit products. Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,
* BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE. Cypress reserves the right to make
* changes to the Software without notice. Cypress does not assume any
* liability arising out of the application or use of the Software or any
* product or circuit described in the Software. Cypress does not
* authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress’s product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*
******************************************************************************/

#ifndef CYPROTECTION_H
#define CYPROTECTION_H

#include "cy_prot.h"
#include "cy_jwt_policy.h"

#define MPU_SMPU_SUBREGIONS_NUMB (8u)
/* MPU is not used yet */
#define CPUSS_PROT_MPU_STRUCT_NR (0u)

#define CPUSS_PROT_PPU_GR_STRUCT_NR (16u)
#define CPUSS_PROT_PPU_PROG_STRUCT_NR (16u)
#define CPUSS_PROT_PPU_FX_SL_STRUCT_NR (16u)
#define CPUSS_PROT_PPU_FX_RG_STRUCT_NR (29u)

#define CPUSS_PROT_PPU_FX_RG_START_ADDR (0x40201000UL)

#define PRIVILEGED_MODE (1u)
#define UNPRIVILEGED_MODE (0u)
#define NONSECURE_MODE (1u)
#define SECURE_MODE (0u)

/* PPU Group existing bitmask - 11001011111 */
#define PERI_PPU_GR_MMIO_EXIST_BITMASK 0x65F
/* PPU MMIO1 Group Fixed Region existing bitmask - 10 */
#define PERI_PPU_GR_MMIO1_EXIST_BITMASK 0x2
/* PPU MMIO2 Group Fixed Region existing bitmask - 11001111111111 */
#define PERI_PPU_GR_MMIO2_EXIST_BITMASK 0x33FF
/* PPU MMIO3 Group Fixed Region existing bitmask - 1111101111111 */
#define PERI_PPU_GR_MMIO3_EXIST_BITMASK 0x1F7F
/* PPU MMIO4 Group Fixed Region existing bitmask - 101 */
#define PERI_PPU_GR_MMIO4_EXIST_BITMASK 0x5
/* PPU MMIO6 Group Fixed Region existing bitmask - 1111111111 */
#define PERI_PPU_GR_MMIO6_EXIST_BITMASK 0x3FF
/* PPU MMIO9 Group Fixed Region existing bitmask - 11 */
#define PERI_PPU_GR_MMIO9_EXIST_BITMASK 0x3
/* PPU MMIO10 Group Fixed Region existing bitmask - 111 */
#define PERI_PPU_GR_MMIO10_EXIST_BITMASK 0x7

/* Only 7 protection contexts are available in PSoC6-BLE2 */
#define ALL_PROTECTION_CONTEXTS_MASK (CY_PROT_PCMASK1 | CY_PROT_PCMASK2 |\
CY_PROT_PCMASK3 | CY_PROT_PCMASK4 | CY_PROT_PCMASK5 | CY_PROT_PCMASK6 |\
CY_PROT_PCMASK7)
#define SECURE_CONTEXTS_MASK (CY_PROT_PCMASK1 | CY_PROT_PCMASK2 |\
CY_PROT_PCMASK3 | CY_PROT_PCMASK4)

#define ALL_SUBREGIONS (0x0)

/* TODO: There is no SWPU configuration part. SWPU has not implemented yet */
typedef struct {
    cy_jwt_smpu_reg_t regName;          /**< Name of SMPU region */
    uint8_t           subregions;       /**< Mask of the 8 subregions to disable (Only applicable to slave) */
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;           /**< Non Secure = 0, Secure = 1 */
    bool              pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t          pcMask;           /**< Mask of allowed protection context(s) */
    PROT_SMPU_SMPU_STRUCT_Type* prot_region; /* protection region */
    cy_en_prot_perm_t userMstPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privMstPermission;   /**< Privileged permissions for the region */
    uint16_t          pcMstMask;           /**< Master Mask of allowed protection context(s) */
} cy_smpu_region_config_t;

/*
* See Cy_Prot_ConfigBusMaster function description for parameters meaning
*
* act_pcMask specifies active PC for Cy_Prot_SetActivePC function
*/
typedef struct {
    en_prot_master_t busMaster;
    bool privileged;
    bool secure;
    uint32_t pcMask;
    uint32_t act_pc;
} cy_bus_master_config_t;

/** Configuration structure for Fixed Group (GR) PPU (PPU_GR) struct initialization */
typedef struct
{
    cy_en_prot_perm_t   userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t   privPermission;   /**< Privileged permissions for the region */
    bool                secure;           /**< Non Secure = 0, Secure = 1 */
    bool                pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t            pcMask;           /**< Mask of allowed protection context(s) */
    cy_en_prot_perm_t   userMstPermission;   /**< Master User permissions for the region */
    cy_en_prot_perm_t   privMstPermission;   /**< Master Privileged permissions for the region */
    bool                secureMst;           /**< Non Secure = 0, Secure = 1 Master */
    uint16_t            pcMstMask;           /**< Master Mask of allowed protection context(s) */
    PERI_PPU_GR_Type    *pPpuStr;           /**< Ppu structure address */
} cy_ppu_fixed_gr_cfg_t;
/** Configuration structure for Fixed Region (RG) PPU (PPU_RG) struct initialization */
typedef struct
{
    cy_en_prot_perm_t   userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t   privPermission;   /**< Privileged permissions for the region */
    bool                secure;           /**< Non Secure = 0, Secure = 1 */
    bool                pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t            pcMask;           /**< Mask of allowed protection context(s) */
    cy_en_prot_perm_t   userMstPermission;   /**< Master User permissions for the region */
    cy_en_prot_perm_t   privMstPermission;   /**< Master Privileged permissions for the region */
    bool                secureMst;           /**< Non Secure = 0, Secure = 1 Master */
    uint16_t            pcMstMask;           /**< Master Mask of allowed protection context(s) */
    PERI_GR_PPU_RG_Type *pPpuStr;           /**< Ppu structure address */
} cy_ppu_fixed_rg_cfg_t;
/** Configuration structure for Fixed Slave (SL) PPU (PPU_SL) struct initialization */
typedef struct
{
    cy_en_prot_perm_t   userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t   privPermission;   /**< Privileged permissions for the region */
    bool                secure;           /**< Non Secure = 0, Secure = 1 */
    bool                pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t            pcMask;           /**< Mask of allowed protection context(s) */
    cy_en_prot_perm_t   userMstPermission;   /**< Master User permissions for the region */
    cy_en_prot_perm_t   privMstPermission;   /**< Master Privileged permissions for the region */
    bool                secureMst;           /**< Non Secure = 0, Secure = 1 Master */
    uint16_t            pcMstMask;           /**< Master Mask of allowed protection context(s) */
    PERI_GR_PPU_SL_Type *pPpuStr;           /**< Ppu structure address */
} cy_ppu_fixed_sl_cfg_t;

/** Configuration structure for Programmable (PROG) PPU (PPU_PR) struct initialization */
typedef struct
{
    uint32_t*         address;          /**< Base address of the memory region (Only applicable to slave) */
    cy_en_prot_size_t regionSize;       /**< Size of the memory region (Only applicable to slave) */
    uint8_t           subregions;       /**< Mask of the 8 subregions to disable (Only applicable to slave) */
    cy_en_prot_perm_t userPermission;   /**< User permissions for the region */
    cy_en_prot_perm_t privPermission;   /**< Privileged permissions for the region */
    bool              secure;           /**< Non Secure = 0, Secure = 1 */
    bool              pcMatch;          /**< Access evaluation = 0, Matching = 1  */
    uint16_t          pcMask;           /**< Mask of allowed protection context(s) */
    cy_en_prot_perm_t userMstPermission;   /**< Master User permissions for the region */
    cy_en_prot_perm_t privMstPermission;   /**< Master Privileged permissions for the region */
    bool              secureMst;           /**< Non Secure = 0, Secure = 1 Master */
    uint16_t          pcMstMask;           /**< Master Mask of allowed protection context(s) */
    PERI_PPU_PR_Type              *pPpuStr;           /**< Ppu structure address */
} cy_ppu_prog_cfg_t;

/** Configuration structure for Fixed PPU v2 struct initialization */
typedef struct
{
    cy_en_prot_perm_t   userPermission;     /**< Slave User permissions for the region */
    cy_en_prot_perm_t   privPermission;     /**< Slave Privileged permissions for the region */
    bool                secure;             /**< Slave Non Secure = 0, Secure = 1 */
    uint16_t            pcMask;             /**< Slave Mask of allowed protection context(s) */
    cy_en_prot_perm_t   userMstPermission;  /**< Master User permissions for the region */
    cy_en_prot_perm_t   privMstPermission;  /**< Master Privileged permissions for the region */
    bool                secureMst;          /**< Non Secure = 0, Secure = 1 Master */
    uint16_t            pcMstMask;          /**< Master Mask of allowed protection context(s) */
    PERI_MS_PPU_FX_Type *pPpuStr;           /**< PPU structure address */
} cy_ppu_fixed_v2_cfg_t;

cy_en_prot_status_t smpu_protect(cy_smpu_region_config_t smpu_config_arr[], uint32_t arr_length);
cy_en_prot_status_t smpu_config_unprotected(cy_stc_smpu_cfg_t *smpu_config_var);
cy_en_prot_status_t ppu_fixed_rg_protect(cy_ppu_fixed_rg_cfg_t ppu_config_arr[], uint32_t arr_length);
cy_en_prot_status_t ppu_fixed_sl_protect(cy_ppu_fixed_sl_cfg_t ppu_config_arr[], uint32_t arr_length);
cy_en_prot_status_t ppu_prog_protect(cy_ppu_prog_cfg_t ppu_config_arr[], uint32_t arr_length);
cy_en_prot_status_t ppu_fixed_gr_protect(cy_ppu_fixed_gr_cfg_t ppu_config_arr[], uint32_t arr_length);
cy_en_prot_status_t ppu_fixed_protect_v2(const cy_ppu_fixed_v2_cfg_t ppuConfigArr[], uint32_t arrLength);

cy_en_prot_status_t apply_protections(void);
cy_en_prot_status_t release_protections(void);

#endif /* CYPROTECTION_H */
