/***************************************************************************//**
* \file main.c
* \version 1.0
*
* \brief
* Demonstrates blinking an LED under firmware control. The Cortex-M4 toggles
* the Red LED. The Cortex-M0+ starts the Cortex-M4 and enters sleep.
*
* Compatible Kits:
*    CY8CPROTO-062-4343W
*    CY8CKIT-064S2-4343W
* Migration to CY8CPROTO-062-4343W kit (ModusToolbox IDE):
*   1. Create this project targeting the CY8CPROTO-062-4343W kit.
*   2. Open design.modus file and replicate P0[3] configuration on P13[7]. Give
*      this pin the alias "LED_RED". Disable P0[3].
*   3. Build and Program
*
* Migration to CY8CPROTO-062-4343W kit (command line make):
*   1. Launch the Device Configurator tool from
*      ModusToolbox_1.0\tools\device-configurator-1.0\
*   2. In Device Configurator, open design.modus file
*      (ModusToolbox_1.0\libraries\psoc6sw-1.0\examples\BlinkyLED\design.modus)
*      and replicate P0[3] configuration on P13[7].
*      Give this pin the alias "LED_RED". Disable P0[3].
*   3. Perform "make clean"
*   4. Build and Program the device with "make DEVICE=CY8C624ABZI-D44 program"
*      Note that depending on the method used to program the device, you may
*      need to manually reset it by pressing the SW1 RESET button on the kit.
*   4. Observe the red blinking LED.
*   5. To switch back to CY8CKIT-062-BLE or CY8CKIT-062-WIFI-BT,
*      perform steps 1 through 3 to reconfigure the "LED_RED" to P0[3].
*      Then use "make program".
*
********************************************************************************
* \copyright
* Copyright 2017-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#include "system_psoc6.h"
#include "cy_pdl.h"
#include "cyhal.h"
#include "cy_retarget_io.h"

/* Define pins for UART debug output */

#define CY_DEBUG_UART_TX (P5_1)
#define CY_DEBUG_UART_RX (P5_0)

#if defined(PSOC_064_1M)
#warning "Check if User LED is correct for your target board."
#define LED_PORT GPIO_PRT13
#define LED_PIN 7U
#elif defined(PSOC_064_2M)
#warning "Check if User LED is correct for your target board."
#define LED_PORT GPIO_PRT1
#define LED_PIN 5U
#elif defined(PSOC_064_512K)
#warning "Check if User LED is correct for your target board."
#define LED_PORT GPIO_PRT2
#define LED_PIN 7U
#endif

#define LED_NUM 5U
#define LED_DRIVEMODE CY_GPIO_DM_STRONG_IN_OFF
#define LED_INIT_DRIVESTATE 1

const cy_stc_gpio_pin_config_t LED_config =
{
    .outVal = 1,
    .driveMode = CY_GPIO_DM_STRONG_IN_OFF,
    .hsiom = HSIOM_SEL_GPIO,
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

#ifdef BOOT_IMG
    #define BLINK_PERIOD          (1000u)
    #define GREETING_MESSAGE_VER  "[BlinkyApp] BlinkyApp v1.0 [CM4]\r\n"
    #define GREETING_MESSAGE_INFO "[BlinkyApp] Red led blinks with 1 sec period\r\n"
#elif UPGRADE_IMG
    #define BLINK_PERIOD          (250u)
    #define GREETING_MESSAGE_VER  "[BlinkyApp] BlinkyApp v2.0 [+]\r\n"
    #define GREETING_MESSAGE_INFO "[BlinkyApp] Red led blinks with 0.25 sec period\r\n"
#else
    #error "[BlinkyApp] Please specify type of image: -DBOOT_IMG or -DUPGRADE_IMG\r\n"
#endif

void check_result(int res)
{
    if (res != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

void test_app_init_hardware(void)
{
    /* enable interrupts */
    __enable_irq();

    Cy_PDL_Init(CY_DEVICE_CFG);

    SystemCoreClockUpdate();

    /* Disabling watchdog so it will not interrupt normal flow later */
//#if defined(DEBUG)
    Cy_GPIO_Pin_Init(LED_PORT, LED_PIN, &LED_config);
    /* Initialize retarget-io to use the debug UART port */
    check_result(cy_retarget_io_init(CY_DEBUG_UART_TX, CY_DEBUG_UART_RX,
                                     CY_RETARGET_IO_BAUDRATE));
//#endif


    printf("\n===========================\r\n");
    printf(GREETING_MESSAGE_VER);
    printf("===========================\r\n");

    printf("[BlinkyApp] GPIO initialized \r\n");
    printf("[BlinkyApp] UART initialized \r\n");
    printf("[BlinkyApp] Retarget I/O set to 115200 baudrate \r\n");

}

int main(void)
{
    uint32_t blinky_period = BLINK_PERIOD;

    test_app_init_hardware();

    printf(GREETING_MESSAGE_INFO);

    for (;;)
    {
        /* Toggle the user LED periodically */
        Cy_SysLib_Delay(blinky_period/2);

        /* Invert the USER LED state */
//#if defined(DEBUG)
        Cy_GPIO_Inv(LED_PORT, LED_PIN);
//#endif
    }
    return 0;
}
