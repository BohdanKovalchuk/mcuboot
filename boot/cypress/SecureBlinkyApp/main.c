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
#include "cybsp.h"
#include "cy_retarget_io.h"

#include "cy_secure_utils.h"
#include "cy_jwt_policy.h"
#include "cy_jwt_bnu_policy.h"

#include "cyprotection.h"

#define CY_SRSS_TST_MODE_ADDR           (SRSS_BASE | 0x0100UL)
#define TST_MODE_TEST_MODE_MASK         (0x80000000UL)
#define TST_MODE_ENTERED_MAGIC          (0x12344321UL)
#define CY_SYS_CM4_PWR_CTL_KEY_OPEN     (0x05FAUL)
#define CY_BL_CM4_ROM_LOOP_ADDR         (0x16004000UL)

#define CM4_APP_HEADER_SIZE             (0x400UL)

#define MASTER_IMG_ID                   (1)

#define GREETING_MESSAGE          "[SecureBlinkyApp]"
#ifdef BOOT_IMG
    #define BLINK_PERIOD          (250u)
    #define CM0_TIMEOUT           (40u)
    #define GREETING_MESSAGE_VER  "[SecureBlinkyApp] SecureBlinkyApp v1.0 [CM0p]\r\n"
    #define GREETING_MESSAGE_INFO "[SecureBlinkyApp] Red led blinks FAST for 10 sec\r\n\
[SecureBlinkyApp] Then CM4 app will be started\r\n"
#elif UPGRADE_IMG
    #define BLINK_PERIOD          (1000u)
    #define CM0_TIMEOUT           (10u)
    #define GREETING_MESSAGE_VER  "[SecureBlinkyApp] SecureBlinkyApp v2.0 [+]\r\n"
    #define GREETING_MESSAGE_INFO "[SecureBlinkyApp] Red led blinks SLOW for 10 sec\r\n \
[SecureBlinkyApp] Then CM4 app will be started\r\n"
#else
    #error "[SecureBlinkyApp] Please specify type of image: -DBOOT_IMG or -DUPGRADE_IMG\r\n"
#endif

/** SecureBoot policies*/
/** Boot & Upgrade policy structure */
bnu_policy_t cy_bl_bnu_policy;

/** Debug policy structure */
debug_policy_t debug_policy;

void check_result(int res)
{
    if (res != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

void test_app_init_hardware(void)
{
    cybsp_init();

    /* enable interrupts */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port */
    check_result(cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                                     CY_RETARGET_IO_BAUDRATE));

    printf("======================================\r\n");
    printf(GREETING_MESSAGE_VER);
    printf("======================================\r\n");

    /* Initialize the User LED */
    check_result(cyhal_gpio_init((cyhal_gpio_t) CYBSP_USER_LED1, CYHAL_GPIO_DIR_OUTPUT,
                                 CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF));

    printf("\r[SecureBlinkyApp] GPIO initialized \r\n");
    printf("[SecureBlinkyApp] UART initialized \r\n");
    printf("[SecureBlinkyApp] Retarget I/O set to 115200 baudrate \r\n");

}

int main(void)
{
    uint32_t blinky_period = BLINK_PERIOD;
    uint32_t i;
    uint32_t jwtLen;
    char *jwt;
    int rc = 0;
    uint32_t app_addr = 0;

    test_app_init_hardware();

    printf(GREETING_MESSAGE_INFO);

    /* Processing of policy in JWT format */
    rc = Cy_JWT_GetProvisioningDetails(FB_POLICY_JWT, &jwt, &jwtLen);
    if(0 == rc)
    {
        rc = Cy_JWT_ParseProvisioningPacket(jwt, &cy_bl_bnu_policy, &debug_policy,
                MASTER_IMG_ID);
    }

    if(0 != rc)
    {
        printf("%s Policy parsing failed with code 0x%08x\n\r", GREETING_MESSAGE, rc);
    }

    app_addr = cy_bl_bnu_policy.bnu_img_policy[1].boot_area.start + CM4_APP_HEADER_SIZE;
    printf("%s CM4 app address 0x%08lx\n\r", GREETING_MESSAGE, app_addr);
    printf("%s Memory regions to protect:\n\r", GREETING_MESSAGE);
    for(i = 0; i < POLICY_MAX_N_OF_PROT_REGIONS; i++)
    {
        if(0 != cy_bl_bnu_policy.prot_regions[i].start)
        {
            printf("address: 0x%08lx; size 0x%08lx;\n\r",
                   cy_bl_bnu_policy.prot_regions[i].start,
                   (uint32_t)(1 << (cy_bl_bnu_policy.prot_regions[i].size + 1)));
        }
    }

    apply_protections();

    for (i = 0; i < CM0_TIMEOUT ; i++)
    {
        /* Toggle the user LED periodically */
        Cy_SysLib_Delay(blinky_period/2);

        /* Invert the USER LED state */
        cyhal_gpio_toggle((cyhal_gpio_t) CYBSP_USER_LED1);
    }

    Cy_Utils_StartAppCM4(app_addr);

    return 0;
}
