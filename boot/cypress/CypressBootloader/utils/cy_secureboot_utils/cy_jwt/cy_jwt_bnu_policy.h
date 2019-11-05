/****************************************?***********************************//**
* \file cy_jwt_bnu_policy.h
* \version 1.0
*
* \brief
*  This is the header file for functions to extract Boot and Upgrade policy
*  from the Provisioning packet.
*
*****************************************?***************************************
* \copyright
* Copyright 2019, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*****************************************?**************************************/

#ifndef CY_JWT_BNU_POLICY
#define CY_JWT_BNU_POLICY

#include "cy_jwt_policy.h"

/*
 * Parses provisioning packet (JWT) and looks for provisioning request JWT packet
 * */
int Cy_JWT_ParseProvisioningPacket(char *provPacket, bnu_policy_t *bnuPolicy,
        debug_policy_t *debugPolicy, uint8_t masterImageId);

/*
 * Parses provisioning request packet (JWT) and looks for Boot and Upgrade policy
 * */
int Cy_JWT_ParseProvisioningRequest(char *provReq, char *buff, uint32_t buffLen,
        bnu_policy_t *bnuPolicy, debug_policy_t *debugPolicy,
        uint8_t masterImageId);

#endif /* CY_JWT_BNU_POLICY */
