/***************************************************************************//**
* \file cyprotection.c
* \version 1.0
*
* \brief
*  This is the source code for the protection unit config functions wrappers.
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
#include "cyprotection.h"

#include "cyprotection_mem_config.h"

#if defined(CYB0644ABZI_S2D44)
    #include "cyprotection_periph_config_2M.h"
#elif defined(CYB06447BZI_D54) || defined(CYB06447BZI_BLD53)
    #include "cyprotection_periph_config.h"
#else
    #error "Unsupported device declared."
#endif

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

/* Silicon revisions for PSoC64A-BLE2 *C and *D */
#define SI_REVISION_C       (0x23)
#define SI_REVISION_D       (0x24)

/* DAP RAM address and size for PSoC64A-BLE2 *C */
#define DAP_RAM_ADDR        (0x802C000U)
#define DAP_RAM_SIZE        (CY_PROT_SIZE_16KB)

extern bnu_policy_t bnu_policy;

/* Helper function to configure SMPU */
cy_en_prot_status_t smpu_protect_region(cy_smpu_region_config_t *smpu_config_struct, uint32_t reg_start, uint32_t reg_size)
{
    cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    cy_stc_smpu_cfg_t smpu_cfg;

    smpu_cfg.address = (uint32_t *)reg_start;
    smpu_cfg.regionSize = (cy_en_prot_size_t)reg_size;
    smpu_cfg.subregions = smpu_config_struct->subregions;
    smpu_cfg.userPermission = smpu_config_struct->userPermission;
    smpu_cfg.privPermission = smpu_config_struct->privPermission;
    smpu_cfg.secure = smpu_config_struct->secure;
    smpu_cfg.pcMatch = smpu_config_struct->pcMatch;
    smpu_cfg.pcMask = smpu_config_struct->pcMask;

    ret = Cy_Prot_ConfigSmpuSlaveStruct(smpu_config_struct->prot_region, &smpu_cfg);
    if (CY_PROT_SUCCESS == ret)
    {
        smpu_cfg.userPermission = smpu_config_struct->userMstPermission;
        smpu_cfg.privPermission = smpu_config_struct->privMstPermission;
        smpu_cfg.pcMask = smpu_config_struct->pcMstMask;
        ret = Cy_Prot_ConfigSmpuMasterStruct(smpu_config_struct->prot_region, &smpu_cfg);
    }
    if (CY_PROT_SUCCESS == ret)
    {
        ret = Cy_Prot_EnableSmpuSlaveStruct(smpu_config_struct->prot_region);
    }
    if (CY_PROT_SUCCESS == ret)
    {
        ret = Cy_Prot_EnableSmpuMasterStruct(smpu_config_struct->prot_region);
    }
    return ret;
}

/* configure SMPU */
cy_en_prot_status_t smpu_protect(cy_smpu_region_config_t smpu_config_arr[], uint32_t arr_length)
{
	cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    uint32_t i;
    uint32_t prot_start, prot_size;

    for (i = 0; i < arr_length; i++)
    {
        if((3 != CPUSS->PROTECTION) && (smpu_config_arr[i].regName < CY_PROT_SRAM_PC0_PRIV))
        {
            continue;
        }

        if(0 != cy_bl_bnu_policy.prot_regions[smpu_config_arr[i].regName].start)
        {
            prot_start = cy_bl_bnu_policy.prot_regions[smpu_config_arr[i].regName].start;
            prot_size = (cy_en_prot_size_t)(cy_bl_bnu_policy.prot_regions[smpu_config_arr[i].regName].size);
            ret = smpu_protect_region(&smpu_config_arr[i],
                                                prot_start,
                                                prot_size);
        }
    }
    
    return ret;
}

/* protect all unconfigured SMPUs */
cy_en_prot_status_t smpu_config_unprotected(cy_stc_smpu_cfg_t *smpu_config_var)
{
    cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    uint32_t i;
    uint32_t att0, att1;

    for (i = 0; i < CPUSS_PROT_SMPU_STRUCT_NR; i++)
    {
        att0 = PROT->SMPU.SMPU_STRUCT[i].ATT0;
        att1 = PROT->SMPU.SMPU_STRUCT[i].ATT1;

        if ((_FLD2VAL(PROT_SMPU_SMPU_STRUCT_ATT0_ENABLED, att0) == 0)
            && (_FLD2VAL(PROT_SMPU_SMPU_STRUCT_ATT1_ENABLED, att1) == 0))
        {
            ret = Cy_Prot_ConfigSmpuMasterStruct(&PROT->SMPU.SMPU_STRUCT[i], smpu_config_var);
            if (ret != CY_PROT_SUCCESS)
            {
                break;
            }
            ret = Cy_Prot_EnableSmpuMasterStruct(&PROT->SMPU.SMPU_STRUCT[i]);
            if (ret != CY_PROT_SUCCESS)
            {
                break;
            }
        }
    }

    return ret;
}

/* configure PPU Fixed Region */
cy_en_prot_status_t ppu_fixed_rg_protect(cy_ppu_fixed_rg_cfg_t ppu_config_arr[], uint32_t arr_length)
{
	cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    uint32_t i;
    cy_stc_ppu_rg_cfg_t ppu_cfg;

    for (i = 0; i < arr_length; i++)
    {
        ppu_cfg.userPermission = ppu_config_arr[i].userPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privPermission;
        ppu_cfg.secure = ppu_config_arr[i].secure;
        ppu_cfg.pcMatch = ppu_config_arr[i].pcMatch;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMask;
        ret = Cy_Prot_ConfigPpuFixedRgSlaveStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ppu_cfg.userPermission = ppu_config_arr[i].userMstPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privMstPermission;
        ppu_cfg.secure = ppu_config_arr[i].secureMst;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMstMask;
        ret = Cy_Prot_ConfigPpuFixedRgMasterStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuFixedRgSlaveStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuFixedRgMasterStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
    }
    
    return ret;
}

/* configure PPU Fixed Slave */
cy_en_prot_status_t ppu_fixed_sl_protect(cy_ppu_fixed_sl_cfg_t ppu_config_arr[], uint32_t arr_length)
{
	cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    uint32_t i;
    cy_stc_ppu_sl_cfg_t ppu_cfg;

    for (i = 0; i < arr_length; i++)
    {
        ppu_cfg.userPermission = ppu_config_arr[i].userPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privPermission;
        ppu_cfg.secure = ppu_config_arr[i].secure;
        ppu_cfg.pcMatch = ppu_config_arr[i].pcMatch;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMask;
        ret = Cy_Prot_ConfigPpuFixedSlSlaveStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ppu_cfg.userPermission = ppu_config_arr[i].userMstPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privMstPermission;
        ppu_cfg.secure = ppu_config_arr[i].secureMst;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMstMask;
        ret = Cy_Prot_ConfigPpuFixedSlMasterStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuFixedSlSlaveStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuFixedSlMasterStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
    }
    
    return ret;
}

/* configure PPU Programmable */
cy_en_prot_status_t ppu_prog_protect(cy_ppu_prog_cfg_t ppu_config_arr[], uint32_t arr_length)
{
	cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    uint32_t i;
    cy_stc_ppu_prog_cfg_t ppu_cfg;

    for (i = 0; i < arr_length; i++)
    {
        ppu_cfg.address = ppu_config_arr[i].address;
        ppu_cfg.regionSize = ppu_config_arr[i].regionSize;
        ppu_cfg.subregions = ppu_config_arr[i].subregions;
        ppu_cfg.userPermission = ppu_config_arr[i].userPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privPermission;
        ppu_cfg.secure = ppu_config_arr[i].secure;
        ppu_cfg.pcMatch = ppu_config_arr[i].pcMatch;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMask;
        ret = Cy_Prot_ConfigPpuProgSlaveStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ppu_cfg.userPermission = ppu_config_arr[i].userMstPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privMstPermission;
        ppu_cfg.secure = ppu_config_arr[i].secureMst;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMstMask;
        ret = Cy_Prot_ConfigPpuProgMasterStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuProgSlaveStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuProgMasterStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
    }
    
    return ret;
}

/* configure PPU Fixed Group */
cy_en_prot_status_t ppu_fixed_gr_protect(cy_ppu_fixed_gr_cfg_t ppu_config_arr[], uint32_t arr_length)
{
	cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    uint32_t i;
    cy_stc_ppu_gr_cfg_t ppu_cfg;

    for (i = 0; i < arr_length; i++)
    {
        ppu_cfg.userPermission = ppu_config_arr[i].userPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privPermission;
        ppu_cfg.secure = ppu_config_arr[i].secure;
        ppu_cfg.pcMatch = ppu_config_arr[i].pcMatch;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMask;
        ret = Cy_Prot_ConfigPpuFixedGrSlaveStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ppu_cfg.userPermission = ppu_config_arr[i].userMstPermission;
        ppu_cfg.privPermission = ppu_config_arr[i].privMstPermission;
        ppu_cfg.secure = ppu_config_arr[i].secureMst;
        ppu_cfg.pcMask = ppu_config_arr[i].pcMstMask;
        ret = Cy_Prot_ConfigPpuFixedGrMasterStruct(ppu_config_arr[i].pPpuStr, &ppu_cfg);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuFixedGrSlaveStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
        ret = Cy_Prot_EnablePpuFixedGrMasterStruct(ppu_config_arr[i].pPpuStr);
        if (ret != CY_PROT_SUCCESS)
        {
            break;
        }
    }
    
    return ret;
}

/* Configure Fixed PPU v2 */
cy_en_prot_status_t ppu_fixed_protect_v2(const cy_ppu_fixed_v2_cfg_t ppuConfigArr[], uint32_t arrLength)
{
    uint32_t i;
    cy_en_prot_status_t ret = CY_PROT_SUCCESS;

    for (i = 0; i < arrLength; i++)
    {
        ret = Cy_Prot_ConfigPpuFixedSlaveAtt(ppuConfigArr[i].pPpuStr,
                                             ppuConfigArr[i].pcMask,
                                             ppuConfigArr[i].userPermission,
                                             ppuConfigArr[i].privPermission,
                                             ppuConfigArr[i].secure);
        if(CY_PROT_SUCCESS != ret)
        {
            break;
        }

        ret = Cy_Prot_ConfigPpuFixedSlaveAtt(ppuConfigArr[i].pPpuStr,
                                             ppuConfigArr[i].pcMask ^ ALL_PROTECTION_CONTEXTS_MASK,
                                             CY_PROT_PERM_R,
                                             CY_PROT_PERM_R,
                                             false);
        if(CY_PROT_SUCCESS != ret)
        {
            break;
        }

        ret = Cy_Prot_ConfigPpuFixedMasterAtt(ppuConfigArr[i].pPpuStr,
                                              ppuConfigArr[i].pcMstMask,
                                              ppuConfigArr[i].userMstPermission,
                                              ppuConfigArr[i].privMstPermission,
                                              ppuConfigArr[i].secureMst);
        if(CY_PROT_SUCCESS != ret)
        {
            break;
        }

        ret = Cy_Prot_ConfigPpuFixedMasterAtt(ppuConfigArr[i].pPpuStr,
                                              ppuConfigArr[i].pcMstMask ^ ALL_PROTECTION_CONTEXTS_MASK,
                                              CY_PROT_PERM_R,
                                              CY_PROT_PERM_R,
                                              false);
        if(CY_PROT_SUCCESS != ret)
        {
            break;
        }
    }

    return ret;
}

cy_en_prot_status_t dap_ram_protect(cy_smpu_region_config_t *dap_ram_config)
{
    cy_en_prot_status_t ret = CY_PROT_SUCCESS;
    cy_stc_smpu_cfg_t smpu_cfg;

    smpu_cfg.address = (uint32_t*)DAP_RAM_ADDR;
    smpu_cfg.regionSize = DAP_RAM_SIZE;
    smpu_cfg.subregions = dap_ram_config->subregions;
    smpu_cfg.userPermission = dap_ram_config->userPermission;
    smpu_cfg.privPermission = dap_ram_config->privPermission;
    smpu_cfg.secure = dap_ram_config->secure;
    smpu_cfg.pcMatch = dap_ram_config->pcMatch;
    smpu_cfg.pcMask = dap_ram_config->pcMask;
    ret = Cy_Prot_ConfigSmpuSlaveStruct(dap_ram_config->prot_region, &smpu_cfg);
    if (ret == CY_PROT_SUCCESS)
    {
        smpu_cfg.userPermission = dap_ram_config->userMstPermission;
        smpu_cfg.privPermission = dap_ram_config->privMstPermission;
        smpu_cfg.pcMask = dap_ram_config->pcMstMask;
        ret = Cy_Prot_ConfigSmpuMasterStruct(dap_ram_config->prot_region, &smpu_cfg);
        if (ret == CY_PROT_SUCCESS)
        {
            ret = Cy_Prot_EnableSmpuSlaveStruct(dap_ram_config->prot_region);
            if (ret == CY_PROT_SUCCESS)
            {
                ret = Cy_Prot_EnableSmpuMasterStruct(dap_ram_config->prot_region);
            }
        }
    }

    return ret;
}

/* Apply all protection settings */
cy_en_prot_status_t apply_protections(void)
{
    cy_en_prot_status_t rc = CY_PROT_SUCCESS;

#if defined(CYB06447BZI_D54) || defined(CYB06447BZI_BLD53)
    rc = ppu_fixed_rg_protect((cy_ppu_fixed_rg_cfg_t*)fixed_rg_ppu_config, NELEMS(fixed_rg_ppu_config));
    if(0 == rc)
    {
        rc = ppu_fixed_sl_protect((cy_ppu_fixed_sl_cfg_t*)fixed_sl_ppu_config, NELEMS(fixed_sl_ppu_config));
    }

    if(0 == rc)
    {
        rc = ppu_prog_protect((cy_ppu_prog_cfg_t*)prog_ppu_config, NELEMS(prog_ppu_config));
    }

    /* TODO: Temporary commented, because it is configured by FlashBoot to fix some silicon issues,
     * see flashboot\source\lib_flashboot.c Cy_FB_ApplyProtection.
    if(0 == rc)
    {
        rc = ppu_fixed_gr_protect((cy_ppu_fixed_gr_cfg_t*)fixed_gr_ppu_config, NELEMS(fixed_gr_ppu_config));
    }*/

    if(SI_REVISION_C == SFLASH_SI_REVISION_ID)
    {
        if(0 == rc)
        {
            rc = dap_ram_protect((cy_smpu_region_config_t*)&smpu_config[0]);
        }
    }
#elif defined(CYB0644ABZI_S2D44) /* || defined(P64-512K) */
    rc = ppu_fixed_protect_v2((cy_ppu_fixed_v2_cfg_t*)fixed_ppu_v2_config, NELEMS(fixed_ppu_v2_config));
#endif

    if(0 == rc)
    {
        rc = smpu_protect((cy_smpu_region_config_t*)smpu_config, NELEMS(smpu_config));
    }

    if(0 == rc)
    {
        rc = smpu_config_unprotected((cy_stc_smpu_cfg_t*)&default_smpu_master_config);
    }

    return rc;
}

/* Release protection settings for CyBootloader RAM */
cy_en_prot_status_t release_protections(void)
{
    cy_en_prot_status_t rc = CY_PROT_SUCCESS;

    extern const volatile uint32_t *_ram_start;
    extern const volatile uint32_t *_ram_end;

    uint32_t size, reg_size;

    size = (uint32_t)(&_ram_end)-(uint32_t)(&_ram_start);

    reg_size = 0;
    /* check if size was 2^N */
    if ((0 != size) && ((size & (size - 1)) == 0))
    {   /* calculate regions size number basing on bytes count */
        while(2 != size)
        {
            reg_size++;
            size >>= 1;
        }
        rc = smpu_protect_region((cy_smpu_region_config_t*)(&smpu_ram_free), (uint32_t)(&_ram_start), reg_size);
    }
    return rc;
}
