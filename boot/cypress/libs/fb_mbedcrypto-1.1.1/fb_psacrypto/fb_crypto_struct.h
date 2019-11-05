/**
 * \file fb_crypto_struct.h
 *
 * \brief PSA cryptography module: Mbed TLS structured type implementations
 *
 * \note This file may not be included directly. Applications must
 * include psa/crypto.h.
 *
 * This file contains the definitions of some data structures with
 * implementation-specific definitions.
 *
 * In implementations with isolation between the application and the
 * cryptography module, it is expected that the front-end and the back-end
 * would have different versions of this file.
 */
/*
 *  Copyright (C) 2019, Cypress Semiconductor
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
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef FB_PSA_CRYPTO_STRUCT_H
#define FB_PSA_CRYPTO_STRUCT_H

struct fb_psa_hash_operation_s
{
    fb_psa_algorithm_t alg;
    union
    {
        unsigned dummy; /* Make the union non-empty even with no supported algorithms. */
#if defined(FB_MBEDTLS_MD2_C)
        fb_mbedtls_md2_context md2;
#endif
#if defined(FB_MBEDTLS_MD4_C)
        fb_mbedtls_md4_context md4;
#endif
#if defined(FB_MBEDTLS_MD5_C)
        fb_mbedtls_md5_context md5;
#endif
#if defined(FB_MBEDTLS_RIPEMD160_C)
        fb_mbedtls_ripemd160_context ripemd160;
#endif
#if defined(FB_MBEDTLS_SHA1_C)
        fb_mbedtls_sha1_context sha1;
#endif
#if defined(FB_MBEDTLS_SHA256_C)
        fb_mbedtls_sha256_context sha256;
#endif
#if defined(FB_MBEDTLS_SHA512_C)
        fb_mbedtls_sha512_context sha512;
#endif
    } ctx;
};

#if defined(FB_MBEDTLS_MD_C)
typedef struct
{
        /** The hash context. */
        struct fb_psa_hash_operation_s hash_ctx;
        /** The HMAC part of the context. */
        uint8_t opad[FB_PSA_HMAC_MAX_HASH_BLOCK_SIZE];
} fb_psa_hmac_internal_data;
#endif /* FB_MBEDTLS_MD_C */

#if defined(FB_MBEDTLS_MD_C)
typedef struct
{
    uint8_t *info;
    size_t info_length;
    fb_psa_hmac_internal_data hmac;
    uint8_t prk[FB_PSA_HASH_MAX_SIZE];
    uint8_t output_block[FB_PSA_HASH_MAX_SIZE];
#if FB_PSA_HASH_MAX_SIZE > 0xff
#error "FB_PSA_HASH_MAX_SIZE does not fit in uint8_t"
#endif
    uint8_t offset_in_block;
    uint8_t block_number;
} fb_psa_hkdf_generator_t;
#endif /* FB_MBEDTLS_MD_C */

struct fb_psa_key_policy_s
{
    fb_psa_key_usage_t usage;
    fb_psa_algorithm_t alg;
    fb_psa_algorithm_t alg2;
};

#if defined(FB_MBEDTLS_MD_C)
typedef struct fb_psa_tls12_prf_generator_s
{
    /* The TLS 1.2 PRF uses the key for each HMAC iteration,
     * hence we must store it for the lifetime of the generator.
     * This is different from HKDF, where the key is only used
     * in the extraction phase, but not during expansion. */
    unsigned char *key;
    size_t key_len;

    /* `A(i) + seed` in the notation of RFC 5246, Sect. 5 */
    uint8_t *Ai_with_seed;
    size_t Ai_with_seed_len;

    /* `HMAC_hash( prk, A(i) + seed )` in the notation of RFC 5246, Sect. 5. */
    uint8_t output_block[FB_PSA_HASH_MAX_SIZE];

#if FB_PSA_HASH_MAX_SIZE > 0xff
#error "FB_PSA_HASH_MAX_SIZE does not fit in uint8_t"
#endif

    /* Indicates how many bytes in the current HMAC block have
     * already been read by the user. */
    uint8_t offset_in_block;

    /* The 1-based number of the block. */
    uint8_t block_number;

} fb_psa_tls12_prf_generator_t;
#endif /* FB_MBEDTLS_MD_C */

struct fb_psa_crypto_generator_s
{
    fb_psa_algorithm_t alg;
    size_t capacity;
    union
    {
        struct
        {
            uint8_t *data;
            size_t size;
        } buffer;
#if defined(FB_MBEDTLS_MD_C)
        fb_psa_hkdf_generator_t hkdf;
        fb_psa_tls12_prf_generator_t tls12_prf;
#endif
    } ctx;
};

struct fb_psa_cipher_operation_s
{
    fb_psa_algorithm_t alg;
    unsigned int key_set : 1;
    unsigned int iv_required : 1;
    unsigned int iv_set : 1;
    uint8_t iv_size;
    uint8_t block_size;
    union
    {
        unsigned dummy; /* Enable easier initializing of the union. */
        fb_mbedtls_cipher_context_t cipher;
    } ctx;
};

#endif /* FB_PSA_CRYPTO_STRUCT_H */
