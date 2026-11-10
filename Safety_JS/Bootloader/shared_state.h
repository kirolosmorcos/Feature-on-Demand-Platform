/*
 * shared_state.h
 *
 *  Created on: 7 Mar 2026
 *      Author: johns
 */

#ifndef SHARED_STATE_H_
#define SHARED_STATE_H_




#include <stdint.h>

/* This struct lives in the .shared linker section (0x400BFF00, 256 bytes)
   It survives resets and is shared between bootloader and application */
typedef struct {
    uint32_t bootFlag;        /* was 0x40040004 - core boot trigger    */
    uint32_t appValidFlag;    /* was 0x40040008 - app valid marker      */
    uint32_t updateModeFlag;  /* was 0x4004000C - bootloader update mode*/
    uint32_t reserved[57];   /* padding to reach offset 0xF0           */
    uint32_t sigControl;      /* was 0x40040100 - signature control     */
    uint32_t sigResult;       /* was 0x40040104 - signature result      */
} SharedState_t;

/* Placed in .shared section by linker (0x400BFF00) */
extern volatile SharedState_t gSharedState;

/* Placed in .noinit section by linker (0x400BFE00) */
extern volatile uint32_t gFlashedArray[32];

/* Placed in .noinit section by linker (0x400BFE80) */
extern volatile uint32_t gSignatureArray[18];







#endif /* SHARED_STATE_H_ */
