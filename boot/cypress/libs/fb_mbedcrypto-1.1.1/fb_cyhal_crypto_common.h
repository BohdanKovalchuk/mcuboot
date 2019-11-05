/***************************************************************************//**
* \file fb_cyhal_crypto_common.h
*
* Description:
* This file provides common defines, addresses, and functions required by drivers
* using the Crypto block.
*
********************************************************************************
* \copyright
* Copyright 2018-2019 Cypress Semiconductor Corporation
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

#pragma once

#include "cy_device.h"
#include "cy_pdl.h"

#if defined(CY_IP_MXCRYPTO)

#if defined(__cplusplus)
extern "C" {
#endif

/** Block count for CRYPTO blocks */
#define CYHAL_CRYPTO_INST_COUNT      CY_IP_MXCRYPTO_INSTANCES

typedef enum
{
    /** CRC hardware acceleration */
    CYHAL_CRYPTO_CRC,
    /** TRNG hardware acceleration */
    CYHAL_CRYPTO_TRNG,
    /** VU hardware acceleration */
    CYHAL_CRYPTO_VU,
    /** Common features of the Crypto block  */
    CYHAL_CRYPTO_COMMON,
} fb_cyhal_crypto_feature_t;

#if defined(__cplusplus)
}
#endif

#endif /* defined(CY_IP_MXCRYPTO) */
