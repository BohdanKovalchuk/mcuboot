/***************************************************************************//**
* \file cy_bootloader_services.h
* \version 1.0
*
* \brief
*  This is the header file for implementation of bootloader services.
*
********************************************************************************
* \copyright
* Copyright 2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#ifndef SOURCE_CY_BOOTLOADER_SERVICES_H_
#define SOURCE_CY_BOOTLOADER_SERVICES_H_

#include <stdint.h>

#define CY_BL_CM4_ROM_LOOP_ADDR (0x16004000UL)

/** DAPControl SysCall parameter: access port state */
typedef enum
{
    CY_AP_DIS = 0,
    CY_AP_EN = 1
}cy_ap_control_t;

/** DAPControl SysCall parameter: access port name */
typedef enum
{
    CY_CM0_AP = 0,
    CY_CM4_AP = 1,
    CY_SYS_AP = 2
}cy_ap_name_t;

void Cy_BLServ_FlashInit(void);

void Cy_BLServ_Assert(int expr);

void Cy_BLServ_StartAppCM0p(uint32_t appAddr);
void Cy_BLServ_StartAppCM4(uint32_t appAddr);

int Cy_BLServ_FreeHeap(void);

int Cy_BLServ_AccessPortControl(cy_ap_name_t ap, cy_ap_control_t en);

void Cy_BLServ_SystemInit(void);

#endif /* SOURCE_CY_BOOTLOADER_SERVICES_H_ */
