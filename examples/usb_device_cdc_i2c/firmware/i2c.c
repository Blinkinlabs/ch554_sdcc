/********************************** (C) COPYRIGHT *******************************
* File Name		: I2C.C
* Author		: Zhiyuan Wan
* License		: MIT
* Version		: V1.0
* Date			: 2018/03/17
* Description		: 8051 软件 I2C
*******************************************************************************/
#include <8051.h>
#include <stdint.h>
#include "i2c.h"

#define I2C_SDAT	P3_4
#define I2C_SCLK	P3_3



void i2c_init()
{ /* GPIO port initial */
	I2C_SDAT = 1;
	I2C_SCLK = 1;
}

void i2c_delay()
{
	volatile char i = 1;
	while(i--);
}

void i2c_start()
{
	I2C_SDAT = 1;
	I2C_SCLK = 1;
	i2c_delay();
	
	I2C_SDAT = 0;
	i2c_delay();
	
	I2C_SCLK = 0;
	i2c_delay();
}

void i2c_stop()
{
	I2C_SDAT = 0;
	I2C_SCLK = 1;
	i2c_delay();
	
	I2C_SDAT = 1;
	i2c_delay();
}

void i2c_write(uint8_t data)
{
	int i;
	
	for(i = 0; i < 8; i++)
	{	
		data <<= 1;
		I2C_SDAT = CY;
		
		I2C_SCLK = 1;
		i2c_delay();
		
		I2C_SCLK = 0;
		i2c_delay();
	}
}

uint8_t i2c_read()
{
	int i;
	uint8_t ret = 0;
	
	I2C_SDAT = 1;
	for(i = 0; i < 8; i++)
	{
		ret <<= 1;
		I2C_SCLK = 1;
		i2c_delay();
		
		if(I2C_SDAT)
			ret |= 0x01;
		
		I2C_SCLK = 0;
		i2c_delay();

	}
	return ret;
}

bool i2c_read_ack()
{
	bool status;
	
	I2C_SDAT = 1;
	
	I2C_SCLK = 1;
	i2c_delay();
	
	status = I2C_SDAT;
	
	I2C_SCLK = 0;
	i2c_delay();
	
	return !status;
}

bool i2c_read_nak()
{
	return !i2c_read_ack();
}
