/***************************************************************************//**
* \file cy_bootloader_hw.c
* \version 1.0
*
* \brief
*  This is the source code implementation file for bootloader hardware layer.
*
********************************************************************************
* \copyright
* Copyright 2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/
#include "cy_pdl.h"
#include "cyhal.h"
#include "cy_retarget_io.h"
#include "cy_result.h"
#include "cy_device_headers.h"
#include "cy_wdt.h"

#include "cy_bootloader_hw.h"

#include "mcuboot_config/mcuboot_logging.h"

/* Define pins for DEBUG uart port */
#define CY_DEBUG_UART_TX (P5_1)
#define CY_DEBUG_UART_RX (P5_0)

#if defined(CY_BOOTLOADER_DIAGNOSTIC_GPIO)
#define LED_RED_NUM 5U
#define LED_RED_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define LED_RED_INIT_DRIVESTATE 1
#ifndef ioss_0_port_1_pin_5_HSIOM
	#define ioss_0_port_1_pin_5_HSIOM HSIOM_SEL_GPIO
#endif
#define LED_RED_HSIOM ioss_0_port_1_pin_5_HSIOM
#define LED_RED_IRQ ioss_interrupts_gpio_0_IRQn

const cy_stc_gpio_pin_config_t LED_RED_config =
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG_IN_OFF,
	.hsiom = LED_RED_HSIOM,
	.intEdge = CY_GPIO_INTR_DISABLE,
	.intMask = 0UL,
	.vtrip = CY_GPIO_VTRIP_CMOS,
	.slewRate = CY_GPIO_SLEW_FAST,
	.driveSel = CY_GPIO_DRIVE_FULL,
	.vregEn = 0UL,
	.ibufMode = 0UL,
	.vtripSel = 0UL,
	.vrefSel = 0UL,
	.vohSel = 0UL,
};
#endif /* CY_BOOTLOADER_DIAGNOSTIC_GPIO */

void Cy_InitSmifPins(void)
{
    static const cy_stc_gpio_pin_config_t QSPI_SS_config =
    {
        .outVal = 1,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = QSPI_SS_HSIOM,
        .intEdge = CY_GPIO_INTR_DISABLE,
        .intMask = 0UL,
        .vtrip = CY_GPIO_VTRIP_CMOS,
        .slewRate = CY_GPIO_SLEW_FAST,
        .driveSel = CY_GPIO_DRIVE_1_2,
        .vregEn = 0UL,
        .ibufMode = 0UL,
        .vtripSel = 0UL,
        .vrefSel = 0UL,
        .vohSel = 0UL,
    };
    static const cy_stc_gpio_pin_config_t QSPI_DATA3_config =
    {
        .outVal = 1,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = QSPI_DATA3_HSIOM,
        .intEdge = CY_GPIO_INTR_DISABLE,
        .intMask = 0UL,
        .vtrip = CY_GPIO_VTRIP_CMOS,
        .slewRate = CY_GPIO_SLEW_FAST,
        .driveSel = CY_GPIO_DRIVE_1_2,
        .vregEn = 0UL,
        .ibufMode = 0UL,
        .vtripSel = 0UL,
        .vrefSel = 0UL,
        .vohSel = 0UL,
    };
    static const cy_stc_gpio_pin_config_t QSPI_DATA2_config =
    {
        .outVal = 1,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = QSPI_DATA2_HSIOM,
        .intEdge = CY_GPIO_INTR_DISABLE,
        .intMask = 0UL,
        .vtrip = CY_GPIO_VTRIP_CMOS,
        .slewRate = CY_GPIO_SLEW_FAST,
        .driveSel = CY_GPIO_DRIVE_1_2,
        .vregEn = 0UL,
        .ibufMode = 0UL,
        .vtripSel = 0UL,
        .vrefSel = 0UL,
        .vohSel = 0UL,
    };
    static const cy_stc_gpio_pin_config_t QSPI_DATA1_config =
    {
        .outVal = 1,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = QSPI_DATA1_HSIOM,
        .intEdge = CY_GPIO_INTR_DISABLE,
        .intMask = 0UL,
        .vtrip = CY_GPIO_VTRIP_CMOS,
        .slewRate = CY_GPIO_SLEW_FAST,
        .driveSel = CY_GPIO_DRIVE_1_2,
        .vregEn = 0UL,
        .ibufMode = 0UL,
        .vtripSel = 0UL,
        .vrefSel = 0UL,
        .vohSel = 0UL,
    };
    static const cy_stc_gpio_pin_config_t QSPI_DATA0_config =
    {
        .outVal = 1,
        .driveMode = CY_GPIO_DM_STRONG,
        .hsiom = QSPI_DATA0_HSIOM,
        .intEdge = CY_GPIO_INTR_DISABLE,
        .intMask = 0UL,
        .vtrip = CY_GPIO_VTRIP_CMOS,
        .slewRate = CY_GPIO_SLEW_FAST,
        .driveSel = CY_GPIO_DRIVE_1_2,
        .vregEn = 0UL,
        .ibufMode = 0UL,
        .vtripSel = 0UL,
        .vrefSel = 0UL,
        .vohSel = 0UL,
    };
    static const cy_stc_gpio_pin_config_t QSPI_SPI_CLOCK_config =
    {
        .outVal = 1,
        .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
        .hsiom = QSPI_SPI_CLOCK_HSIOM,
        .intEdge = CY_GPIO_INTR_DISABLE,
        .intMask = 0UL,
        .vtrip = CY_GPIO_VTRIP_CMOS,
        .slewRate = CY_GPIO_SLEW_FAST,
        .driveSel = CY_GPIO_DRIVE_1_2,
        .vregEn = 0UL,
        .ibufMode = 0UL,
        .vtripSel = 0UL,
        .vrefSel = 0UL,
        .vohSel = 0UL,
    };

    Cy_GPIO_Pin_Init(QSPI_SS_PORT, QSPI_SS_PIN, &QSPI_SS_config);
    Cy_GPIO_Pin_Init(QSPI_DATA3_PORT, QSPI_DATA3_PIN, &QSPI_DATA3_config);
    Cy_GPIO_Pin_Init(QSPI_DATA2_PORT, QSPI_DATA2_PIN, &QSPI_DATA2_config);
    Cy_GPIO_Pin_Init(QSPI_DATA1_PORT, QSPI_DATA1_PIN, &QSPI_DATA1_config);
    Cy_GPIO_Pin_Init(QSPI_DATA0_PORT, QSPI_DATA0_PIN, &QSPI_DATA0_config);
    Cy_GPIO_Pin_Init(QSPI_SPI_CLOCK_PORT, QSPI_SPI_CLOCK_PIN, &QSPI_SPI_CLOCK_config);
}

void Cy_InitPSoC6_HW(void)
{
    /* enable interrupts */
    __enable_irq();

    /* Disabling watchdog so it will not interrupt normal flow later */
    Cy_WDT_Unlock();
    Cy_WDT_Disable();
#if (MCUBOOT_LOG_LEVEL != MCUBOOT_LOG_LEVEL_OFF)
#if defined(CY_BOOTLOADER_DIAGNOSTIC_GPIO)
    Cy_GPIO_Pin_Init(LED_RED_PORT, LED_RED_PIN, &LED_RED_config);
#endif
    /* Initialize retarget-io to use the debug UART port */
    cy_retarget_io_init(CY_DEBUG_UART_TX, CY_DEBUG_UART_RX, CY_RETARGET_IO_BAUDRATE);
#endif
}
