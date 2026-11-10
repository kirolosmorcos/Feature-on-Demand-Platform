#ifndef MBEDTLS_CONFIG_H
#define MBEDTLS_CONFIG_H

/* ============================================================
 * Minimal Mbed TLS config
 * Use case:
 * - Parse PEM public key
 * - SHA-256 hash
 * - RSA signature verification
 * ============================================================ */

/* ---------- System support ---------- */

/* Uncomment only if your toolchain/platform needs this */
// #define MBEDTLS_HAVE_ASM
// #define MBEDTLS_HAVE_TIME
// #define MBEDTLS_HAVE_TIME_DATE

/* Platform abstraction */
#define MBEDTLS_PLATFORM_C

/* ---------- Feature support ---------- */

/* Required for RSA PKCS#1 v1.5 verification */
#define MBEDTLS_PKCS1_V15

/* ---------- Required modules ---------- */

/* Core parsing and math */
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_BASE64_C
#define MBEDTLS_BIGNUM_C
#define MBEDTLS_OID_C

/* PEM public key parsing */
#define MBEDTLS_PEM_PARSE_C

/* Public key abstraction + parsing */
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C

/* RSA */
#define MBEDTLS_RSA_C

/* Digest layer + SHA-256 */
#define MBEDTLS_MD_C
#define MBEDTLS_SHA256_C

/* ---------- Optional but harmless ---------- */

/* Only keep if you want mbedtls_strerror() */
 #define MBEDTLS_ERROR_C

/* Only keep if your compiler complains about deprecated APIs */
// #define MBEDTLS_DEPRECATED_REMOVED

/* ============================================================
 * Excluded on purpose
 * Not needed for your current signature verification path
 * ============================================================ */

/* No TLS / SSL */
/* No X509 certificate parsing */
/* No ECC / ECDSA / ECDH */
/* No AES / DES / GCM / CCM / cipher layer */
/* No entropy / DRBG / PSA */
/* No MD5 / SHA1 / SHA512 */

/* ============================================================
 * Include config checks
 * ============================================================ */
#include "mbedtls/check_config.h"

#endif /* MBEDTLS_CONFIG_H */
