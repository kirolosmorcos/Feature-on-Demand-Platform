/**
 * @file debug.c
 * @brief Debugging facilities
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2025 Oryx Embedded SARL. All rights reserved.
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

//Dependencies
#include <console_io.h>
#include "mpc5748g.h"
#include "system_mpc5748g.h"
#include "debug.h"


/**
 * @brief Debug UART initialization
 * @param[in] baudrate UART baudrate
 **/

void debugInit(uint32_t baudrate)
{
   uint32_t div;

   //Configure LIN2_TX (PC8)
   SIUL2->MSCR[40] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK | SIUL2_MSCR_SSS(1);

   //Configure LIN2_RX (PC9)
   SIUL2->MSCR[41] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK |
      SIUL2_MSCR_PUS_MASK | SIUL2_MSCR_PUE_MASK;
   SIUL2->IMCR[202] = SIUL2_IMCR_SSS(2);

   //Enter INIT mode
   LINFlexD_2->LINCR1 = LINFlexD_LINCR1_BF_MASK | LINFlexD_LINCR1_INIT_MASK;

   //Wait for the mode transition to complete
   while((LINFlexD_2->LINSR & LINFlexD_LINSR_LINS_MASK) != LINFlexD_LINSR_LINS(1))
   {
   }

   //Select UART mode
   LINFlexD_2->UARTCR = LINFlexD_UARTCR_UART_MASK;
   //Set word length (8 bits)
   LINFlexD_2->UARTCR |= LINFlexD_UARTCR_WL0_MASK;
   //Enable transmitter and receiver
   LINFlexD_2->UARTCR |= LINFlexD_UARTCR_TxEn_MASK | LINFlexD_UARTCR_RxEn_MASK;

   //Compute the baudrate divider
   div = SystemCoreClock / (2 * baudrate);

   //Select baudrate
   LINFlexD_2->LINIBRR = div / 16;
   LINFlexD_2->LINFBRR = div % 16;

   //Enter NORMAL mode
   LINFlexD_2->LINCR1 = LINFlexD_LINCR1_BF_MASK;
}


/**
 * @brief Display the contents of an array
 * @param[in] stream Pointer to a FILE object that identifies an output stream
 * @param[in] prepend String to prepend to the left of each line
 * @param[in] data Pointer to the data array
 * @param[in] length Number of bytes to display
 **/

void debugDisplayArray(FILE *stream,
   const char_t *prepend, const void *data, size_t length)
{
   uint_t i;

   for(i = 0; i < length; i++)
   {
      //Beginning of a new line?
      if((i % 16) == 0)
         printf("%s", prepend);
      //Display current data byte
      printf("%02" PRIX8 " ", *((uint8_t *) data + i));
      //End of current line?
      if((i % 16) == 15 || i == (length - 1))
         printf("\r\n");
   }
}


/**
 * @brief Write to output stream
 * @param[in] handle Handle that identifies a stream
 * @param[in] buffer Pointer to the data buffer
 * @param[in] count Number of characters to be written
 * @return Error code
 **/

int_t __write_console(__file_handle handle, uint8_t *buffer, size_t *count)
{
   size_t i;

   //Process all the characters
   for(i = 0; i < *count; i++)
   {
      //Send character
      ((uint8_t *) &LINFlexD_2->BDRL)[3] = buffer[i];

      //Wait for the transfer to complete
      while((LINFlexD_2->UARTSR & LINFlexD_UARTSR_DTFTFF_MASK) == 0)
      {
      }

      //Clear DTFTFF flag
      LINFlexD_2->UARTSR &= LINFlexD_UARTSR_DTFTFF_MASK;
   }

   //Successful processing
   return __no_io_error;
}


/**
 * @brief Read from input stream
 * @param[in] handle Handle that identifies a stream
 * @param[in] buffer Pointer to the data buffer
 * @param[in] count Maximum number of characters to be read
 * @return Error code
 **/

int_t __read_console(__file_handle handle, uint8_t *buffer, size_t *count)
{
   //Successful processing
   return __no_io_error;
}


/**
 * @brief Close stream
 * @param[in] handle Handle that identifies a stream
 * @return Error code
 **/

int_t __close_console(__file_handle handle)
{
   //Successful processing
   return __no_io_error;
}
