/*
 * mbed Microcontroller Library
 * Copyright (c) 2019 Cypress Semiconductor Corporation
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
 */

/**
 * \file    fb_crypto_common.h
 * \version 1.0
 *
 * \brief   Header file for common mbedtls acceleration functions
 *
 */

#if !defined(FB_CRYPTO_COMMON_H)
#define CRYPTO_COMMON_H

/** CRYPTO object */
typedef struct {
#if defined(CY_IP_MXCRYPTO_INSTANCES) || defined(CPUSS_CRYPTO_PRESENT)
    CRYPTO_Type*                base;
    fb_cyhal_resource_inst_t    resource;
    fb_cyhal_crypto_feature_t   feature;
#endif
} fb_cy_hw_crypto_t;

#endif /* (CRYPTO_COMMON_H) */
