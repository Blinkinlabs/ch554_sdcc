/*
 * Copyright (c) 2013-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ----------------------------------------------------------------------
 *
 * $Date:        1. December 2017
 * $Revision:    V2.0.0
 *
 * Project:      CMSIS-DAP Source
 * Title:        SW_DP.c CMSIS-DAP SW DP I/O
 *
 *---------------------------------------------------------------------------*/
// SDCC/8051 optimizations by Ralph Doncaster 2021

#include "DAP.h"

volatile uint8_t swdelay;

#define SW_CLOCK_CYCLE() \
  SWK = 1; SWK = 0;

#define SW_WRITE_BIT(bits) \
  SWD = (bits)&1; SWK = 1; SWK = 0;

#define SW_READ_BIT(bits) \
  bits = SWD; SWK = 1; SWK = 0;

// todo: look at difference between SWJ & SWD sequence
// use SW_WRITE_BIT macro?
// Generate SWJ Sequence
//   count:  sequence bits count
//   data:   pointer to sequence bits data
//   return: none
void SWJ_Sequence(uint8_t count, const __xdata uint8_t *data)
{
    uint8_t val;
    uint8_t n;

    val = 0U;
    n = 0U;
    while (count--)
    {
        if (n == 0U)
        {
            val = *data++;
            n = 8U;
        }

        SW_WRITE_BIT(val);
        val >>= 1;
        n--;
    }
}

// Generate SWD Sequence
//   info:   sequence information
//   swdo:   pointer to SWDIO generated data
//   swdi:   pointer to SWDIO captured data
//   return: none
void SWD_Sequence(uint8_t info, const __xdata uint8_t *swdo, __xdata uint8_t *swdi)
{
    uint8_t val;
    uint8_t bits;
    uint8_t n, k;

    n = info & SWD_SEQUENCE_CLK;
    if (n == 0U)
    {
        n = 64U;
    }

    if (info & SWD_SEQUENCE_DIN)
    {
        while (n)
        {
            val = 0U;
            for (k = 8U; k && n; k--, n--)
            {
                SW_READ_BIT(bits);
                val >>= 1;
                val |= bits << 7;
            }
            val >>= k;
            *swdi++ = (uint8_t)val;
        }
    }
    else
    {
        while (n)
        {
            val = *swdo++;
            for (k = 8U; k && n; k--, n--)
            {
                SW_WRITE_BIT(val);
                val >>= 1;
            }
        }
    }
}

// SWD Transfer I/O
//   request: A[3:2] RnW APnDP
//   data:    DATA[31:0]
//   return:  ACK[2:0]
uint8_t SWD_Transfer(uint8_t req, __xdata uint8_t *data)
{
    uint8_t ack;
    uint8_t bits = req;
    uint8_t val;
    uint8_t parity;

    uint8_t m, n;

    /* Packet req */
    parity = 0U;
    SW_WRITE_BIT(1U); /* Start Bit */
    SW_WRITE_BIT(bits); /* APnDP Bit */
    parity += bits;
    bits >>= 1;
    SW_WRITE_BIT(bits); /* RnW Bit */
    parity += bits;
    bits >>= 1;
    SW_WRITE_BIT(bits); /* A2 Bit */
    parity += bits;
    bits >>= 1;
    SW_WRITE_BIT(bits); /* A3 Bit */
    parity += bits;
    SW_WRITE_BIT(parity); /* Parity Bit */
    SW_WRITE_BIT(0U);     /* Stop Bit */
    SW_WRITE_BIT(1U);     /* Park Bit */

    /* Turnaround is always 1 cycle */
    SWD_OUT_DISABLE();
    SW_CLOCK_CYCLE();

    /* Acknowledge res */
    SW_READ_BIT(bits);
    ack = bits << 0;
    SW_READ_BIT(bits);
    ack |= bits << 1;
    SW_READ_BIT(bits);
    ack |= bits << 2;

    if (ack == DAP_TRANSFER_OK)
    {
        /* OK res */
        /* Data transfer */
        if (req & DAP_TRANSFER_RnW)
        {
            /* Read data */
            val = 0U;
            parity = 0U;
            for (m = 0; m < 4; m++)
            {
                for (n = 8U; n; n--)
                {
                    SW_READ_BIT(bits); /* Read RDATA[0:31] */
                    parity += bits;
                    val >>= 1;
                    //val |= bits << 7;
                    if (bits) val |= 0x80;
                }
                if (data)
                {
                    data[m] = val;
                }
            }
            SW_READ_BIT(bits); /* Read Parity */
            if ((parity ^ bits) & 1U)
            {
                ack = DAP_TRANSFER_ERROR;
            }

            /* Turnaround */
            SW_CLOCK_CYCLE();
            SWD_OUT_ENABLE();
        }
        else        // write data
        {
            /* Turnaround */
            SW_CLOCK_CYCLE();
            SWD_OUT_ENABLE();

            // Write WDATA[0:31]
            parity = 0U;
            for (m = 0; m < 4; m++)
            {
                val = data[m];
                ACC = val;
                //parity += P;
                if (P) parity++;
                for (n = 8U; n; n--)
                {
                    SW_WRITE_BIT(val);
                    val >>= 1;
                }
                /*
                __asm
                mov r2, #8
                1$:
                rrc A
                mov P1.6, C
                setb P1.7
                clr P1.7
                djnz r2, 1$
                __endasm;
                */

            }
            SW_WRITE_BIT(parity); /* Write Parity Bit */
        }
        /* Idle cycles */
        SWD = 0;
        n = idle_cycles;
        if (n)
        {
            for (; n; n--)
            {
                SW_CLOCK_CYCLE();
            }
        }
        SWD = 1;
        return (ack);
    }

    if ((ack == DAP_TRANSFER_WAIT) || (ack == DAP_TRANSFER_FAULT))
    {
        /* Turnaround */
        SW_CLOCK_CYCLE();
        SWD_OUT_ENABLE();

        SWD = 1;
        return (ack);
    }

    /* Protocol error - clock out 32bits + parity + turnaround */
    n = 32U + 1U + 1U;
    do {
        SW_CLOCK_CYCLE();       // back off data phase
    } while (--n);

    SWD_OUT_ENABLE();
    SWD = 1;
    return (ack);
}
