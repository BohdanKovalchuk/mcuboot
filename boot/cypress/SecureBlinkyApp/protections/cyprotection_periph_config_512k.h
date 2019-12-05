/***************************************************************************//**
* \file cyprotection_config.h
* \version 1.0
*
* \brief
*  This is the header with periperal protection configurations defines
*  for PSoC6A2M.
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

const cy_ppu_fixed_v2_cfg_t fixed_ppu_v2_config[] = {
    {   /* PERI_MAIN */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_MAIN,
    },
    {   /* PERI_GR0_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR0_GROUP,
    },
    {   /* PERI_GR1_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR1_GROUP,
    },
    {   /* PERI_GR2_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR2_GROUP,
    },
    {   /* PERI_GR3_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR3_GROUP,
    },
    {   /* PERI_GR4_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR4_GROUP,
    },
    {   /* PERI_GR5_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR5_GROUP,
    },
    {   /* PERI_GR6_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR6_GROUP,
    },
    {   /* PERI_GR9_GROUP */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_GR9_GROUP,
    },
    {   /* PERI_TR */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PERI_TR,
    },
    {   /* CPUSS_CM0 */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_CPUSS_CM0,
    },
    {   /* CPUSS_CM0_INT */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_CPUSS_CM0_INT,
    },
    {   /* IPC_STRUCT3_IPC */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_IPC_STRUCT3_IPC,
    },
    {   /* IPC_STRUCT4_IPC */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_IPC_STRUCT4_IPC,
    },
    {   /* IPC_STRUCT5_IPC */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_IPC_STRUCT5_IPC,
    },
    {   /* IPC_STRUCT6_IPC */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_IPC_STRUCT6_IPC,
    },
    {   /* IPC_INTR_STRUCT1_INTR */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_IPC_INTR_STRUCT1_INTR,
    },
    {   /* IPC_INTR_STRUCT2_INTR */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK - CY_PROT_PCMASK7,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_IPC_INTR_STRUCT2_INTR,
    },
    {   /* PROT_MPU0_MAIN */
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = SECURE_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_PROT_MPU0_MAIN,
    },
    {   /* SRSS_MAIN1 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_SRSS_MAIN1,
    },
    {   /* SRSS_MAIN2 */
        /* This region contains TST_MODE register that is required for
           programming, see FWSECURITY-813
        .userPermission = CY_PROT_PERM_DISABLED,
        .privPermission = CY_PROT_PERM_RW,
        .pcMask = SECURE_CONTEXTS_MASK,
        */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_SRSS_MAIN2,
    },
    {   /* WDT */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_WDT,
    },
    {   /* MAIN, SRSS Multi-Counter WDT */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_MAIN,
    },
    {   /* SRSS_MAIN3 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_SRSS_MAIN3,
    },
    {   /* SRSS_MAIN4 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_SRSS_MAIN4,
    },
    {   /* SRSS_MAIN5 */
        .userPermission = CY_PROT_PERM_RW,
        .privPermission = CY_PROT_PERM_RW,
        .secure = false,
        .pcMask = ALL_PROTECTION_CONTEXTS_MASK,
        .userMstPermission = CY_PROT_PERM_R,
        .privMstPermission = CY_PROT_PERM_RW,
        .secureMst = false,
        .pcMstMask = SECURE_CONTEXTS_MASK,
        .pPpuStr = PERI_MS_PPU_FX_SRSS_MAIN5,
    }
};

#endif /* CYPROTECTION_PERIPH_CONFIG_H */
