/***************************************************************************//**
* \file cyprotection_config.h
* \version 1.0
*
* \brief
*  This is the header with peripheral protection configurations defines 
*  for PSoC6ABLE2.
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

#ifndef CYPROTECTION_PERIPH_CONFIG_H
#define CYPROTECTION_PERIPH_CONFIG_H

#include <stdint.h>
#include "cyprotection.h"

const cy_ppu_fixed_rg_cfg_t fixed_rg_ppu_config[] = {
    {   /* IPC_STRUCT3 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_IPC_STRUCT3,
    },
    {   /* IPC_STRUCT4 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_IPC_STRUCT4,
    },
    {   /* IPC_STRUCT5 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_IPC_STRUCT5,
    },
    {   /* IPC_STRUCT6 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_IPC_STRUCT6,
    },
    {   /* IPC_INTR_STRUCT1 */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_IPC_INTR_STRUCT1,
    },
    {   /* IPC_INTR_STRUCT2 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_IPC_INTR_STRUCT2,
    },
    {   /* CPUSS-M4.PROT.SMPU */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_SMPU,
    },
    {   /* CPUSS-M4.PROT.MPU_CM0P */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_MPU_CM0P,
    },
    {   /* CPUSS-M4.PROT.MPU_CRYPTO */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_MPU_CRYPTO,
    },
    {   /* CPUSS-M4.PROT.MPU_TC */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_MPU_TC,
    },
    /* Commented out according to FWSECURITY-293, remove after final approval
    {    DW1_DW_CH_STRUCT0
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_DW1_DW_CH_STRUCT0,
    },*/
    /* Commented out according to FWSECURITY-293, remove after final approval
    {    DW1_DW_CH_STRUCT1
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_DW1_DW_CH_STRUCT1,
    },*/
    /* Commented out according to FWSECURITY-293, remove after final approval
    {    DW1_DW_CH_STRUCT2
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_DW1_DW_CH_STRUCT2,
    },*/
    /* Commented out according to FWSECURITY-293, remove after final approval
    {    DW1_DW_CH_STRUCT3
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_RG_DW1_DW_CH_STRUCT3,
    }*/
};

const cy_ppu_fixed_sl_cfg_t fixed_sl_ppu_config[] = {
    /* Commented out according to FWSECURITY-293, remove after final approval
    {    CPUSS-M4.CRYPTO
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_CRYPTO,
    },*/
    {   /* CPUSS-M4.CPUSS */
        /* Does not work with these settings
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        /* Used less secure option to have CM4 working */
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_CPUSS,
    },
    {   /* CPUSS-M4.IPC */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_IPC,
    },
    {   /* CPUSS-M4.PROT */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_PROT,
    },
    {   /* SRSS.SRSS */
        /*
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        /* Used less secure option to have CM4 working */
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_SRSS,
    },/* SRSS.BACKUP. SAS - Needs a secure driver to support secure time. Alternate option, no secure time
    {
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_BACKUP,
    },*/
    /* Commented out according to FWSECURITY-293, remove after final approval
    {    CPUSS-M4.DW[1]
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_DW1,
    },*/
    /* Needs a secure driver to enforce protection settings
    {
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_GR_PPU_SL_SMIF,
    }*/
};

const cy_ppu_prog_cfg_t prog_ppu_config[] = {
    {   /* PERI.PERI_GROUP_STRUCT[] */
        .address = (uint32_t *)PERI_GR0,
        .regionSize = CY_PROT_SIZE_1KB,/* 0x00400 */
        .subregions = ALL_SUBREGIONS,
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_PPU_PR4,
    }
};

const cy_ppu_fixed_gr_cfg_t fixed_gr_ppu_config[] = {
    {   /* PERI.PERI */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMatch = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_PPU_GR0,
    }
};

#endif /* CYPROTECTION_PERIPH_CONFIG_H */
