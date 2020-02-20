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
#include "cycfg_clocks.h"
#include "cycfg_peripherals.h"
#include "cycfg_pins.h"
#include "cycfg_routing.h"

#include "cy_pdl.h"
#include "cy_retarget_io_pdl.h"
#include "cy_result.h"
#include "cy_device_headers.h"
#include "cy_wdt.h"

#include "cy_bootloader_hw.h"

#include "mcuboot_config/mcuboot_logging.h"

#include "cy_scb_uart.h"
#include "cy_sysclk.h"

#if (MCUBOOT_LOG_LEVEL != MCUBOOT_LOG_LEVEL_OFF)
/* Define pins for DEBUG uart port */
#define scb_5_HW SCB5

#define ioss_0_port_5_pin_1_ENABLED 1U
#define ioss_0_port_5_pin_1_PORT GPIO_PRT5
#define ioss_0_port_5_pin_1_PORT_NUM 5U
#define ioss_0_port_5_pin_1_PIN 1U
#define ioss_0_port_5_pin_1_NUM 1U
#define ioss_0_port_5_pin_1_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define ioss_0_port_5_pin_1_INIT_DRIVESTATE 1
#define ioss_0_port_5_pin_1_HSIOM P5_1_SCB5_UART_TX
#define ioss_0_port_5_pin_1_IRQ ioss_interrupts_gpio_5_IRQn

const cy_stc_gpio_pin_config_t ioss_0_port_5_pin_1_config =
{
    .outVal = 1,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = ioss_0_port_5_pin_1_HSIOM,
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

static const cy_stc_scb_uart_config_t scb_5_config =
{
    .uartMode = CY_SCB_UART_STANDARD,
    .enableMutliProcessorMode = false,
    .smartCardRetryOnNack = false,
    .irdaInvertRx = false,
    .irdaEnableLowPowerReceiver = false,
    .oversample = 8,
    .enableMsbFirst = false,
    .dataWidth = 8UL,
    .parity = CY_SCB_UART_PARITY_NONE,
    .stopBits = CY_SCB_UART_STOP_BITS_1,
    .enableInputFilter = false,
    .breakWidth = 11UL,
    .dropOnFrameError = false,
    .dropOnParityError = false,
    .receiverAddress = 0x0UL,
    .receiverAddressMask = 0x0UL,
    .acceptAddrInFifo = false,
    .enableCts = false,
    .ctsPolarity = CY_SCB_UART_ACTIVE_LOW,
    .rtsRxFifoLevel = 0UL,
    .rtsPolarity = CY_SCB_UART_ACTIVE_LOW,
    .rxFifoTriggerLevel = 63UL,
    .rxFifoIntEnableMask = 0UL,
    .txFifoTriggerLevel = 63UL,
    .txFifoIntEnableMask = 0UL,
};

cy_stc_scb_uart_context_t cy_scb5_uart_context;
#endif

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
    init_cycfg_clocks();
    init_cycfg_peripherals();
    init_cycfg_pins();

    /* enable interrupts */
    __enable_irq();

    /* Disabling watchdog so it will not interrupt normal flow later */
    Cy_WDT_Unlock();
    Cy_WDT_Disable();

#if (MCUBOOT_LOG_LEVEL != MCUBOOT_LOG_LEVEL_OFF)
#if defined(CY_BOOTLOADER_DIAGNOSTIC_GPIO)
    Cy_GPIO_Pin_Init(LED_RED_PORT, LED_RED_PIN, &LED_RED_config);
#endif

    /* Initialize retarget-io to use the debug UART port (CYBSP_UART_HW) */
    cy_retarget_io_pdl_init(115200u);

#endif
}
