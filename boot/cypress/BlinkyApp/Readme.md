### Blinking LED test application for MCUBoot Bootloader.

**Description:**

Implements simple Blinky LED CM4 application to demonstrate MCUBoot Application and CypressBootloader operation in terms of BOOT and UPGRADE process.

It is started by MCUBoot Application or CypressBootloader which is running on CM0p.

* Blinks RED led with 2 different rates, depending on type of image - BOOT or UPGRADE.
* Prints debug info and version of itself to terminal at 115200 baud.
* Can be built for BOOT slot or UPGRADE slot bootloader.

**Pre-build action:**

Pre-build action is implemented for defining start address and size of flash dedicated to BlinkyApp. Pre-build action calls GCC preprocessor to instantiate values to `BlinkyApp_template.ld` found in `main.h` file of BlinkyApp of defined at compile time via macroses `-DUSER_APP_START` and `-DUSER_APP_SIZE`.

Default values set for currently supported targets:
* CY8CPROTO_062_4343W in `BlinkyApp.mk` to `-DUSER_APP_START=0x10010000`
* CY8CKIT_064S2_4343W in `BlinkyApp.mk` to `-DUSER_APP_START=0x10000000`

Start of `BlinkyApp` built to use with Secure Boot enabled targets corresponds to default policy settings provided with `cysecuretools` package.

**Building Multi-Image**

`BlinkyApp` can be built to use in multi-image bootloader configuration.

To build appropriate image build flag `MULTI_IMAGE=1`("Enabled") has to be passed to `make`. Default value is `MULTI_IMAGE=0`("Disabled").

*Note:* only 2 images are supported at the moment.

Default `BlinkyApp` addresses for `BOOT` and `UPGRADE` (PSoC-062 2M device is suggested):

Single image scheme (default):
    
    BOOT    - 0x10000000
    UPGRADE - 0x10010000

Multiimage scheme, second image in a boot chain (`MULTI_IMAGE=1`):

    BOOT    - 0x10020000
    UPGRADE - 0x10030000

**Building an application:**

Root directory for build is **boot/cypress.**

The following command will build regular HEX file of a Blinky Application, BOOT slot:

    make app APP_NAME=BlinkyApp TARGET=CY8CPROTO-062-4343W IMG_TYPE=BOOT

This have following defaults suggested:

    BUILDCFG=Debug
    IMG_TYPE=BOOT

To build UPGRADE image use following command:

    make app APP_NAME=BlinkyApp TARGET=CY8CPROTO-062-4343W IMG_TYPE=UPGRADE HEADER_OFFSET=0x10000
    
Example command-line for single-image:

    make app APP_NAME=BlinkyApp TARGET=CY8CPROTO-062-4343W IMG_TYPE=BOOT MULTI_IMAGE=0

Example command-line for dual-image:

    make app APP_NAME=BlinkyApp TARGET=CY8CPROTO-062-4343W IMG_TYPE=BOOT MULTI_IMAGE=1

**Post-Build:**

Post build action is executed at compile time for `BlinkyApp`. It calls `imgtool` from `MCUBoot` scripts and add signature to compiled image. This signature is then validated by CypressBootloader or MCUBoot application.

**How to program an application:**

**_Use any preferred tool for programming hex files._**

Currently implemented makefile jobs use DAPLINK interface for programming.

To program BOOT image:

    make load_boot APP_NAME=BlinkyApp TARGET=CY8CPROTO-062-4343W

To program UPGRADE image:

    make load_upgrade APP_NAME=BlinkyApp TARGET=CY8CPROTO-062-4343W

Flags defaults:

    BUILDCFG=Debug

**Flags:**
- `BUILDCFG` - configuration **Release** or **Debug**
- `MAKEINFO` - 0 (default) - less build info, 1 - verbose output of compilation.
- `HEADER_OFFSET` - 0 (default) - no offset of output hex file, 0x%VALUE% - offset for output hex file. Value 0x10000 is slot size MCUBoot Bootloader in this example.
- `IMG_TYPE` - `BOOT` (default) - build image for BOOT slot of MCUBoot Bootloader, `UPGRADE` - build image for UPGRADE slot of MCUBoot Bootloader.
- `MULTI_IMAGE` - `0` (default) - set addresses for single image scheme. `1` - set addresses for multiimage scheme.

**NOTE**: In case of `UPGRADE` image `HEADER_OFFSET` should be set to MCUBoot Bootloader slot size.

**Example terminal output:**

When user application programmed in BOOT slot:

    ===========================
    [BlinkyApp] BlinkyApp v1.0 [CM4]
    ===========================
    [BlinkyApp] GPIO initialized
    [BlinkyApp] UART initialized
    [BlinkyApp] Retarget I/O set to 115200 baudrate
    [BlinkyApp] Red led blinks with 1 sec period

When user application programmed in UPRADE slot and upgrade procedure was successful:

    ===========================
    [BlinkyApp] BlinkyApp v2.0 [CM4]
    ===========================

    [BlinkyApp] GPIO initialized
    [BlinkyApp] UART initialized
    [BlinkyApp] Retarget I/O set to 115200 baudrate
    [BlinkyApp] Red led blinks with 0.25 sec period
    [BlinkyApp] Red led blinks with 0.25 sec period
