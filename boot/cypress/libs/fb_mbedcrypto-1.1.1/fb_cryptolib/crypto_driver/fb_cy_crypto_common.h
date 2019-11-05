/***************************************************************************//**
* \file fb_cy_crypto_common.h
* \version `$CY_MAJOR_VERSION`.`$CY_MINOR_VERSION`
*
* \brief
*  This file provides common constants and parameters
*  for the Crypto driver.
*
********************************************************************************
* \copyright
* Copyright 2016-2018, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(FB_CY_CRYPTO_COMMON_H)
#define FB_CY_CRYPTO_COMMON_H

/** Defines the Crypto AES block size (in bytes) */
#define FB_CY_CRYPTO_AES_BLOCK_SIZE          (16u)

/** Defines the Crypto AES_256 key maximum size (in bytes) */
#define FB_CY_CRYPTO_AES_256_KEY_SIZE        (32u)

/** Defines the Crypto AES key maximum size (in bytes) */
#define FB_CY_CRYPTO_AES_MAX_KEY_SIZE        (FB_CY_CRYPTO_AES_256_KEY_SIZE)

/** Defines the Crypto AES_256 key maximum size (in four-byte words) */
#define FB_CY_CRYPTO_AES_MAX_KEY_SIZE_U32    (uint32_t)(FB_CY_CRYPTO_AES_MAX_KEY_SIZE / 4ul)

/** Defines size of the AES block, in four-byte words */
#define FB_CY_CRYPTO_AES_BLOCK_SIZE_U32      (uint32_t)(FB_CY_CRYPTO_AES_BLOCK_SIZE / 4ul)

/** The key length options for the AES method. */
typedef enum
{
    FB_CY_CRYPTO_KEY_AES_128   = 0x00u,   /**< The AES key size is 128 bits */
    FB_CY_CRYPTO_KEY_AES_192   = 0x01u,   /**< The AES key size is 192 bits */
    FB_CY_CRYPTO_KEY_AES_256   = 0x02u    /**< The AES key size is 256 bits */
} fb_cy_en_crypto_aes_key_length_t;

/* The structure to define used memory buffers */
typedef struct
{
    uint32_t key[FB_CY_CRYPTO_AES_MAX_KEY_SIZE_U32];
    uint32_t keyInv[FB_CY_CRYPTO_AES_MAX_KEY_SIZE_U32];
    uint32_t block0[FB_CY_CRYPTO_AES_BLOCK_SIZE_U32];
    uint32_t block1[FB_CY_CRYPTO_AES_BLOCK_SIZE_U32];
    uint32_t block2[FB_CY_CRYPTO_AES_BLOCK_SIZE_U32];
} fb_cy_stc_crypto_aes_buffers_t;

/** Structure for storing the AES state */
typedef struct
{
    /** AES key length */
    fb_cy_en_crypto_aes_key_length_t keyLength;
    /** Pointer to AES work buffers */
    fb_cy_stc_crypto_aes_buffers_t *buffers;
    /** AES processed block index (for CMAC, SHA operations) */
    uint32_t blockIdx;
} fb_cy_stc_crypto_aes_state_t;

/* The structure for storing the SHA context */
typedef struct
{
    uint32_t mode;
    uint32_t modeHw;
    uint8_t *block;
    uint32_t blockSize;
    uint8_t *hash;
    uint32_t hashSize;
    uint8_t *roundMem;
    uint32_t roundMemSize;
    uint32_t messageSize;
    uint32_t digestSize;
    uint32_t blockIdx;
    uint8_t  const *initialHash;
} fb_cy_stc_crypto_sha_state_t;

#endif /* #if !defined(FB_CY_CRYPTO_COMMON_H) */

/* [] END OF FILE */
