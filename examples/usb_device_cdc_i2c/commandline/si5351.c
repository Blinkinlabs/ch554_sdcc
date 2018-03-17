#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "si5351.h"


int readline(char *buffer, int length, int fd)
{
	int spot = 0;
	int n = 0;
	char sbuf;
	do
	{
		n = read(fd, &sbuf, 1);
		
		if(n == -1)
			return -1;
		buffer[spot] = sbuf;
		spot += n;

	} while(sbuf != '\n' && (spot + 2) < length);
	
	if(buffer[spot - 2] == '\r')
		spot--;
	buffer[spot] = '\0';
	return spot;
}

int main(int argc, char *argv[])
{ /* 无需关注波特率 */
	char cmd_getReference[] = "Q";
	char cmd_getVersion[] = "V";
	char cmd_i2cWrite[16 + 2] = {'T', 0}; /* T, LEN, 16bytes data */
	char response[32];
	uint32_t referenceClock;
	int length = 0;
	
	int fd;
	
	fd = open("/dev/ttyACM0", O_RDWR | O_NOCTTY);
	if(fd == -1)
	{
		fprintf(stderr, "Can't open serial port!\r\n");
		exit(1);
	}
	write(fd, cmd_getVersion, sizeof(cmd_getVersion) - 1);
	length = readline(response, 32, fd);
	
	printf("Device firmware version: %s\n", response);
	
	write(fd, cmd_getReference, sizeof(cmd_getReference) - 1);
	length = readline(response, 32, fd);
	
	referenceClock = atoi(response);
	
	printf("Reference clock frequency %d\n", referenceClock);
	
	int i;
	for(i = 0; i < SI5351A_REVB_REG_CONFIG_NUM_REGS; i++)
	{
		cmd_i2cWrite[1] = 0x03;
		cmd_i2cWrite[2] = SI5351_AR;
		cmd_i2cWrite[3] = si5351a_revb_registers[i].address & 0xff;
		cmd_i2cWrite[4] = si5351a_revb_registers[i].value;
		
		//printf("Write to register 0x%02X with 0x%02X\n", si5351a_revb_registers[i].address, si5351a_revb_registers[i].value);
		write(fd, cmd_i2cWrite, 5);
		length = readline(response, 32, fd);
		if(!strcmp(response, "OK"))
		{
			fprintf(stderr, "Write error during writing to reg 0x%02x\r\n", si5351a_revb_registers[i].address & 0xff);
			exit(1);
		}
	}
		
	printf("Configuration done!\n");
	close(fd);
	return 0;
}
