/*
 * si5351.h 
 *
 * Copyright (C) 2018 Zhiyuan Wan <h@iloli.bid>
 *
 * This file is deriived from Jason Milldrum's AVR SI5351 library.
 * Copyright (C) 2014 Jason Milldrum <milldrum@gmail.com>
 *
 * Many defines derived from clk-si5351.h in the Linux kernel.
 * Sebastian Hesselbarth <sebastian.hesselbarth@gmail.com>
 * Rabeeh Khoury <rabeeh@solid-run.com>
 *
 * do_div() macro derived from /include/asm-generic/div64.h in
 * the Linux kernel.
 * Copyright (C) 2003 Bernardo Innocenti <bernie@develer.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SI5351_H_
#define SI5351_H_

/* Define definitions */

#define SI5351_BUS_BASE_ADDR				0xC0
#define SI5351_XTAL_FREQ					26000000
#define SI5351_PLL_FIXED					900000000

/*
 * Si5351A Rev B Configuration Register Export Header File
 *
 * This file represents a series of Silicon Labs Si5351A Rev B 
 * register writes that can be performed to load a single configuration 
 * on a device. It was created by a Silicon Labs ClockBuilder Pro
 * export tool.
 *
 * Part:		                                       Si5351A Rev B
 * Design ID:                                          
 * Includes Pre/Post Download Control Register Writes: Yes
 * Created By:                                         ClockBuilder Pro v2.21 [2018-01-19]
 * Timestamp:                                          2018-03-16 18:42:24 GMT+08:00
 *
 * A complete design report corresponding to this export is included at the end 
 * of this header file.
 *
 */

#define SI5351A_REVB_REG_CONFIG_NUM_REGS				57

typedef struct
{
	unsigned int address; /* 16-bit register address */
	unsigned char value; /* 8-bit register data */

} si5351a_revb_register_t;

si5351a_revb_register_t const si5351a_revb_registers[SI5351A_REVB_REG_CONFIG_NUM_REGS] =
{
	{ 0x0002, 0x53 },
	{ 0x0003, 0x00 },
	{ 0x0007, 0x00 },
	{ 0x000F, 0x00 },
	{ 0x0010, 0x4F },
	{ 0x0011, 0x0F },
	{ 0x0012, 0x0F },
	{ 0x0013, 0x8C },
	{ 0x0014, 0x8C },
	{ 0x0015, 0x8C },
	{ 0x0016, 0x8C },
	{ 0x0017, 0x8C },
	{ 0x001A, 0x00 },
	{ 0x001B, 0x0D },
	{ 0x001C, 0x00 },
	{ 0x001D, 0x0D },
	{ 0x001E, 0x93 },
	{ 0x001F, 0x00 },
	{ 0x0020, 0x00 },
	{ 0x0021, 0x09 },
	{ 0x002A, 0x00 },
	{ 0x002B, 0x01 },
	{ 0x002C, 0x00 },
	{ 0x002D, 0x01 },
	{ 0x002E, 0x00 },
	{ 0x002F, 0x00 },
	{ 0x0030, 0x00 },
	{ 0x0031, 0x00 },
	{ 0x0032, 0x00 },
	{ 0x0033, 0x01 },
	{ 0x0034, 0x00 },
	{ 0x0035, 0x19 },
	{ 0x0036, 0x00 },
	{ 0x0037, 0x00 },
	{ 0x0038, 0x00 },
	{ 0x0039, 0x00 },
	{ 0x003A, 0x00 },
	{ 0x003B, 0x01 },
	{ 0x003C, 0x01 },
	{ 0x003D, 0x93 },
	{ 0x003E, 0x00 },
	{ 0x003F, 0x00 },
	{ 0x0040, 0x00 },
	{ 0x0041, 0x00 },
	{ 0x005A, 0x00 },
	{ 0x005B, 0x00 },
	{ 0x0095, 0x00 },
	{ 0x0096, 0x00 },
	{ 0x0097, 0x00 },
	{ 0x0098, 0x00 },
	{ 0x0099, 0x00 },
	{ 0x009A, 0x00 },
	{ 0x009B, 0x00 },
	{ 0x00A2, 0x00 },
	{ 0x00A3, 0x00 },
	{ 0x00A4, 0x00 },
	{ 0x00B7, 0x12 },

};

#define SI5351_AR 0xC0

/*
 * Design Report
 *
 * Overview
 * ========
 * Part:         Si5351A
 * Project File: C:\Users\ZHIYUAN\Documents\Si5351A-RevB-Project.slabtimeproj
 * Created By:   ClockBuilder Pro v2.21 [2018-01-19]
 * Timestamp:    2018-03-16 18:42:24 GMT+08:00
 * 
 * Design Rule Check
 * =================
 * Errors:
 * - No errors
 * 
 * Warnings:
 * - No warnings
 * 
 * Design
 * ======
 * Inputs:
 *     IN0: 26 MHz
 * 
 * Outputs:
 *    OUT0: 30 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT1: 15 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 *    OUT2: 1 MHz
 *          Enabled LVCMOS 8 mA
 *          Offset 0.000 s 
 * 
 * Frequency Plan
 * ==============
 * PLL_A:
 *    Enabled Features = None
 *    Fvco             = 900 MHz
 *    M                = 34.6153846153846153... [ 34 + 8/13 ]
 *    Input0:
 *       Source           = Crystal
 *       Source Frequency = 26 MHz
 *       Fpfd             = 26 MHz
 *       Load Capacitance = Not_Applicable
 *    Output0:
 *       Features       = None
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 30 MHz
 *       N              = 30
 *    Output1:
 *       Features       = None
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 15 MHz
 *       N              = 60
 *    Output2:
 *       Features       = None
 *       Disabled State = StopLow
 *       R              = 1  (2^0)
 *       Fout           = 1 MHz
 *       N              = 900
 * 
 * Settings
 * ========
 * 
 * Location      Setting Name   Decimal Value      Hex Value        
 * ------------  -------------  -----------------  -----------------
 * 0x0002[3]     XO_LOS_MASK    0                  0x0              
 * 0x0002[4]     CLK_LOS_MASK   1                  0x1              
 * 0x0002[5]     LOL_A_MASK     0                  0x0              
 * 0x0002[6]     LOL_B_MASK     1                  0x1              
 * 0x0002[7]     SYS_INIT_MASK  0                  0x0              
 * 0x0003[7:0]   CLK_OEB        0                  0x00             
 * 0x0007[7:4]   I2C_ADDR_CTRL  0                  0x0              
 * 0x000F[2]     PLLA_SRC       0                  0x0              
 * 0x000F[3]     PLLB_SRC       0                  0x0              
 * 0x000F[4]     PLLA_INSELB    0                  0x0              
 * 0x000F[5]     PLLB_INSELB    0                  0x0              
 * 0x000F[7:6]   CLKIN_DIV      0                  0x0              
 * 0x0010[1:0]   CLK0_IDRV      3                  0x3              
 * 0x0010[3:2]   CLK0_SRC       3                  0x3              
 * 0x0010[4]     CLK0_INV       0                  0x0              
 * 0x0010[5]     MS0_SRC        0                  0x0              
 * 0x0010[6]     MS0_INT        0                  0x0              
 * 0x0010[7]     CLK0_PDN       0                  0x0              
 * 0x0011[1:0]   CLK1_IDRV      3                  0x3              
 * 0x0011[3:2]   CLK1_SRC       3                  0x3              
 * 0x0011[4]     CLK1_INV       0                  0x0              
 * 0x0011[5]     MS1_SRC        0                  0x0              
 * 0x0011[6]     MS1_INT        0                  0x0              
 * 0x0011[7]     CLK1_PDN       0                  0x0              
 * 0x0012[1:0]   CLK2_IDRV      3                  0x3              
 * 0x0012[3:2]   CLK2_SRC       3                  0x3              
 * 0x0012[4]     CLK2_INV       0                  0x0              
 * 0x0012[5]     MS2_SRC        0                  0x0              
 * 0x0012[6]     MS2_INT        0                  0x0              
 * 0x0012[7]     CLK2_PDN       0                  0x0              
 * 0x0013[1:0]   CLK3_IDRV      0                  0x0              
 * 0x0013[3:2]   CLK3_SRC       3                  0x3              
 * 0x0013[4]     CLK3_INV       0                  0x0              
 * 0x0013[5]     MS3_SRC        0                  0x0              
 * 0x0013[6]     MS3_INT        0                  0x0              
 * 0x0013[7]     CLK3_PDN       1                  0x1              
 * 0x0014[1:0]   CLK4_IDRV      0                  0x0              
 * 0x0014[3:2]   CLK4_SRC       3                  0x3              
 * 0x0014[4]     CLK4_INV       0                  0x0              
 * 0x0014[5]     MS4_SRC        0                  0x0              
 * 0x0014[6]     MS4_INT        0                  0x0              
 * 0x0014[7]     CLK4_PDN       1                  0x1              
 * 0x0015[1:0]   CLK5_IDRV      0                  0x0              
 * 0x0015[3:2]   CLK5_SRC       3                  0x3              
 * 0x0015[4]     CLK5_INV       0                  0x0              
 * 0x0015[5]     MS5_SRC        0                  0x0              
 * 0x0015[6]     MS5_INT        0                  0x0              
 * 0x0015[7]     CLK5_PDN       1                  0x1              
 * 0x0016[1:0]   CLK6_IDRV      0                  0x0              
 * 0x0016[3:2]   CLK6_SRC       3                  0x3              
 * 0x0016[4]     CLK6_INV       0                  0x0              
 * 0x0016[5]     MS6_SRC        0                  0x0              
 * 0x0016[6]     FBA_INT        0                  0x0              
 * 0x0016[7]     CLK6_PDN       1                  0x1              
 * 0x0017[1:0]   CLK7_IDRV      0                  0x0              
 * 0x0017[3:2]   CLK7_SRC       3                  0x3              
 * 0x0017[4]     CLK7_INV       0                  0x0              
 * 0x0017[5]     MS7_SRC        0                  0x0              
 * 0x0017[6]     FBB_INT        0                  0x0              
 * 0x0017[7]     CLK7_PDN       1                  0x1              
 * 0x001C[17:0]  MSNA_P1        3918               0x00F4E          
 * 0x001F[19:0]  MSNA_P2        10                 0x0000A          
 * 0x001F[23:4]  MSNA_P3        13                 0x0000D          
 * 0x002C[17:0]  MS0_P1         3328               0x00D00          
 * 0x002F[19:0]  MS0_P2         0                  0x00000          
 * 0x002F[23:4]  MS0_P4         1                  0x00001          
 * 0x0034[17:0]  MS1_P1         7168               0x01C00          
 * 0x0037[19:0]  MS1_P2         0                  0x00000          
 * 0x0037[23:4]  MS1_P4         1                  0x00001          
 * 0x003C[17:0]  MS2_P1         114688             0x1C000          
 * 0x003F[19:0]  MS2_P2         0                  0x00000          
 * 0x003F[23:4]  MS2_P4         1                  0x00001          
 * 0x005A[7:0]   MS6_P2         0                  0x00             
 * 0x005B[7:0]   MS7_P2         0                  0x00             
 * 0x0095[14:0]  SSDN_P2        0                  0x0000           
 * 0x0095[7]     SSC_EN         0                  0x0              
 * 0x0097[14:0]  SSDN_P3        0                  0x0000           
 * 0x0097[7]     SSC_MODE       0                  0x0              
 * 0x0099[11:0]  SSDN_P1        0                  0x000            
 * 0x009A[15:4]  SSUDP          0                  0x000            
 * 0x00A2[21:0]  VCXO_PARAM     0                  0x000000         
 * 0x00B7[7:6]   XTAL_CL        0                  0x0
 * 
 *
 */


#endif /* SI5351_H_ */

