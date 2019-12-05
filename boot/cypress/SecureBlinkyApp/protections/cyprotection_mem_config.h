/***************************************************************************//**
* \file cyprotection_config.h
* \version 1.0
*
* \brief
*  This is the header with memory protection configurations defines.
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
* Software source code solely for use in connection with Cypress's
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
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*
******************************************************************************/

#ifndef CYPROTECTION_MEM_CONFIG_H
#define CYPROTECTION_MEM_CONFIG_H

#include <stdint.h>
#include "cyprotection.h"

const cy_smpu_region_config_t smpu_config[] = {
    {   /* DAP_RAM - This region is used to pass parameters to SysCalls if they
           are called by DAP, must be opened for DAP */
        .regName = CY_PROT_SRAM_DAP,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RWX,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT10,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* FLASH_PC1_SPM */
        .regName = CY_PROT_FLASH_PC1_SPM,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RWX,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = CY_PROT_PCMASK1 + CY_PROT_PCMASK2,
        .prot_region = PROT_SMPU_SMPU_STRUCT4,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* FLASH_PC2 */
        .regName = CY_PROT_FLASH_PC2,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RWX,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = CY_PROT_PCMASK1 + CY_PROT_PCMASK2, /* PC1 needs access to FLASH_PC2 because CyBootloader reads boot area of PC2 app */
        .prot_region = PROT_SMPU_SMPU_STRUCT3,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* FLASH_PC3 */
        .regName = CY_PROT_FLASH_PC3,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RWX,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT2,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
	/* PC4 is not supported due to absence of available SMPUs for FLASH and SRAM_PC4
    {    FLASH_PC4
        .regName = CY_PROT_FLASH_PC4,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RWX,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },*/
    {   /* FLASH_MAIN_XO */
        .regName = CY_PROT_FLASH_MAIN_XO,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RX,
        .privPermission = CY_PROT_PERM_RX,
        .secure = false,
        .pcMatch = true,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK1,
        .prot_region = PROT_SMPU_SMPU_STRUCT1,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* FLASH_MAIN_DATA */
        .regName = CY_PROT_FLASH_MAIN_DATA,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = true,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK1,
        .prot_region = PROT_SMPU_SMPU_STRUCT0,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* SRAM_SPM_PRIV - must include SRAM_SPM_PUB area */
        .regName = CY_PROT_SRAM_SPM_PRIV,
        .subregions = 0xC0,
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT8,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {    /* SRAM_SPM_PUB - must be a neighbor of SRAM_SPM_PRIV */
        .regName = CY_PROT_SRAM_SPM_PUB,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_R,
        .privPermission = CY_PROT_PERM_R,
        .secure = false,
        .pcMatch = true,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - SECURE_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT9,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* SRAM_PC1_PRIV */
        .regName = CY_PROT_SRAM_PC1_PRIV,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = CY_PROT_PCMASK1,
        .prot_region = PROT_SMPU_SMPU_STRUCT7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* SRAM_PC2_PRIV */
        .regName = CY_PROT_SRAM_PC2_PRIV,
        .subregions = 0xC0 /*ALL_SUBREGIONS*/,
        .userPermission = CY_PROT_PERM_RWX,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT6,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
    {   /* SRAM_PC3_PRIV */
        .regName = CY_PROT_SRAM_PC3_PRIV,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = CY_PROT_PCMASK3,
        .prot_region = PROT_SMPU_SMPU_STRUCT5,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    },
	/* PC4 is not supported due to absence of available SMPUs for FLASH and SRAM_PC4
    {   SRAM_PC4_PRIV
        .regName = CY_PROT_SRAM_PC4_PRIV,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = CY_PROT_PCMASK4,
        .prot_region = PROT_SMPU_SMPU_STRUCT,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    }*/
};

const cy_smpu_region_config_t smpu_ram_free[] = {
    {
        .regName = CY_PROT_SRAM_SPM_PRIV,
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RWX,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .prot_region = PROT_SMPU_SMPU_STRUCT8,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .pcMstMask = CY_PROT_PCMASK1,
    }
};

/* Only privileged PC=1 master can change the SMPUs */
const cy_stc_smpu_cfg_t default_smpu_master_config = {
    .address = NULL, /* Not used */
    .regionSize = CY_PROT_SIZE_256B, /* Not used */
    .subregions = ALL_SUBREGIONS, /* Not used */
    .userPermission = CY_PROT_PERM_R,
    .privPermission = CY_PROT_PERM_RW,
    .secure = false,
    .pcMatch = false,
    .pcMask = CY_PROT_PCMASK1,
};

#endif /* CYPROTECTION_MEM_CONFIG_H */
