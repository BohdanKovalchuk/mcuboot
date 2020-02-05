/***************************************************************************//**
* \file cy_smif_psoc6.c
* \version 1.0
*
* \brief
*  This is the source file of external flash driver adoption layer between PSoC6 
*  and standard MCUBoot code.
*
********************************************************************************
* \copyright
*
* (c) 2020, Cypress Semiconductor Corporation
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
* Software source code solely for use in connection with Cypress?s
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
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"

#ifdef MCUBOOT_HAVE_ASSERT_H
#include "mcuboot_config/mcuboot_assert.h"
#else
#include <assert.h>
#endif

#include "flash_map_backend/flash_map_backend.h"
#include <sysflash/sysflash.h>

#include "cy_device_headers.h"
#include "cy_flash_psoc6.h"
#include "cy_flash.h"
#include "cy_syspm.h"

#include "cycfg_qspi_memslot.h"

#include "flash_qspi.h"

#define PSOC6_WR_SUCCESS                    (0)
#define PSOC6_WR_ERROR_INVALID_PARAMETER    (1)
#define PSOC6_WR_ERROR_FLASH_WRITE          (2)

#define PSOC6_FLASH_ERASE_BLOCK_SIZE	CY_FLASH_SIZEOF_ROW /* PSoC6 Flash erases by Row */

int psoc6_smif_read(const struct flash_area *fap,
                                        off_t addr,
                                        void *data,
                                        size_t len)
{   /* TODO: implement it */
	int rc = -1;
    cy_stc_smif_mem_config_t *cfg;
    cy_en_smif_status_t st;

    cfg = qspi_get_memory_config(FLASH_DEVICE_GET_EXT_INDEX(fap->fa_device_id));

    /* convert to offset inside memory device */
    addr = addr - cfg->baseAddress;

    st = Cy_SMIF_MemRead(qspi_get_device(), cfg, addr, data, len, qspi_get_context());
    if (st == CY_SMIF_SUCCESS)
        rc = 0;
	return rc;
}

int psoc6_smif_write(const struct flash_area *fap,
                                        off_t addr,
                                        const void *data,
                                        size_t len)
{   /* TODO: implement it */
	int rc = -1;    
    cy_en_smif_status_t st ;
    cy_stc_smif_mem_config_t *cfg;

    cfg =  qspi_get_memory_config(FLASH_DEVICE_GET_EXT_INDEX(fap->fa_device_id));

    /* convert to offset inside memory device */
    addr = addr - cfg->baseAddress;

    st = Cy_SMIF_MemWrite(qspi_get_device(), cfg, addr, data, len, qspi_get_context());
    if (st == CY_SMIF_SUCCESS)
        rc = 0;
	return rc;
}

int psoc6_smif_erase(off_t addr, size_t size)
{
	int rc = -1;
    
//    /* TODO: implement it */
//    uint8_t addrBuf[4];
//    if(IS_FLASH_SMIF(addr))
//    {
//#ifdef MCUBOOT_USE_SMIF_XIP
//        if(0 != cy_bl_bnu_policy.bnu_img_policy.smif_id)
//        {
//            /* Memory/XIP Read is Done. Switching back to Normal/CMD */
//            Cy_SMIF_SetMode(SMIF0, CY_SMIF_NORMAL);
//        }
//#endif
//        uint8_t zero_buff[SMIF_ZERO_BUFF_SIZE];
//        uint32_t buff_num, rem_num, cur_addr;
//        /* Zeroise data on external Flash instead of Erasing */
//        memset(zero_buff, 0x00, sizeof(zero_buff));
//
//        /* We do not know how big piece of data needs
//         * to be Erased, so splitting it into 256-bytes
//         * chunks to save zero-buffer RAM */
//        buff_num = len/SMIF_ZERO_BUFF_SIZE;
//        rem_num  = len%SMIF_ZERO_BUFF_SIZE;
//
//        /* convert to offset inside memory device */
//        addr = addr - smifMemConfigs[0]->baseAddress;
//
//        for(;((buff_num>0)&&(0 == rc)); buff_num--)
//        {
//            Flash_SMIF_GetAddrBuff(cur_addr, addrBuf);
//
//            rc = Flash_SMIF_WriteMemory(SMIF0       /* SMIF_Type *baseaddr */,
//                                        &QSPIContext/* cy_stc_smif_context_t *smifContext */,
//                                        zero_buff   /* uint8_t txBuffer[] */,
//                                        SMIF_ZERO_BUFF_SIZE     /* uint32_t txSize */,
//                                        addrBuf   /* uint8_t *address */);
//
//            cur_addr += SMIF_ZERO_BUFF_SIZE;
//        }
//
//        if((0 != rem_num)&&(0 == rc))
//        {
//            Flash_SMIF_GetAddrBuff(cur_addr, addrBuf);
//            rc = Flash_SMIF_WriteMemory(SMIF0       /* SMIF_Type *baseaddr */,
//                                        &QSPIContext/* cy_stc_smif_context_t *smifContext */,
//                                        zero_buff   /* uint8_t txBuffer[] */,
//                                        rem_num     /* uint32_t txSize */,
//                                        addrBuf   /* uint8_t *address */);
//        }
//    }
	// TODO:
	rc = 0;
	return rc;
}
