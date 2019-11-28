### SecureBlinking application to test miltiimage in CyBootloader.

**Description:**

Implements simple Blinky LED `CM0p` application to demonstrate miltiimage configuration of CypressBootloader.

It is started by CypressBootloader which is also running on `CM0p`.

* Blinks RED led for 10 sec with 2 different rates, depending on type of image - BOOT or UPGRADE.
* Starts next application on `CM4`
* Prints debug info and version of itself to terminal at 115200 baud.
* Can be built for BOOT slot or UPGRADE slot of bootloader.

**Pre-build action:**

Pre-build action is implemented for defining start address and size of flash dedicated to `SecureBlinkyApp`. Pre-build action calls GCC preprocessor to instantiate values to `SecureBlinkyApp_template.ld` found in `main.h` file of BlinkyApp of defined at compile time via macroses `-DSECURE_APP_START` and `-DSECURE_APP_SIZE`.

Default values set for currently supported targets:
* CY8CKIT_064S2_4343W in `SecureBlinkyApp.mk` to `-DSECURE_APP_START=0x10000000`

Start of `SecureBlinkyApp` built to use with Secure Boot enabled targets corresponds to default policy settings provided with `cysecuretools` package.

**Build an application:**

Root directory for build is **boot/cypress.**

The following command will build regular HEX file of a Blinky Application, BOOT slot:

    make app APP_NAME=SecureBlinkyApp TARGET=CY8CKIT-064S2-4343W IMG_TYPE=BOOT

This have following defaults suggested:

    BUILDCFG=Debug
    IMG_TYPE=BOOT

To build UPGRADE image use following command:

    make app APP_NAME=SecureBlinkyApp TARGET=CY8KIT-064S2-4343W IMG_TYPE=UPGRADE HEADER_OFFSET=0x10000

**Post-Build:**

Post build action is executed at compile time for `SecureBlinkyApp`. It calls `imgtool` from `MCUBoot` scripts and add signature to compiled image. This signature is then validated by CypressBootloader application.

**How to program an application:**

**_Use any preffered tool for programming hex files._**

Currently implemented makefile jobs use DAPLINK interface for programming.

To program BOOT image:

    make load_boot APP_NAME=SecureBlinkyApp TARGET=CY8CKIT-064S2-4343W

To program UPGRADE image:

    make load_upgrade APP_NAME=SecureBlinkyApp TARGET=CY8CKIT-064S2-4343W

Flags defaults:

    BUILDCFG=Debug

**Flags:**
- `BUILDCFG` - configation **Release** of **Debug**
- `MAKEINFO` - 0 (default) - less build info, 1 - verbose output of complilation.
- `HEADER_OFFSET` - 0 (default) - no offset of output hex file, 0x%VALUE% - offset for output hex file. Value 0x10000 is slot size CyBootloader in this example
- `IMG_TYPE` - `BOOT` (default) - build image for BOOT slot of CyBootloader, `UPGRADE` - build image for `UPGRADE` slot of CyBootloader.

**NOTE**: In case of `UPGRADE` image `HEADER_OFFSET` should be set to MCUBoot Bootloader slot size

**Example terminal output:**

When user application programmed in BOOT slot:

    =============================================
    [SecureBlinkyApp] SecureBlinkyApp v1.0 [CM0p]
    =============================================

    [SecureBlinkyApp] GPIO initialized 
    [SecureBlinkyApp] UART initialized 
    [SecureBlinkyApp] Retarget I/O set to 115200 baudrate 
    [SecureBlinkyApp] Red led blinks FAST for 10 sec
    [SecureBlinkyApp] Then CM4 app will be started

When user application programmed in UPRADE slot and upgrade procedure was successful:

    =============================================
    [SecureBlinkyApp] SecureBlinkyApp v1.0 [CM0p]
    =============================================

    [SecureBlinkyApp] GPIO initialized 
    [SecureBlinkyApp] UART initialized 
    [SecureBlinkyApp] Retarget I/O set to 115200 baudrate 
    [SecureBlinkyApp] Red led blinks SLOW for 10 sec
    [SecureBlinkyApp] Then CM4 app will be started
