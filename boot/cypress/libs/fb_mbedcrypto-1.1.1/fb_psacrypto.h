/***************************************************************************//**
* \file flashboot_psacrypto.h
* \version 1.0
*
* \brief
*  This is the header file for the flashboot psacrypto syscalls data sizes.
*
********************************************************************************
* \copyright
*
* © 2019, Cypress Semiconductor Corporation
* or a subsidiary of Cypress Semiconductor Corporation. All rights
* reserved.
*
* This software, including source code, documentation and related
* materials (“Software”), is owned by Cypress Semiconductor
* Corporation or one of its subsidiaries (“Cypress”) and is protected by
* and subject to worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, non-
* exclusive, non-transferable license to copy, modify, and compile the
* Software source code solely for use in connection with Cypress�s
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
* significant property damage, injury or death (“High Risk Product”). By
* including Cypress’s product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*
******************************************************************************/

#ifndef FB_PSACRYPTO_H
#define FB_PSACRYPTO_H

#include "fb_mbed_tls_config.h"

#include "fb_mbedtls/fb_cipher.h"

#include "fb_cyhal_crypto_common.h"
#include "fb_cyhal_hw_resources.h"

#include "fb_cryptolib/crypto_driver/fb_cy_crypto_common.h"
// #include "fb_cryptolib/crypto_driver/fb_cy_crypto_core_sha_v1.h"
// #include "fb_cryptolib/crypto_driver/fb_cy_crypto_core_sha_v2.h"
#include "fb_cryptolib/crypto_driver/fb_cy_crypto_core_sha.h"

#include "fb_cryptolib/mbedtls_target/fb_crypto_common.h"

#include "fb_sha256_alt.h"

#include "fb_psacrypto/fb_crypto.h"
#include "fb_psacrypto/fb_crypto_platform.h"
#include "fb_psacrypto/fb_crypto_types.h"
#include "fb_psacrypto/fb_crypto_sizes.h"
#include "fb_psacrypto/fb_crypto_struct.h"

#endif /* FB_PSACRYPTO_H */
