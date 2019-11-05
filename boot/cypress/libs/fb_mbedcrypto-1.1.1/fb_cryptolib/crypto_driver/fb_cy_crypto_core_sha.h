/***************************************************************************//**
* \file fb_cy_crypto_core_sha.h
* \version 2.40
*
* \brief
*  This file provides constants and function prototypes
*  for the API for the SHA method in the Crypto block driver.
*
********************************************************************************
* Copyright 2016-2019 Cypress Semiconductor Corporation
* SPDX-License-Identifier: Apache-2.0
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/


#if !defined(FB_CY_CRYPTO_CORE_SHA_H)
#define FB_CY_CRYPTO_CORE_SHA_H

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct
{
    /* Allocates CRYPTO_MAX_BLOCK_SIZE Bytes for the block. */
    uint32_t block[CY_CRYPTO_SHA256_BLOCK_SIZE / 4u];

    /* Allocates CRYPTO_MAX_HASH_SIZE Bytes for the hash. */
    uint32_t hash[CY_CRYPTO_SHA256_HASH_SIZE / 4u];

#if (CY_IP_MXCRYPTO_VERSION == 1u)
    /* Allocates CRYPTO_MAX_ROUND_MEM_SIZE Bytes for roundMem. */
    uint32_t roundMem[CY_CRYPTO_SHA256_ROUND_MEM_SIZE / 4u];
#endif /* (CY_IP_MXCRYPTO_VERSION == 1u) */

} fb_cy_stc_crypto_sha256_buffers_t;

typedef fb_cy_stc_crypto_sha256_buffers_t fb_cy_stc_crypto_sha_buffers_t;

#if defined(__cplusplus)
}
#endif

#endif /* !defined(FB_CY_CRYPTO_CORE_SHA_H) */

/* [] END OF FILE */
