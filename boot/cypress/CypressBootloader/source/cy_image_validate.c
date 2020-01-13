/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
 /***************************************************************************//**
* \copyright
*
* (c) 2018-2019, Cypress Semiconductor Corporation
* or a subsidiary of Cypress Semiconductor Corporation. All rights
* reserved.
*
* This software is a port of the open source MCUBoot project.
* This file was modified to fit Cypress Secure Bootloader design.
*
* Portions of this software, including source code, documentation and related
* materials ("Software"), are owned by Cypress Semiconductor
* Corporation or one of its subsidiaries ("Cypress") and is protected by
* and subject to worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-
* exclusive, non-transferable license to copy, modify, and compile the
* Software source code solely for use in connection with Cypress's
* integrated circuit products. Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO
* WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING,
* BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE. Cypress reserves the right to make
* changes to the Software without notice. Cypress does not assume any
* liability arising out of the application or use of the Software or any
* product or circuit described in the Software. Cypress does not
* authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*
********************************************************************************/

#ifdef MCUBOOT_HAVE_ASSERT_H
#include "mcuboot_config/mcuboot_assert.h"
#else
#include <assert.h>
#endif

#include <stddef.h>
#include <inttypes.h>
#include <string.h>

#include <flash_map_backend/flash_map_backend.h>

#include "bootutil/image.h"
#include "bootutil/sha256.h"
#include "bootutil/sign_key.h"

#include "mcuboot_config/mcuboot_config.h"

#include "bootutil/bootutil_log.h"

#ifdef MCUBOOT_ENC_IMAGES
#include "bootutil/enc_key.h"
#endif
#if defined(MCUBOOT_SIGN_RSA)
#include "mbedtls/rsa.h"
#endif
#if defined(MCUBOOT_SIGN_EC) || defined(MCUBOOT_SIGN_EC256)
#include "mbedtls/ecdsa.h"
#endif
#include "mbedtls/asn1.h"

#include "bootutil_priv.h"

#include "cy_jwt_policy.h"

extern bnu_policy_t cy_bl_bnu_policy;

/* Additional TLV tags */
#define IMAGE_TLV_CYSB_IMAGE_ID            0x81   /* Image ID */
#define IMAGE_TLV_CYSB_ROLLBACK_CNT        0x82   /* Rollback counter */

/*
 * Compute SHA256 over the image.
 */
static int
bootutil_img_hash(struct enc_key_data *enc_state, int image_index,
                  struct image_header *hdr, const struct flash_area *fap,
                  uint8_t *tmp_buf, uint32_t tmp_buf_sz, uint8_t *hash_result,
                  uint8_t *seed, int seed_len)
{
    bootutil_sha256_context sha256_ctx;
    uint32_t blk_sz;
    uint32_t size;
    uint16_t hdr_size;
    uint32_t off;
    int rc;
    uint32_t blk_off;
    uint32_t tlv_off;

    // TODO: run-time multi-image
//#if (BOOT_IMAGE_NUMBER == 1) || !defined(MCUBOOT_ENC_IMAGES)
    (void)enc_state;
    (void)image_index;
    (void)hdr_size;
    (void)blk_off;
    (void)tlv_off;
//#endif

#ifdef MCUBOOT_ENC_IMAGES
    /* Encrypted images only exist in the secondary slot */
    if (MUST_DECRYPT(fap, image_index, hdr) &&
            !boot_enc_valid(enc_state, image_index, fap)) {
        return -1;
    }
#endif
    psa_status_t psa_ret;

    psa_ret = bootutil_sha256_init(&sha256_ctx);

    if(0 == psa_ret)
    {
        /* in some cases (split image) the hash is seeded with data from
         * the loader image */
        if (seed && (seed_len > 0)) {
        	psa_ret = bootutil_sha256_update(&sha256_ctx, seed, seed_len);
        }

        if(0 == psa_ret)
        {

            /* Hash is computed over image header and image itself. */
            size = hdr_size = hdr->ih_hdr_size;
            size += hdr->ih_img_size;
            tlv_off = size;

            /* If protected TLVs are present they are also hashed. */
            size += hdr->ih_protect_tlv_size;

            for (off = 0; off < size; off += blk_sz) {
                blk_sz = size - off;
                if (blk_sz > tmp_buf_sz) {
                    blk_sz = tmp_buf_sz;
                }
#ifdef MCUBOOT_ENC_IMAGES
                /* The only data that is encrypted in an image is the payload;
                 * both header and TLVs (when protected) are not.
                 */
                if ((off < hdr_size) && ((off + blk_sz) > hdr_size)) {
                    /* read only the header */
                    blk_sz = hdr_size - off;
                }
                if ((off < tlv_off) && ((off + blk_sz) > tlv_off)) {
                    /* read only up to the end of the image payload */
                    blk_sz = tlv_off - off;
                }
#endif
                rc = flash_area_read(fap, off, tmp_buf, blk_sz);
                if (rc) {
                    return rc;
                }
#ifdef MCUBOOT_ENC_IMAGES
                if (MUST_DECRYPT(fap, image_index, hdr)) {
                    /* Only payload is encrypted (area between header and TLVs) */
                    if (off >= hdr_size && off < tlv_off) {
                        blk_off = (off - hdr_size) & 0xf;
                        boot_encrypt(enc_state, image_index, fap, off - hdr_size,
                                blk_sz, blk_off, tmp_buf);
                    }
                }
#endif
                psa_ret = bootutil_sha256_update(&sha256_ctx, tmp_buf, blk_sz);
            }

    		psa_ret = bootutil_sha256_finish(&sha256_ctx, hash_result);
	    }
    }

    return (int)psa_ret;
}

/*
 * Currently, we only support being able to verify one type of
 * signature, because there is a single verification function that we
 * call.  List the type of TLV we are expecting.  If we aren't
 * configured for any signature, don't define this macro.
 */
#if (defined(MCUBOOT_SIGN_RSA)      + \
     defined(MCUBOOT_SIGN_EC)       + \
     defined(MCUBOOT_SIGN_EC256)    + \
     defined(MCUBOOT_SIGN_ED25519)) > 1
#error "Only a single signature type is supported!"
#endif

#if defined(MCUBOOT_SIGN_RSA)
#    if MCUBOOT_SIGN_RSA_LEN == 2048
#        define EXPECTED_SIG_TLV IMAGE_TLV_RSA2048_PSS
#    elif MCUBOOT_SIGN_RSA_LEN == 3072
#        define EXPECTED_SIG_TLV IMAGE_TLV_RSA3072_PSS
#    else
#        error "Unsupported RSA signature length"
#    endif
#    define SIG_BUF_SIZE (MCUBOOT_SIGN_RSA_LEN / 8)
#    define EXPECTED_SIG_LEN(x) ((x) == SIG_BUF_SIZE) /* 2048 bits */
#elif defined(MCUBOOT_SIGN_EC)
#    define EXPECTED_SIG_TLV IMAGE_TLV_ECDSA224
#    define SIG_BUF_SIZE 128
#    define EXPECTED_SIG_LEN(x) ((x) >= 64) /* oids + 2 * 28 bytes */
#elif defined(MCUBOOT_SIGN_EC256)
#    define EXPECTED_SIG_TLV IMAGE_TLV_ECDSA256
#    define SIG_BUF_SIZE 128
#    define EXPECTED_SIG_LEN(x) ((x) >= 72) /* oids + 2 * 32 bytes */
#elif defined(MCUBOOT_SIGN_ED25519)
#    define EXPECTED_SIG_TLV IMAGE_TLV_ED25519
#    define SIG_BUF_SIZE 64
#    define EXPECTED_SIG_LEN(x) ((x) == SIG_BUF_SIZE)
#else
#    define SIG_BUF_SIZE 32 /* no signing, sha256 digest only */
#endif

#ifdef EXPECTED_SIG_TLV
int
bootutil_find_key(uint8_t *keyhash, uint8_t keyhash_len)
{
	psa_status_t psa_ret = -1;
    bootutil_sha256_context sha256_ctx;
    int i = 0;
    const struct bootutil_key *key;
    uint8_t hash[PSA_HASH_SIZE(PSA_ALG_SHA_256)];

    assert(keyhash_len <= PSA_HASH_SIZE(PSA_ALG_SHA_256));

    if(bootutil_keys[0].key == NULL)
    {
        /* skip [0] if go through PSA key storage */
    	i = 1;
    }

    for (; i < bootutil_key_cnt; i++) {
        key = &bootutil_keys[i];

        if (NULL != key->key)
        {
            psa_ret = bootutil_sha256_init(&sha256_ctx);
            if(0 == psa_ret)
            {
                psa_ret = bootutil_sha256_update(&sha256_ctx, key->key, *key->len);
            }
            if(0 == psa_ret)
            {
                psa_ret = bootutil_sha256_finish(&sha256_ctx, hash);
            }
            if(0 == psa_ret)
            {
                if (!memcmp(hash, keyhash, keyhash_len)) {
                    return i;
                }
                else
                {
                    psa_ret = -1;
                }
            }
            else
            {
                psa_ret = -1;
            }
        }
    }
    return psa_ret;
}

#endif

static int cy_bootutil_get_multi_idx(const struct flash_area *fap)
{
    int multi_idx = -1;

    /* find out if it is some of multi-image */
    if((fap->fa_id == FLASH_AREA_IMAGE_PRIMARY(0)) ||
        (fap->fa_id == FLASH_AREA_IMAGE_SECONDARY(0)))
    {
        multi_idx = 0;
    }
    else
    if((fap->fa_id == FLASH_AREA_IMAGE_PRIMARY(1)) ||
        (fap->fa_id == FLASH_AREA_IMAGE_SECONDARY(1)))
    {
        multi_idx = 1;
    }

    return multi_idx;
}

static int cy_bootutil_get_slot_id(const struct flash_area *fap)
{
    int slot_id = -1;

    /* find out if it is slot_0 or slot_1*/
    if((fap->fa_id == FLASH_AREA_IMAGE_PRIMARY(0)) ||
        (fap->fa_id == FLASH_AREA_IMAGE_PRIMARY(1)))
    {
        slot_id = 0;
    }
    else
    if((fap->fa_id == FLASH_AREA_IMAGE_SECONDARY(0)) ||
        (fap->fa_id == FLASH_AREA_IMAGE_SECONDARY(1)))
    {
        slot_id = 1;
    }

    return slot_id;
}

static int cy_bootutil_find_key(const struct flash_area *fap)
{
    int key = 0;
    int multi_idx = -1;
    int slot_id = -1;

    /* find out if it is some of multi-image */
    multi_idx = cy_bootutil_get_multi_idx(fap);

    if (multi_idx >= 0)
    {
        /* find out if it is slot_0 or slot_1*/
        slot_id = cy_bootutil_get_slot_id(fap);

        if (slot_id >= 0)
        {
            if (slot_id > 0)
            {
                key = cy_bl_bnu_policy.bnu_img_policy[multi_idx].upgrade_auth[0];
            }
            else
            {
                key = cy_bl_bnu_policy.bnu_img_policy[multi_idx].boot_auth[0];
            }
        }
    }

    return key;
}

static int cy_bootutil_check_image_id(const struct flash_area *fap, uint8_t image_id)
{
    int rc = 1;
    int img_idx;

    img_idx = cy_bootutil_get_multi_idx(fap);

    if (img_idx >= 0)
    {
        rc = (int)(image_id != cy_bl_bnu_policy.bnu_img_policy[img_idx].id);
    }

    return rc;
}

static int cy_bootutil_check_upgrade(const struct flash_area *fap)
{
    int rc = 1;
    int img_idx, slot_id;

    slot_id = cy_bootutil_get_slot_id(fap);

    if (slot_id >= 0)
    {
        if (slot_id > 0)
        {
            /* This is an UPGRADE slot */
            img_idx = cy_bootutil_get_multi_idx(fap);

            if (img_idx >= 0)
            {
                rc = (int)(!cy_bl_bnu_policy.bnu_img_policy[img_idx].upgrade);
            }
        }
        else
        {
            /* This is a BOOT slot, no upgrade policy checking */
            rc = 0;
        }
    }

    return rc;
}

/*
 * Verify the integrity of the image.
 * Return non-zero if image could not be validated/does not validate.
 */
int
bootutil_img_validate(struct enc_key_data *enc_state, int image_index,
                      struct image_header *hdr, const struct flash_area *fap,
                      uint8_t *tmp_buf, uint32_t tmp_buf_sz, uint8_t *seed,
                      int seed_len, uint8_t *out_hash)
{
    uint32_t off;
    uint16_t len;
    uint8_t type;
    int valid_sha256 = 0;
    int valid_image_id = 0;
#ifdef EXPECTED_SIG_TLV
    int valid_signature = 0;
    int key_id = 0;
#endif
    struct image_tlv_iter it;
    uint8_t buf[SIG_BUF_SIZE];
    uint8_t hash[32];
    int rc;

    uint8_t image_id = 0u;

    rc = bootutil_img_hash(enc_state, image_index, hdr, fap, tmp_buf,
            tmp_buf_sz, hash, seed, seed_len);
    if (rc) {
        return rc;
    }

    if (out_hash) {
        memcpy(out_hash, hash, 32);
    }

    /* Check for upgrade is enabled in the policy */
    rc = cy_bootutil_check_upgrade(fap);
    if (rc) {
        return rc;
    }

    /*
     * Traverse through all of the TLVs, performing any checks we know
     * and are able to do.
     */
    rc = bootutil_tlv_iter_begin(&it, hdr, fap, IMAGE_TLV_ANY, false);
    if (rc) {
        return rc;
    }

    while (true)
    {
        rc = bootutil_tlv_iter_next(&it, &off, &len, &type);
        if (rc < 0) {
            return -1;
        } else if (rc > 0) {
            break;
        }

        switch (type)
        {
            case IMAGE_TLV_CYSB_IMAGE_ID:
                rc = flash_area_read(fap, off, &image_id, len);
                if (rc) {
                    return rc;
                }

                /* Check for image ID is equal to ID from the policy */
                rc = cy_bootutil_check_image_id(fap, image_id);
                if (rc) {
                    return -1;
                }

                valid_image_id = 1;

                break;
            case IMAGE_TLV_SHA256:
                {
                    /*
                     * Verify the SHA256 image hash.  This must always be
                     * present.
                     */
                    if (len == sizeof(hash)) {

                        rc = flash_area_read(fap, off, buf, sizeof hash);
                        if (rc) {
                            return rc;
                        }

                        rc = memcmp(hash, buf, sizeof(hash));
                        if (rc == 0) {
                            valid_sha256 = 1;
                        }
                    }
                }
                break;
        #ifdef EXPECTED_SIG_TLV
            case IMAGE_TLV_KEYHASH:
                {
                    /*
                     * Determine which key we should be checking.
                     */
                    if (len > 32) {
                        return -1;
                    }
                    rc = flash_area_read(fap, off, buf, len);
                    if (rc) {
                        return rc;
                    }

                    key_id = cy_bootutil_find_key(fap);

                    /*
                     * The key may not be found, which is acceptable.  There
                     * can be multiple signatures, each preceded by a key.
                     */
                }
                break;
            case EXPECTED_SIG_TLV:
                {
                    /* Ignore this signature if it is out of bounds. */
                    if (key_id <= 0 || key_id > (int)CY_FB_MAX_KEY_COUNT) {
                        key_id = -1;
                        continue;
                    }
                    if (!EXPECTED_SIG_LEN(len) || len > sizeof(buf)) {
                        return -1;
                    }
                    rc = flash_area_read(fap, off, buf, len);
                    if (rc) {
                        return -1;
                    }
                    rc = bootutil_verify_sig(hash, sizeof(hash), buf, len, key_id);
                    if (rc == 0) {
                        valid_signature = 1;
                    }
                    key_id = 0;
                }
                break;
        #endif /* EXPECTED_SIG_TLV */
            default:
                break;
        }
    }

    if (!valid_sha256) {
        BOOT_LOG_INF("Invalid SHA256 digest of bootable image") ;
        return -1;
    }

    if (!valid_image_id) {
        BOOT_LOG_INF("Invalid image ID of bootable image") ;
        return -1;
    }

#ifdef EXPECTED_SIG_TLV
    if (!valid_signature) {
        BOOT_LOG_INF("Invalid signature of bootable image") ;
        return -1;
    }
#endif

    return 0;
}
