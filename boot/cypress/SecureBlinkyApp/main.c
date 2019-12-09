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

#include "cy_ipc_drv.h"
#include "cy_scb_uart.h"
#include "cy_syspm.h"

/** SysCall timeout value */
#define SYSCALL_TIMEOUT                 (15000UL)

/** DAPControl SysCall opcode */
#define DAPCONTROL_SYSCALL_OPCODE       (0x3AUL << 24UL)

/** PSA crypto SysCall return codes */
#define CY_FB_SYSCALL_SUCCESS           (0xA0000000UL)

#define CY_SRSS_TST_MODE_ADDR           (SRSS_BASE | 0x0100UL)
#define TST_MODE_TEST_MODE_MASK         (0x80000000UL)
#define TST_MODE_ENTERED_MAGIC          (0x12344321UL)
#define CY_SYS_CM4_PWR_CTL_KEY_OPEN     (0x05FAUL)
#define CY_BL_CM4_ROM_LOOP_ADDR         (0x16004000UL)

#define CM4_APP_START_ADDR              (0x10020400UL)

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

void check_result(int res)
{
    if (res != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }
}

CY_SECTION(".cy_ramfunc") CY_NOINLINE
static void Cy_Utils_SRAMTestBitLoop(void)
{
    while((CY_GET_REG32(CY_SRSS_TST_MODE_ADDR) & TST_MODE_TEST_MODE_MASK) != 0UL);
}

/* rnok: this function is taken from cy_secure_utils until fwsecurity-880 implementation arrive */
static void Cy_Utils_TurnOnCM4(void)
{
    uint32_t regValue;

    regValue = CPUSS->CM4_PWR_CTL & ~(CPUSS_CM4_PWR_CTL_VECTKEYSTAT_Msk | CPUSS_CM4_PWR_CTL_PWR_MODE_Msk);
    regValue |= _VAL2FLD(CPUSS_CM4_PWR_CTL_VECTKEYSTAT, CY_SYS_CM4_PWR_CTL_KEY_OPEN);
    regValue |= CY_SYS_CM4_STATUS_ENABLED;
    CPUSS->CM4_PWR_CTL = regValue;

    while((CPUSS->CM4_STATUS & CPUSS_CM4_STATUS_PWR_DONE_Msk) == 0UL)
    {
        /* Wait for the power mode to take effect */
    }
}

/* rnok: this function is taken from cy_secure_utils until fwsecurity-880 implementation arrive */
int Cy_Utils_AccessPortControl(cy_ap_name_t ap, cy_ap_control_t en)
{
    int rc = -1;
    volatile uint32_t syscallCmd = 0U;
    uint32_t timeout = 0U;

    syscallCmd |= DAPCONTROL_SYSCALL_OPCODE;
    syscallCmd |= (uint8_t)en << 16;
    syscallCmd |= (uint8_t)ap << 8;
    syscallCmd |= 1;

    
    /* Get IPC base register address */
    IPC_STRUCT_Type * ipcStruct = Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_SYSCALL);

    while((CY_IPC_DRV_SUCCESS != Cy_IPC_Drv_LockAcquire(ipcStruct)) &&
            (timeout < SYSCALL_TIMEOUT))
    {
        ++timeout;
    }

    if(timeout < SYSCALL_TIMEOUT)
    {
        timeout = 0U;

        Cy_IPC_Drv_WriteDataValue(ipcStruct, syscallCmd);
        Cy_IPC_Drv_AcquireNotify(ipcStruct, (1<<CY_IPC_CHAN_SYSCALL));

        while((Cy_IPC_Drv_IsLockAcquired(ipcStruct))&&
                (timeout < SYSCALL_TIMEOUT))
        {
            ++timeout;
        }

        if(timeout < SYSCALL_TIMEOUT)
        {
            syscallCmd = Cy_IPC_Drv_ReadDataValue(ipcStruct);
            if(CY_FB_SYSCALL_SUCCESS != syscallCmd)
            {
                rc = syscallCmd;
            }
            else
            {
                rc = 0;
            }
        }
    }
    return rc;
}

void Cy_Utils_StartAppCM4(uint32_t appAddr)
{
    int rc = -1;

    /* Stop if we are in the TEST MODE */
    if((CY_GET_REG32(CY_SRSS_TST_MODE_ADDR) & TST_MODE_TEST_MODE_MASK) != 0UL)
    {
        /* Get IPC base register address */
        IPC_STRUCT_Type * ipcStruct = Cy_IPC_Drv_GetIpcBaseAddress(CY_IPC_CHAN_SYSCALL_DAP);
        Cy_IPC_Drv_WriteDataValue(ipcStruct, TST_MODE_ENTERED_MAGIC);

        printf("[SecureBlinkyApp] - TEST MODE");

        __disable_irq();

        CPUSS->CM4_VECTOR_TABLE_BASE = CY_BL_CM4_ROM_LOOP_ADDR;
        Cy_Utils_TurnOnCM4();

        Cy_SysLib_Delay(1);

        CPUSS->CM4_VECTOR_TABLE_BASE = appAddr;
        Cy_Utils_SRAMTestBitLoop();
        __enable_irq();
    }

    /* It is aligned to 0x400 (256 records in vector table*4bytes each) */
    Cy_SysEnableCM4(appAddr);

    while(1)
    {
        Cy_SysPm_CpuEnterDeepSleep(CY_SYSPM_WAIT_FOR_INTERRUPT);
        // TODO: Cy_Utils_SRAMBusyLoop(); /* use this for PSVP */
    }
}

void start_cm4_app(void)
{
    int rc = CY_RSLT_SUCCESS;

    /* rnok: this function is taken from cy_secure_utils until fwsecurity-880 implementation arrive */
    rc = Cy_Utils_AccessPortControl(CY_CM4_AP, CY_AP_EN);

    if (rc == 0)
    {
        Cy_Utils_StartAppCM4(CM4_APP_START_ADDR);
    }
}

void test_app_init_hardware(void)
{
    int rc = !CY_RSLT_SUCCESS;

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

    test_app_init_hardware();

    printf(GREETING_MESSAGE_INFO);

    for (i = 0; i < CM0_TIMEOUT ; i++)
    {
        /* Toggle the user LED periodically */
        Cy_SysLib_Delay(blinky_period/2);

        /* Invert the USER LED state */
        cyhal_gpio_toggle((cyhal_gpio_t) CYBSP_USER_LED1);
    }

    start_cm4_app();

    return 0;
}
