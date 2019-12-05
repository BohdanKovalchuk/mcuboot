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

void Cy_BLServ_FlashInit(void);

void Cy_BLServ_Assert(int expr);

int Cy_BLServ_FreeHeap(void);

void Cy_BLServ_SystemInit(void);

#endif /* SOURCE_CY_BOOTLOADER_SERVICES_H_ */
