/**
 * \file fb_crypto.h
 * \brief Platform Security Architecture cryptography module
 */
/*
 *  Copyright (C) 2018, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef FB_PSA_CRYPTO_H
#define FB_PSA_CRYPTO_H

#include "fb_crypto_platform.h"

#include <stddef.h>

#ifdef __DOXYGEN_ONLY__
/* This __DOXYGEN_ONLY__ block contains mock definitions for things that
 * must be defined in the crypto_platform.h header. These mock definitions
 * are present in this file as a convenience to generate pretty-printed
 * documentation that includes those definitions. */

/** \defgroup platform Implementation-specific definitions
 * @{
 */

/** \brief Key handle.
 *
 * This type represents open handles to keys. It must be an unsigned integral
 * type. The choice of type is implementation-dependent.
 *
 * 0 is not a valid key handle. How other handle values are assigned is
 * implementation-dependent.
 */
typedef _unsigned_integral_type_ fb_psa_key_handle_t;

/**@}*/
#endif /* __DOXYGEN_ONLY__ */

#ifdef __cplusplus
extern "C" {
#endif

/* The file "crypto_types.h" declares types that encode errors,
 * algorithms, key types, policies, etc. */
#include "fb_crypto_types.h"

#ifdef __cplusplus
}
#endif

#endif /* FB_PSA_CRYPTO_H */
