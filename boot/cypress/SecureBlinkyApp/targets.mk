################################################################################
# \file targets.mk
# \version 1.0
#
# \brief
# Makefile to describe supported boards and platforms for Cypress MCUBoot based applications.
#
################################################################################
# \copyright
# Copyright 2018-2019 Cypress Semiconductor Corporation
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

# Target board MCUBoot is built for. CY8CPROTO-062-4343W is set as default
# Supported:
#	- CY8CKIT-064S2-4343W
#	- more to come

# default TARGET
TARGET ?= CY8CKIT-064S2-4343W
#
SB_TARGETS := CY8CKIT-064S2-4343W CY8CKIT-064B0S2-4343W

# For which core this application is built
CORE := CM0P

CUR_LIBS_PATH := $(CURDIR)/libs
BSP_PATH  := $(CUR_LIBS_PATH)/bsp/TARGET_$(TARGET)

ifneq ($(filter $(TARGET), $(SB_TARGETS)),)
include ./libs/bsp/TARGET_$(TARGET)/$(TARGET).mk
else
$(error Not supported target: '$(TARGET)')
endif

# Check if path to cysecuretools is set
ifeq ($(CY_SEC_TOOLS_PATH), )
$(error Variable CY_SEC_TOOLS_PATH - path to cysecuretools package not set. \
		Use `python -m pip show cysecuretools` to determine intallation folder.` \
		Then set it in Makefile to continue work.)
endif

# Collect dirrectories containing headers for TARGET BSP
INCLUDE_DIRS_BSP := $(BSP_PATH)/COMPONENT_BSP_DESIGN_MODUS/GeneratedSource/
INCLUDE_DIRS_BSP += $(BSP_PATH)/startup
INCLUDE_DIRS_BSP += $(BSP_PATH)
INCLUDE_DIRS_BSP += $(CUR_LIBS_PATH)/bsp/psoc6hal/include

# Collect C source files for TARGET BSP
SOURCES_BSP := $(wildcard $(BSP_PATH)/COMPONENT_BSP_DESIGN_MODUS/GeneratedSource/*.c)
#SOURCES_BSP += $(wildcard $(BSP_PATH)/COMPONENT_$(CORE)/*.c)
SOURCES_BSP += $(BSP_PATH)/COMPONENT_$(CORE)/system_psoc6_cm0plus.c
# exclude CapSense for now
SOURCES_BSP := $(filter-out $(BSP_PATH)/COMPONENT_BSP_DESIGN_MODUS/GeneratedSource/cycfg_capsense.c, \
				 $(SOURCES_BSP))
SOURCES_BSP += $(BSP_PATH)/cybsp.c
SOURCES_BSP += $(wildcard $(CUR_LIBS_PATH)/bsp/psoc6hal/src/*.c)
SOURCES_BSP += $(wildcard $(CUR_LIBS_PATH)/bsp/psoc6hal/src/pin_packages/*.c)

# Collect Assembler files for TARGET BSP
# TODO: need to include _01_, _02_ or _03_ depending on device family.
STARTUP_FILE := $(BSP_PATH)/COMPONENT_$(CORE)/TOOLCHAIN_$(COMPILER)/startup_psoc6_02_cm0plus

ifeq ($(COMPILER), GCC_ARM)
	ASM_FILES_BSP := $(STARTUP_FILE).S
else
$(error Only GCC ARM is supported at this moment)
endif

# Add device name from BSP makefile to defines
DEFINES += $(DEVICE)

# Get defines from BSP makefile and convert it to regular -DMY_NAME style 
ifneq ($(DEFINES),)
	DEFINES_BSP :=$(addprefix -D, $(subst -,_,$(DEFINES)))
endif

ifneq ($(COMPILER), GCC_ARM)
$(error Only GCC ARM is supported at this moment)
endif
ifeq ($(MAKEINFO) , 1)
$(info ==============================================================================)
$(info = BSP files =)
$(info ==============================================================================)
$(info $(SOURCES_BSP))
$(info $(ASM_FILES_BSP))
endif

# TODO: include appropriate BSP linker(s)
# TODO: include appropriate BSP precompiled

