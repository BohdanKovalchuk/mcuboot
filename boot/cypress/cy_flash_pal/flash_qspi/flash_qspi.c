#include "cy_pdl.h"
#include <stdio.h>
#include "flash_qspi.h"

////////////////////////////////////////////////////////////////
//
// This is the board specific stuff that should align with
// your board.
//
//
// QSPI resources:
// SS  - P11_2
// D3  - P11_3
// D2  - P11_4
// D1  - P11_5
// D0  - P11_6
// SCK - P11_7
// SMIF Block - SMIF0
//

static GPIO_PRT_Type *SS0Port = GPIO_PRT11;
static int SS0Pin = 2;
static en_hsiom_sel_t SS0MuxPort = P11_2_SMIF_SPI_SELECT0;

static GPIO_PRT_Type *SS1Port = GPIO_PRT11;
static int SS1Pin = 0;
static en_hsiom_sel_t SS1MuxPort = P11_0_SMIF_SPI_SELECT2;

static GPIO_PRT_Type *D3Port = GPIO_PRT11;
static int D3Pin = 3;
static en_hsiom_sel_t D3MuxPort = P11_3_SMIF_SPI_DATA3;

static GPIO_PRT_Type *D2Port = GPIO_PRT11;
static int D2Pin = 4;
static en_hsiom_sel_t D2MuxPort = P11_4_SMIF_SPI_DATA2;

static GPIO_PRT_Type *D1Port = GPIO_PRT11;
static int D1Pin = 5;
static en_hsiom_sel_t D1MuxPort = P11_5_SMIF_SPI_DATA1;

static GPIO_PRT_Type *D0Port = GPIO_PRT11;
static int D0Pin = 6;
static en_hsiom_sel_t D0MuxPort = P11_6_SMIF_SPI_DATA0;

static GPIO_PRT_Type *SCKPort = GPIO_PRT11;
static int SCKPin = 7;
static en_hsiom_sel_t SCKMuxPort = P11_7_SMIF_SPI_CLK;

static SMIF_Type *QSPIPort  = SMIF0;

////////////////////////////////////////////////////////////////

//
// Used for printing information about the discovered device
//
cy_stc_smif_mem_cmd_t sfdpcmd =
{
    .command = 0x5A,
    .cmdWidth = CY_SMIF_WIDTH_SINGLE,
    .addrWidth = CY_SMIF_WIDTH_SINGLE,
    .mode = 0xFFFFFFFFU,
    .dummyCycles = 8,
    .dataWidth = CY_SMIF_WIDTH_SINGLE,
};

static cy_stc_smif_mem_cmd_t rdcmd0;
static cy_stc_smif_mem_cmd_t wrencmd0;
static cy_stc_smif_mem_cmd_t wrdiscmd0;
static cy_stc_smif_mem_cmd_t erasecmd0;
static cy_stc_smif_mem_cmd_t chiperasecmd0;
static cy_stc_smif_mem_cmd_t pgmcmd0;
static cy_stc_smif_mem_cmd_t readsts0;
static cy_stc_smif_mem_cmd_t readstsqecmd0;
static cy_stc_smif_mem_cmd_t writestseqcmd0;

static cy_stc_smif_mem_device_cfg_t dev_sfdp_0 =
{
    .numOfAddrBytes = 4,
    .readSfdpCmd = &sfdpcmd,
    .readCmd = &rdcmd0,
    .writeEnCmd = &wrencmd0,
    .writeDisCmd = &wrdiscmd0,
    .programCmd = &pgmcmd0,
    .eraseCmd = &erasecmd0,
    .chipEraseCmd = &chiperasecmd0,
    .readStsRegWipCmd = &readsts0,
    .readStsRegQeCmd = &readstsqecmd0,
    .writeStsRegQeCmd = &writestseqcmd0,
};

//static cy_stc_smif_mem_cmd_t rdcmd1;
//static cy_stc_smif_mem_cmd_t wrencmd1;
//static cy_stc_smif_mem_cmd_t wrdiscmd1;
//static cy_stc_smif_mem_cmd_t erasecmd1;
//static cy_stc_smif_mem_cmd_t chiperasecmd1;
//static cy_stc_smif_mem_cmd_t pgmcmd1;
//static cy_stc_smif_mem_cmd_t readsts1;
//static cy_stc_smif_mem_cmd_t readstsqecmd1;
//static cy_stc_smif_mem_cmd_t writestseqcmd1;
//
//static cy_stc_smif_mem_device_cfg_t dev1 =
//{
//    .numOfAddrBytes = 0,
//    .readSfdpCmd = &sfdpcmd,
//    .readCmd = &rdcmd1,
//    .writeEnCmd = &wrencmd1,
//    .writeDisCmd = &wrdiscmd1,
//    .programCmd = &pgmcmd1,
//    .eraseCmd = &erasecmd1,
//    .chipEraseCmd = &chiperasecmd1,
//    .readStsRegWipCmd = &readsts1,
//    .readStsRegQeCmd = &readstsqecmd1,
//    .writeStsRegQeCmd = &writestseqcmd1,
//};

static cy_stc_smif_mem_config_t mem_sfdp_0 =
{
    /* The base address the memory slave is mapped to in the PSoC memory map.
    Valid when the memory-mapped mode is enabled. */
    .baseAddress = 0x18000000U,
    /* The size allocated in the PSoC memory map, for the memory slave device.
    The size is allocated from the base address. Valid when the memory mapped mode is enabled. */
//    .memMappedSize = 0x4000000U,
    /* If this memory device is one of the devices in the dual quad SPI configuration.
    Valid when the memory mapped mode is enabled. */
//    .dualQuadSlots = 0,
    .flags = CY_SMIF_FLAG_DETECT_SFDP,
    .slaveSelect = CY_SMIF_SLAVE_SELECT_0,
    .dataSelect = CY_SMIF_DATA_SEL0,
    .deviceCfg = &dev_sfdp_0
};

//static cy_stc_smif_mem_config_t mem_sfdp_s1 =
//{
//    .flags = CY_SMIF_FLAG_DETECT_SFDP,
//    .slaveSelect = CY_SMIF_SLAVE_SELECT_2,
//    .dataSelect = CY_SMIF_DATA_SEL0,
//    .deviceCfg = &dev1
//};
//
cy_stc_smif_mem_config_t *mems_sfdp[2] =
{
    &mem_sfdp_0,
//    &mem_sfdp_1,
};

static cy_stc_smif_block_config_t smifBlockConfig_sfdp =
{
    .memCount = 1,
    .memConfig = mems_sfdp,
};

static cy_stc_smif_block_config_t *smif_blk_config;

static cy_stc_smif_context_t QSPI_context;

cy_stc_smif_config_t const QSPI_config =
{
    .mode = CY_SMIF_NORMAL,
    .deselectDelay = 1,
    .rxClockSel = CY_SMIF_SEL_INV_INTERNAL_CLK,
    .blockEvent = CY_SMIF_BUS_ERROR
};

cy_stc_sysint_t smifIntConfig =
{
//    .intrSrc =  smif_interrupt_IRQn,
        // TODO: check which is valid for CM0p
    .intrSrc = NvicMux7_IRQn,
    .cm0pSrc = smif_interrupt_IRQn,
    .intrPriority = 1
};

const cy_stc_gpio_pin_config_t QSPI_SS_config = 
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG_IN_OFF,
	.hsiom = P11_2_SMIF_SPI_SELECT0,
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
   
const cy_stc_gpio_pin_config_t QSPI_DATA3_config = 
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG,
	.hsiom = P11_3_SMIF_SPI_DATA3,
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
const cy_stc_gpio_pin_config_t QSPI_DATA2_config = 
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG,
	.hsiom = P11_4_SMIF_SPI_DATA2,
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
const cy_stc_gpio_pin_config_t QSPI_DATA1_config = 
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG,
	.hsiom = P11_5_SMIF_SPI_DATA1,
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
const cy_stc_gpio_pin_config_t QSPI_DATA0_config = 
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG,
	.hsiom = P11_6_SMIF_SPI_DATA0,
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
const cy_stc_gpio_pin_config_t QSPI_SCK_config = 
{
	.outVal = 1,
	.driveMode = CY_GPIO_DM_STRONG_IN_OFF,
	.hsiom = P11_7_SMIF_SPI_CLK,
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

void Isr_SMIF(void)
{
    Cy_SMIF_Interrupt(QSPIPort, &QSPI_context);
}

static void failed(const char *fun, cy_en_smif_status_t st)
{
    printf("%s", fun);
    printf(" failed - ");
    switch(st)
    {
        case CY_SMIF_SUCCESS:
            printf("CY_SMIF_SUCCESS");
            break;
        case CY_SMIF_CMD_FIFO_FULL:
            printf("CY_SMIF_CMD_FIFO_FULL");
            break;
        case CY_SMIF_EXCEED_TIMEOUT:
            printf("CY_SMIF_EXCEED_TIMEOUT");
            break;
        case CY_SMIF_NO_QE_BIT:
            printf("CY_SMIF_NO_QE_BIT");
            break;
        case CY_SMIF_BAD_PARAM:
            printf("CY_SMIF_BAD_PARAM");
            break;
        case CY_SMIF_NO_SFDP_SUPPORT:
            printf("CY_SMIF_NO_SFDP_SUPPORT");
            break;
        case CY_SMIF_SFDP_SS0_FAILED:
            printf("CY_SMIF_SFDP_SS0_FAILED");
            break;
        case CY_SMIF_SFDP_SS1_FAILED:
            printf("CY_SMIF_SFDP_SS1_FAILED");
            break;
        case CY_SMIF_SFDP_SS2_FAILED:
            printf("CY_SMIF_SFDP_SS2_FAILED");
            break;
        case CY_SMIF_SFDP_SS3_FAILED:
            printf("CY_SMIF_SFDP_SS3_FAILED");
            break;
        case CY_SMIF_CMD_NOT_FOUND:
            printf("CY_SMIF_CMD_NOT_FOUND");
            break;
    }
    printf("\r\n");
}

cy_en_smif_status_t qspi_init_hardware()
{
    cy_en_smif_status_t st;

	Cy_GPIO_Pin_Init(SS0Port, SS0Pin, &QSPI_SS_config);
    Cy_GPIO_SetHSIOM(SS0Port, SS0Pin, SS0MuxPort);

	Cy_GPIO_Pin_Init(SS1Port, SS1Pin, &QSPI_SS_config);
    Cy_GPIO_SetHSIOM(SS1Port, SS1Pin, SS1MuxPort);

    Cy_GPIO_Pin_Init(D3Port, D3Pin, &QSPI_DATA3_config);
    Cy_GPIO_SetHSIOM(D3Port, D3Pin, D3MuxPort);

    Cy_GPIO_Pin_Init(D2Port, D2Pin, &QSPI_DATA2_config);
    Cy_GPIO_SetHSIOM(D2Port, D2Pin, D2MuxPort);

    Cy_GPIO_Pin_Init(D1Port, D1Pin, &QSPI_DATA1_config);
    Cy_GPIO_SetHSIOM(D1Port, D1Pin, D1MuxPort);

    Cy_GPIO_Pin_Init(D0Port, D0Pin, &QSPI_DATA0_config);
    Cy_GPIO_SetHSIOM(D0Port, D0Pin, D0MuxPort);

	Cy_GPIO_Pin_Init(SCKPort, SCKPin, &QSPI_SCK_config);
    Cy_GPIO_SetHSIOM(SCKPort, SCKPin, SCKMuxPort);

              
    /*
     * Setup the interrupt for the SMIF block.  For the CM0 there
     * is a two stage process to setup the interrupts.
     */
    Cy_SysInt_Init(&smifIntConfig, Isr_SMIF);

    st = Cy_SMIF_Init(QSPIPort, &QSPI_config, 1000, &QSPI_context);
    if (st != CY_SMIF_SUCCESS)
    {
        failed("Cy_SMIF_Init", st);
        return st;
    }

    NVIC_EnableIRQ(smifIntConfig.intrSrc); /* Finally, Enable the SMIF interrupt */

    Cy_SMIF_Enable(QSPIPort, &QSPI_context);

    return CY_SMIF_SUCCESS;
}

void qspi_dump_device(cy_stc_smif_mem_device_cfg_t *dev)
{
    uint32_t s;

    if (dev->memSize & 0x80000000)
    {
        s = (1 << (dev->memSize & 0x7fffffff));
    }
    else
    {
        s = dev->memSize;
    }

    printf("Address Bytes.......... %ld bytes\n", dev->numOfAddrBytes);
    printf("Memory Size............ %ld bytes\n", s);
    printf("Erase Size............. %ld bytes\n", dev->eraseSize);
    printf("Program Size........... %ld bytes\n", dev->programSize);
    printf("Sector Erase Time ..... %ld ms\n", dev->eraseTime);
    printf("Chip Erase Time ....... %ld ms\n", dev->chipEraseTime);
    printf("Program Time .......... %ld ms\n", dev->programTime);
    printf("Read Command .......... %lx\n", dev->readCmd->command);
    printf("Sector Erase Command .. %lx\n", dev->eraseCmd->command);
    printf("Chip Erase Command..... %lx\n", dev->chipEraseCmd->command);
    printf("Program Command ....... %lx\n", dev->programCmd->command);
    printf("WriteEnable Command.... %lx\n", dev->writeEnCmd->command);
    printf("WriteDisable Command .. %lx\n", dev->writeDisCmd->command);
    printf("ReadWIPStatus Command . %lx, mask %lx\n", dev->readStsRegWipCmd->command, dev->stsRegBusyMask);
    printf("ReadQEStatus Command .. %lx, mask %lx\n", dev->readStsRegQeCmd->command, dev->stsRegQuadEnableMask);
    printf("WriteQEStatus Command . %lx, mask %lx\n", dev->writeStsRegQeCmd->command, dev->stsRegQuadEnableMask);
}

cy_stc_smif_mem_config_t *qspi_get_memory_config(int index) 
{
    return smif_blk_config->memConfig[index];
}

SMIF_Type *qspi_get_device()
{
    return QSPIPort;
}

cy_stc_smif_context_t *qspi_get_context()
{
    return &QSPI_context;
}

cy_en_smif_status_t qspi_init(cy_stc_smif_block_config_t *blk_config)
{
    cy_en_smif_status_t st;

    st = qspi_init_hardware();
    if (st == CY_SMIF_SUCCESS)
    {
        smif_blk_config = blk_config;
        st = Cy_SMIF_MemInit(QSPIPort, smif_blk_config, &QSPI_context);
    }
    return st;
}

cy_en_smif_status_t qspi_init_sfdp(void)
{
    return qspi_init(&smifBlockConfig_sfdp);
}
