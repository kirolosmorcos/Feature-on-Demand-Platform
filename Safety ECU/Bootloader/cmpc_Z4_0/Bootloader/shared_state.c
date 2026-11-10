/*
 * shared_state.c
 *
 *  Created on: 7 Mar 2026
 *      Author: johns
 */

#include "shared_state.h"



/* 256 bytes shared region - survives reset, visible to all cores */
__attribute__((section(".shared")))
volatile SharedState_t gSharedState;

/* No-init region - not zeroed on startup */
__attribute__((section(".noinit")))
volatile uint32_t gFlashedArray[32];

__attribute__((section(".noinit")))
volatile uint32_t gSignatureArray[18];
