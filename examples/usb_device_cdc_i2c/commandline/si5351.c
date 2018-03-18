#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "si5351.h"

uint32_t referenceClock;

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

int si5351_write(uint8_t reg, uint8_t value, int fd)
{
	int n;
	char cmd_i2cWrite[16 + 2] = {'T', 0x03, SI5351_BUS_BASE_ADDR, reg, value}; /* T, LEN, 16bytes data */
	char response[32];
		
	n = write(fd, cmd_i2cWrite, 5);
	if(n == -1)
		goto err;
	n = readline(response, 32, fd);
	if(n == -1)
		goto err;
	if(!strcmp(response, "OK"))
		goto err;
	//printf("Write reg %02x\n", reg);
	return 0;
err:
	fprintf(stderr, "Write error during writing to reg 0x%02x\r\n", reg);
	return -1;
}

int si5351_write_bulk(uint8_t reg, uint8_t bytes, uint8_t *value, int fd)
{
	int i;
	for(i = 0; i < bytes; i++)
	{
		if(si5351_write(reg + i, value[i], fd) != 0)
			return -1;
	}
	return 0;
}

int si5351_read(uint8_t reg, uint8_t *value, int fd)
{
	char cmd_i2cWrite[16 + 2] = {'T', 0x82, SI5351_BUS_BASE_ADDR, reg}; /* T, LEN, 16bytes data */
	char cmd_i2cRead [16 + 2] = {'R', SI5351_BUS_BASE_ADDR | 0x01, 0x01}; /* R, AR, LEN */
	char response[32];
	int n;
		
	n = write(fd, cmd_i2cWrite, 4);
	if(n == -1)
		goto err;
	n = readline(response, 32, fd);
	if(n == -1)
		goto err;
	if(!strcmp(response, "OK"))
		goto err;
	n = write(fd, cmd_i2cRead, 3);
	if(n == -1)
		goto err;
	n = read(fd, value, 1);
	if(n == -1)
		goto err;
	return 0;
err:
	fprintf(stderr, "Write error during fetching to reg 0x%02x\r\n", reg);
	return -1;	
}



int32_t ref_correction = 0;
uint32_t plla_freq = 0;
uint32_t pllb_freq = 0;

struct Si5351Status dev_status;
struct Si5351IntStatus dev_int_status;


/*
 * si5351_set_freq(uint32_t freq, uint32_t pll_freq, enum si5351_clock output)
 *
 * Sets the clock frequency of the specified CLK output
 *
 * freq - Output frequency in Hz
 * pll_freq - Frequency of the PLL driving the Multisynth
 *   Use a 0 to have the function choose a PLL frequency
 * clk - Clock output
 *   (use the si5351_clock enum)
 */
void si5351_set_freq(uint32_t freq, uint32_t pll_freq, enum si5351_clock clk, int fd)
{
	struct Si5351RegSet ms_reg, pll_reg;
	enum si5351_pll target_pll;
	uint8_t willsetpll = 0;

	/* Calculate the synth parameters */
	/* If pll_freq is 0, let the algorithm pick a PLL frequency */
	if(pll_freq == 0)
	{
		pll_freq = multisynth_calc(freq, &ms_reg);
		
		willsetpll = 1;
	}
	/* TODO: bounds checking */
	else
	{
		multisynth_recalc(freq, pll_freq, &ms_reg);
	}
	
	/* Determine which PLL to use */
	/* CLK0 gets PLLA, CLK1 gets PLLB */
	/* CLK2 gets PLLB if necessary */
	/* Only good for Si5351A3 variant at the moment */
	if(clk == SI5351_CLK0)
	{
		target_pll = SI5351_PLLA;
		plla_freq = pll_freq;
	}
	else if(clk == SI5351_CLK1)
	{
		target_pll = SI5351_PLLB;
		pllb_freq = pll_freq;
	}
	else
	{
		/* need to account for CLK2 set before CLK1 */
		if(pllb_freq == 0)
		{
			target_pll = SI5351_PLLB;
			pllb_freq = pll_freq;
		}
		else
		{
			target_pll = SI5351_PLLB;
			pll_freq = pllb_freq;
			multisynth_recalc(freq, pll_freq, &ms_reg);
		}
	}

	pll_calc(pll_freq, &pll_reg, ref_correction);
	

	/* Derive the register values to write */

	/* Prepare an array for parameters to be written to */
	uint8_t *params = malloc(sizeof(uint8_t) * 30);
	uint8_t i = 0;
	uint8_t temp;

	/* PLL parameters first */

	if(willsetpll == 1)
	{
		/* Registers 26-27 */
		temp = ((pll_reg.p3 >> 8) & 0xFF);
		params[i++] = temp;

		temp = (uint8_t)(pll_reg.p3  & 0xFF);
		params[i++] = temp;

		/* Register 28 */
		temp = (uint8_t)((pll_reg.p1 >> 16) & 0x03);
		params[i++] = temp;

		/* Registers 29-30 */
		temp = (uint8_t)((pll_reg.p1 >> 8) & 0xFF);
		params[i++] = temp;

		temp = (uint8_t)(pll_reg.p1  & 0xFF);
		params[i++] = temp;

		/* Register 31 */
		temp = (uint8_t)((pll_reg.p3 >> 12) & 0xF0);
		temp += (uint8_t)((pll_reg.p2 >> 16) & 0x0F);
		params[i++] = temp;

		/* Registers 32-33 */
		temp = (uint8_t)((pll_reg.p2 >> 8) & 0xFF);
		params[i++] = temp;

		temp = (uint8_t)(pll_reg.p2  & 0xFF);
		params[i++] = temp;

		/* Write the parameters */
		if(target_pll == SI5351_PLLA)
		{
			si5351_write_bulk(SI5351_PLLA_PARAMETERS, i + 1, params, fd);
		}
		else if(target_pll == SI5351_PLLB)
		{
			si5351_write_bulk(SI5351_PLLB_PARAMETERS, i + 1, params, fd);
		}
	}

	free(params);
	

	/* Now the multisynth parameters */
	params = malloc(sizeof(char) * 30);
	i = 0;

	/* Registers 42-43 */
	temp = (uint8_t)((ms_reg.p3 >> 8) & 0xFF);
	params[i++] = temp;

	temp = (uint8_t)(ms_reg.p3  & 0xFF);
	params[i++] = temp;

	/* Register 44 */
	/* TODO: add code for output divider */
	temp = (uint8_t)((ms_reg.p1 >> 16) & 0x03);
	params[i++] = temp;

	/* Registers 45-46 */
	temp = (uint8_t)((ms_reg.p1 >> 8) & 0xFF);
	params[i++] = temp;

	temp = (uint8_t)(ms_reg.p1  & 0xFF);
	params[i++] = temp;

	/* Register 47 */
	temp = (uint8_t)((ms_reg.p3 >> 12) & 0xF0);
	temp += (uint8_t)((ms_reg.p2 >> 16) & 0x0F);
	params[i++] = temp;

	/* Registers 48-49 */
	temp = (uint8_t)((ms_reg.p2 >> 8) & 0xFF);
	params[i++] = temp;

	temp = (uint8_t)(ms_reg.p2  & 0xFF);
	params[i++] = temp;

	/* Write the parameters */
	switch(clk)
	{
	case SI5351_CLK0:
		si5351_write_bulk(SI5351_CLK0_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK1:
		si5351_write_bulk(SI5351_CLK1_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK2:
		si5351_write_bulk(SI5351_CLK2_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK3:
		si5351_write_bulk(SI5351_CLK3_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK4:
		si5351_write_bulk(SI5351_CLK4_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK5:
		si5351_write_bulk(SI5351_CLK5_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK6:
		si5351_write_bulk(SI5351_CLK6_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	case SI5351_CLK7:
		si5351_write_bulk(SI5351_CLK7_PARAMETERS, i + 1, params, fd);
		si5351_set_ms_source(clk, target_pll, fd);
		break;
	}
	printf("Sets CLK0 output frequency = %d Hz\n", freq);
	free(params);
}

/*
 * si5351_set_pll(uint32_t pll_freq, enum si5351_pll target_pll)
 *
 * Set the specified PLL to a specific oscillation frequency
 *
 * pll_freq - Desired PLL frequency
 * target_pll - Which PLL to set
 *     (use the si5351_pll enum)
 */
void si5351_set_pll(uint32_t pll_freq, enum si5351_pll target_pll, int fd)
{
	struct Si5351RegSet pll_reg;

	pll_calc(pll_freq, &pll_reg, ref_correction);

	/* Derive the register values to write */

	/* Prepare an array for parameters to be written to */
	uint8_t *params = malloc(sizeof(uint8_t) * 30);
	uint8_t i = 0;
	uint8_t temp;

	/* Registers 26-27 */
	temp = ((pll_reg.p3 >> 8) & 0xFF);
	params[i++] = temp;

	temp = (uint8_t)(pll_reg.p3  & 0xFF);
	params[i++] = temp;

	/* Register 28 */
	temp = (uint8_t)((pll_reg.p1 >> 16) & 0x03);
	params[i++] = temp;

	/* Registers 29-30 */
	temp = (uint8_t)((pll_reg.p1 >> 8) & 0xFF);
	params[i++] = temp;

	temp = (uint8_t)(pll_reg.p1  & 0xFF);
	params[i++] = temp;

	/* Register 31 */
	temp = (uint8_t)((pll_reg.p3 >> 12) & 0xF0);
	temp += (uint8_t)((pll_reg.p2 >> 16) & 0x0F);
	params[i++] = temp;

	/* Registers 32-33 */
	temp = (uint8_t)((pll_reg.p2 >> 8) & 0xFF);
	params[i++] = temp;

	temp = (uint8_t)(pll_reg.p2  & 0xFF);
	params[i++] = temp;

	/* Write the parameters */
	if(target_pll == SI5351_PLLA)
	{
		si5351_write_bulk(SI5351_PLLA_PARAMETERS, i + 1, params, fd);
	}
	else if(target_pll == SI5351_PLLB)
	{
		si5351_write_bulk(SI5351_PLLB_PARAMETERS, i + 1, params, fd);
	}
}

/*
 * si5351_clock_enable(enum si5351_clock clk, uint8_t enable)
 *
 * Enable or disable a chosen clock
 * clk - Clock output
 *   (use the si5351_clock enum)
 * enable - Set to 1 to enable, 0 to disable
 */
void si5351_clock_enable(enum si5351_clock clk, uint8_t enable, int fd)
{
	uint8_t reg_val;

	if(si5351_read(SI5351_OUTPUT_ENABLE_CTRL, &reg_val, fd) != 0)
	{
		return;
	}

	if(enable == 1)
	{
		reg_val &= ~(1<<(uint8_t)clk);
	}
	else
	{
		reg_val |= (1<<(uint8_t)clk);
	}

	si5351_write(SI5351_OUTPUT_ENABLE_CTRL, reg_val, fd);
}

/*
 * si5351_drive_strength(enum si5351_clock clk, enum si5351_drive drive)
 *
 * Sets the drive strength of the specified clock output
 *
 * clk - Clock output
 *   (use the si5351_clock enum)
 * drive - Desired drive level
 *   (use the si5351_drive enum)
 */
void si5351_drive_strength(enum si5351_clock clk, enum si5351_drive drive, int fd)
{
	uint8_t reg_val;
	const uint8_t mask = 0x03;

	if(si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk, &reg_val, fd) != 0)
	{
		return;
	}

	switch(drive)
	{
	case SI5351_DRIVE_2MA:
		reg_val &= ~(mask);
		reg_val |= 0x00;
		break;
	case SI5351_DRIVE_4MA:
		reg_val &= ~(mask);
		reg_val |= 0x01;
		break;
	case SI5351_DRIVE_6MA:
		reg_val &= ~(mask);
		reg_val |= 0x02;
		break;
	case SI5351_DRIVE_8MA:
		reg_val &= ~(mask);
		reg_val |= 0x03;
		break;
	default:
		break;
	}

	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val, fd);
}

/*
 * si5351_update_status(void)
 *
 * Call this to update the status structs, then access them
 * via the dev_status and dev_int_status global variables.
 *
 * See the header file for the struct definitions. These
 * correspond to the flag names for registers 0 and 1 in
 * the Si5351 datasheet.
 */
void si5351_update_status(int fd)
{
	si5351_update_sys_status(&dev_status, fd);
	si5351_update_int_status(&dev_int_status, fd);
}

/*******************************/
/* Suggested private functions */
/*******************************/

/*
 * Calculate best rational approximation for a given fraction
 * taking into account restricted register size, e.g. to find
 * appropriate values for a pll with 5 bit denominator and
 * 8 bit numerator register fields, trying to set up with a
 * frequency ratio of 3.1415, one would say:
 *
 * rational_best_approximation(31415, 10000,
 *              (1 << 8) - 1, (1 << 5) - 1, &n, &d);
 *
 * you may look at given_numerator as a fixed point number,
 * with the fractional part size described in given_denominator.
 *
 * for theoretical background, see:
 * http://en.wikipedia.org/wiki/Continued_fraction
 */

void rational_best_approximation(
        unsigned long given_numerator, unsigned long given_denominator,
        unsigned long max_numerator, unsigned long max_denominator,
        uint32_t *best_numerator, uint32_t *best_denominator)
{
	unsigned long n, d, n0, d0, n1, d1;
	n = given_numerator;
	d = given_denominator;
	n0 = d1 = 0;
	n1 = d0 = 1;
	for (;;) {
		unsigned long t, a;
		if ((n1 > max_numerator) || (d1 > max_denominator)) {
			n1 = n0;
			d1 = d0;
			break;
		}
		if (d == 0)
			break;
		t = d;
		a = n / d;
		d = n % d;
		n = t;
		t = n0 + a * n1;
		n0 = n1;
		n1 = t;
		t = d0 + a * d1;
		d0 = d1;
		d1 = t;
	}
	*best_numerator = n1;
	*best_denominator = d1;
}

uint32_t pll_calc(uint32_t freq, struct Si5351RegSet *reg, int32_t correction)
{
	uint32_t ref_freq = referenceClock;
	uint32_t rfrac, denom, a, b, c, p1, p2, p3;
	uint64_t lltmp;

	/* Factor calibration value into nominal crystal frequency */
	/* Measured in parts-per-ten million */
	ref_freq += (uint32_t)((double)(correction / 10000000.0) * (double)ref_freq);

	/* PLL bounds checking */
	if (freq < SI5351_PLL_VCO_MIN)
		freq = SI5351_PLL_VCO_MIN;
	if (freq > SI5351_PLL_VCO_MAX)
		freq = SI5351_PLL_VCO_MAX;

	/* Determine integer part of feedback equation */
	a = freq / ref_freq;

	if (a < SI5351_PLL_A_MIN)
		freq = ref_freq * SI5351_PLL_A_MIN;
	if (a > SI5351_PLL_A_MAX)
		freq = ref_freq * SI5351_PLL_A_MAX;

	/* find best approximation for b/c = fVCO mod fIN */
	denom = 1000L * 1000L;
	lltmp = freq % ref_freq;
	lltmp *= denom;
	//do_div(lltmp, ref_freq);
	lltmp = lltmp / ref_freq;
	rfrac = (uint32_t)lltmp;

	b = 0;
	c = 1;
	if (rfrac)
		rational_best_approximation(rfrac, denom,
				    SI5351_PLL_B_MAX, SI5351_PLL_C_MAX, &b, &c);
				    
	//printf("a = %d, b = %d, c = %d\n", a, b, c);

	/* calculate parameters */
	p3  = c;
	p2  = (128 * b) % c;
	p1  = 128 * a;
	p1 += (128 * b / c);
	p1 -= 512;

	/* recalculate rate by fIN * (a + b/c) */
	lltmp  = ref_freq;
	lltmp *= b;
	//do_div(lltmp, c);
	lltmp = lltmp / c;

	freq  = (uint32_t)lltmp;
	freq += ref_freq * a;

	reg->p1 = p1;
	reg->p2 = p2;
	reg->p3 = p3;
	printf("Actual PLL frequency = %d Hz\n", freq);

	return freq;
}

uint32_t multisynth_calc(uint32_t freq, struct Si5351RegSet *reg)
{
	uint32_t pll_freq;
	uint64_t lltmp;
	uint32_t a, b, c, p1, p2, p3;
	uint8_t divby4;
	
	/* Multisynth bounds checking */
	if (freq > SI5351_MULTISYNTH_MAX_FREQ)
		freq = SI5351_MULTISYNTH_MAX_FREQ;
	if (freq < SI5351_MULTISYNTH_MIN_FREQ)
		freq = SI5351_MULTISYNTH_MIN_FREQ;

	divby4 = 0;
	if (freq > SI5351_MULTISYNTH_DIVBY4_FREQ)
		divby4 = 1;

	/* Find largest integer divider for max */
	/* VCO frequency and given target frequency */
	if (divby4 == 0)
	{
		lltmp = SI5351_PLL_VCO_MAX;
		//do_div(lltmp, freq);
		lltmp = lltmp / freq;
		a = (uint32_t)lltmp;
	}
	else
		a = 4;

	b = 0;
	c = 1;
	pll_freq = a * freq;

	/* Recalculate output frequency by fOUT = fIN / (a + b/c) */
	lltmp  = pll_freq;
	lltmp *= c;
	//do_div(lltmp, a * c + b);
	lltmp = lltmp / (a* c + b);
	
	freq  = (unsigned long)lltmp;

	/* Calculate parameters */
	if (divby4)
	{
		p3 = 1;
		p2 = 0;
		p1 = 0;
	}
	else
	{
		p3  = c;
		p2  = (128 * b) % c;
		p1  = 128 * a;
		p1 += (128 * b / c);
		p1 -= 512;
	}

	reg->p1 = p1;
	reg->p2 = p2;
	reg->p3 = p3;
	
	printf("Automatic selected PLL freq = %d Hz\n", pll_freq);

	return pll_freq;
}

uint32_t multisynth_recalc(uint32_t freq, uint32_t pll_freq, struct Si5351RegSet *reg)
{
	uint64_t lltmp;
	uint32_t rfrac, denom, a, b, c, p1, p2, p3;
	uint8_t divby4;

	/* Multisynth bounds checking */
	if (freq > SI5351_MULTISYNTH_MAX_FREQ)
		freq = SI5351_MULTISYNTH_MAX_FREQ;
	if (freq < SI5351_MULTISYNTH_MIN_FREQ)
		freq = SI5351_MULTISYNTH_MIN_FREQ;

	divby4 = 0;
	if (freq > SI5351_MULTISYNTH_DIVBY4_FREQ)
		divby4 = 1;

	/* Determine integer part of feedback equation */
	a = pll_freq / freq;

	/* TODO: not sure this is correct */
	if (a < SI5351_MULTISYNTH_A_MIN)
		freq = pll_freq / SI5351_MULTISYNTH_A_MIN;
	if (a > SI5351_MULTISYNTH_A_MAX)
		freq = pll_freq / SI5351_MULTISYNTH_A_MAX;

	/* find best approximation for b/c */
	denom = 1000L * 1000L;
	lltmp = pll_freq % freq;
	lltmp *= denom;
	//do_div(lltmp, freq);
	lltmp = lltmp / freq;
	rfrac = (uint32_t)lltmp;

	b = 0;
	c = 1;
	if (rfrac)
		rational_best_approximation(rfrac, denom,
				    SI5351_MULTISYNTH_B_MAX, SI5351_MULTISYNTH_C_MAX, &b, &c);

	/* Recalculate output frequency by fOUT = fIN / (a + b/c) */
	lltmp  = pll_freq;
	lltmp *= c;
	//do_div(lltmp, a * c + b);
	lltmp = lltmp / (a * c + b);
	freq  = (unsigned long)lltmp;

	/* Calculate parameters */
	if (divby4)
	{
		p3 = 1;
		p2 = 0;
		p1 = 0;
	}
	else
	{
		p3  = c;
		p2  = (128 * b) % c;
		p1  = 128 * a;
		p1 += (128 * b / c);
		p1 -= 512;
	}

	reg->p1 = p1;
	reg->p2 = p2;
	reg->p3 = p3;

	return freq;
}

void si5351_update_sys_status(struct Si5351Status *status, int fd)
{
	uint8_t reg_val = 0;

	if(si5351_read(SI5351_DEVICE_STATUS, &reg_val, fd) != 0)
	{
		return;
	}

	/* Parse the register */
	status->SYS_INIT = (reg_val >> 7) & 0x01;
	status->LOL_B = (reg_val >> 6) & 0x01;
	status->LOL_A = (reg_val >> 5) & 0x01;
	status->LOS = (reg_val >> 4) & 0x01;
	status->REVID = reg_val & 0x03;
}

void si5351_update_int_status(struct Si5351IntStatus *int_status, int fd)
{
	uint8_t reg_val = 0;

	if(si5351_read(SI5351_DEVICE_STATUS, &reg_val, fd) != 0)
	{
		return;
	}

	/* Parse the register */
	int_status->SYS_INIT_STKY = (reg_val >> 7) & 0x01;
	int_status->LOL_B_STKY = (reg_val >> 6) & 0x01;
	int_status->LOL_A_STKY = (reg_val >> 5) & 0x01;
	int_status->LOS_STKY = (reg_val >> 4) & 0x01;
}

void si5351_set_ms_source(enum si5351_clock clk, enum si5351_pll pll, int fd)
{
	uint8_t reg_val = 0x0c;
	uint8_t reg_val2;

	if(si5351_read(SI5351_CLK0_CTRL + (uint8_t)clk, &reg_val2, fd) != 0)
	{
		return;
	}

	if(pll == SI5351_PLLA)
	{
		reg_val &= ~(SI5351_CLK_PLL_SELECT);
	}
	else if(pll == SI5351_PLLB)
	{
		reg_val |= SI5351_CLK_PLL_SELECT;
	}
	si5351_write(SI5351_CLK0_CTRL + (uint8_t)clk, reg_val, fd);
}

void si5351_init(int fd)
{
	si5351_write(SI5351_CRYSTAL_LOAD, 0, fd); /* We use external TCXO */
}

int main(int argc, char *argv[])
{ /* 无需关注波特率 */
	char cmd_getReference[] = "Q";
	char cmd_getBaud[] = "B";
	char cmd_Ping[] = "AT";
	char cmd_getVersion[] = "V";
	char response[32];
	int baud = 0;
	uint8_t value;
	int length = 0;
	uint32_t frequency;
	int i;
	int n;
	
	int fd;
	if(argc != 3)
	{
		fprintf(stderr, "%s: <serial port> <output frequency in hertz>\n", argv[0]);
		fprintf(stderr, "\nThis program can access SI5351 through CH55x USB-I2C adapter.\nIt only accesses SI5351 through fixed address 0x60.\nAnd it sets CLK0 output frequency.\n");
		fprintf(stderr, "\nZhiyuan Wan <h@iloli.bid> Mar 2018\n");
		exit(1);
	}
	fd = open(argv[1], O_RDWR | O_NOCTTY);
	if(fd == -1)
	{
		fprintf(stderr, "Can't open serial port!\r\n");
		exit(1);
	}
	n = write(fd, cmd_Ping, sizeof(cmd_Ping) - 1);
	if(n == -1)
		goto err;
	length = readline(response, 32, fd);
	
	if(strncmp(response, "OK", 2))
	{
		fprintf(stderr, "It doesn't like a USB-I2C adapter!\r\n");
		exit(1);		
	}
	
	n = write(fd, cmd_getVersion, sizeof(cmd_getVersion) - 1);
	if(n == -1)
		goto err;
	length = readline(response, 32, fd);
	
	printf("Device firmware version: %s\n", response);
	
	n = write(fd, cmd_getReference, sizeof(cmd_getReference) - 1);
	if(n == -1)
		goto err;
	length = readline(response, 32, fd);
	
	referenceClock = atoi(response);
	
	printf("Reference clock frequency: %d Hz\n", referenceClock);
	
	frequency = atoi(argv[2]);
	
	si5351_init(fd);

	si5351_set_freq(frequency, 0, SI5351_CLK0, fd);
	
	
	printf("Configuration done!\n");
	close(fd);
	return 0;
err:
	close(fd);
	fprintf(stderr, "I/O error!\r\n");
}
