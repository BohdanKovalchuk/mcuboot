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
#   - CY8CPROTO-062-4343W
#	- CY8CKIT_062_WIFI_BT
#	- more to come

# default TARGET
#TARGET ?= CY8CPROTO-062-4343W-M0
# this must be 064-series only
TARGET ?= 064_2M
 
# 
TARGETS := 064_2M 064_1M 064_512K

# For which core this application is built
CORE := CM0P

CUR_LIBS_PATH := $(CURDIR)/libs
BSP_PATH := $(CURDIR)/$(APP_NAME)/platform

# MCU device selection
DEVICE:=CYB0644ABZI-S2D44

# Additional components supported by the target
COMPONENTS+=BSP_DESIGN_MODUS PSOC6HAL
# Use CyHAL
DEFINES:=CY_USING_HAL

# Collect C source files for TARGET BSP
#SOURCES_BSP := $(wildcard $(BSP_PATH)/COMPONENT_BSP_DESIGN_MODUS/GeneratedSource/*.c)
#SOURCES_BSP += $(wildcard $(BSP_PATH)/COMPONENT_$(CORE)/*.c)
SOURCES_BSP += $(wildcard $(BSP_PATH)/*.c)
SOURCES_BSP += $(wildcard $(CUR_LIBS_PATH)/bsp/psoc6hal/src/*.c)
SOURCES_BSP += $(wildcard $(CUR_LIBS_PATH)/bsp/psoc6hal/src/pin_packages/*.c)


# Collect dirrectories containing headers for TARGET BSP
#INCLUDE_DIRS_BSP := $(BSP_PATH)/COMPONENT_BSP_DESIGN_MODUS/GeneratedSource/
#INCLUDE_DIRS_BSP += $(BSP_PATH)/startup
INCLUDE_DIRS_BSP += $(BSP_PATH)
INCLUDE_DIRS_BSP += $(CUR_LIBS_PATH)/bsp/psoc6hal/include
# Collect Assembler files for TARGET BSP
# TODO: need to include _01_, _02_ or _03_ depending on device family.
STARTUP_FILE := $(BSP_PATH)/$(TARGET)/$(CORE)/$(COMPILER)/startup_psoc6_02_cm0plus

ifeq ($(COMPILER), GCC_ARM)
	ASM_FILES_BSP := $(STARTUP_FILE).S
else
$(error Only GCC ARM is supported at this moment)
endif

# Add device name from BSP makefile to defines
DEFINES += $(DEVICE)
DEFINES += $(COMPONENTS)

# Get defines from BSP makefile and convert it to regular -DMY_NAME style 
ifneq ($(DEFINES),)
	DEFINES_BSP :=$(addprefix -D, $(subst -,_,$(DEFINES)))
endif

ifeq ($(COMPILER), GCC_ARM)
LINKER_SCRIPT ?= $(BSP_PATH)/$(TARGET)/$(CORE)/$(COMPILER)/*_cm0plus.ld
else
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

