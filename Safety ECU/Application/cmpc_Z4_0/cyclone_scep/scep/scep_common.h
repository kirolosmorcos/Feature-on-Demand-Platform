/**
 * @file scep_common.h
 * @brief SCEP common definitions
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2024-2025 Oryx Embedded SARL. All rights reserved.
 *
 * This file is part of CycloneSCEP Open.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.4
 **/

#ifndef _SCEP_COMMON_H
#define _SCEP_COMMON_H

//Dependencies
#include "scep_config.h"
#include "core/crypto.h"
#include "pkix/x509_common.h"


/*
 * CycloneSCEP Open is licensed under GPL version 2. In particular:
 *
 * - If you link your program to CycloneSCEP Open, the result is a derivative
 *   work that can only be distributed under the same GPL license terms.
 *
 * - If additions or changes to CycloneSCEP Open are made, the result is a
 *   derivative work that can only be distributed under the same license terms.
 *
 * - The GPL license requires that you make the source code available to
 *   whoever you make the binary available to.
 *
 * - If you sell or distribute a hardware product that runs CycloneSCEP Open,
 *   the GPL license requires you to provide public and full access to all
 *   source code on a nondiscriminatory basis.
 *
 * If you fully understand and accept the terms of the GPL license, then edit
 * the os_port_config.h header and add the following directive:
 *
 * #define GPL_LICENSE_TERMS_ACCEPTED
 */

#ifndef GPL_LICENSE_TERMS_ACCEPTED
   #error Before compiling CycloneSCEP Open, you must accept the terms of the GPL license
#endif

//Version string
#define CYCLONE_SCEP_VERSION_STRING "2.5.4"
//Major version
#define CYCLONE_SCEP_MAJOR_VERSION 2
//Minor version
#define CYCLONE_SCEP_MINOR_VERSION 5
//Revision number
#define CYCLONE_SCEP_REV_NUMBER 4

//Nonce size
#define SCEP_NONCE_SIZE 16

//C++ guard
#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief SCEP message types
 **/

typedef enum
{
   SCEP_MSG_TYPE_RESERVED    = 0,  ///<Reserved
   SCEP_MSG_TYPE_CERT_REP    = 3,  ///<CertRep
   SCEP_MSG_TYPE_RENEWAL_REQ = 17, ///<RenewalReq
   SCEP_MSG_TYPE_PKCS_REQ    = 19, ///<PKCSReq
   SCEP_MSG_TYPE_CERT_POLL   = 20, ///<CertPoll
   SCEP_MSG_TYPE_GET_CERT    = 21, ///<GetCert
   SCEP_MSG_TYPE_GET_CRL     = 22  ///<GetCRL
} ScepMessageType;


/**
 * @brief PKI status
 **/

typedef enum
{
   SCEP_PKI_STATUS_SUCCESS = 0, ///<Success
   SCEP_PKI_STATUS_FAILURE = 2, ///<Failure
   SCEP_PKI_STATUS_PENDING = 3  ///<Pending
} ScepPkiStatus;


/**
 * @brief Fail info
 **/

typedef enum
{
   SCEP_FAIL_INFO_BAD_ALG           = 0, ///<badAlg
   SCEP_FAIL_INFO_BAD_MESSAGE_CHECK = 1, ///<badMessageCheck
   SCEP_FAIL_INFO_BAD_REQUEST       = 2, ///<badRequest
   SCEP_FAIL_INFO_BAD_TIME          = 3, ///<badTime
   SCEP_FAIL_INFO_BAD_CERT_ID       = 4  ///<badCertId
} ScepFailInfo;


/**
 * @brief CA capabilities
 */

typedef enum
{
   SCEP_CA_CAPS_NONE               = 0x00, ///<None
   SCEP_CA_CAPS_AES                = 0x01, ///<AES
   SCEP_CA_CAPS_DES3               = 0x02, ///<DES3
   SCEP_CA_CAPS_GET_NEXT_CA_CERT   = 0x04, ///<GetNextCACert
   SCEP_CA_CAPS_POST_PKI_OPERATION = 0x08, ///<POSTPKIOperation
   SCEP_CA_CAPS_RENEWAL            = 0x10, ///<Renewal
   SCEP_CA_CAPS_SHA1               = 0x20, ///<SHA-1
   SCEP_CA_CAPS_SHA256             = 0x40, ///<SHA-256
   SCEP_CA_CAPS_SHA512             = 0x80, ///<SHA-512
} ScepCaCaps;


/**
 * @brief Issuer and subject
 **/

typedef struct
{
   X509Name issuer;
   X509Name subject;
} ScepIssuerAndSubject;


//SCEP related constants
extern const uint8_t SCEP_MESSAGE_TYPE_OID[10];
extern const uint8_t SCEP_PKI_STATUS_OID[10];
extern const uint8_t SCEP_FAIL_INFO_OID[10];
extern const uint8_t SCEP_SENDER_NONCE_OID[10];
extern const uint8_t SCEP_RECIPIENT_NONCE_OID[10];
extern const uint8_t SCEP_TRANSACTION_ID_OID[10];
extern const uint8_t SCEP_FAIL_INFO_TEXT_OID[8];

//C++ guard
#ifdef __cplusplus
}
#endif

#endif
