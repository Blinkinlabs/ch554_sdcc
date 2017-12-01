# Convert a Keil C51 format platform file into SDCC format

import argparse
import shutil
import re
from collections import OrderedDict

parser = argparse.ArgumentParser(description='Convert a Keil C51 format platform file into SDCC format')
parser.add_argument('source', help='Source filename')
parser.add_argument('destination', help='Output filename')

args = parser.parse_args()

inputFileName = args.source
outputFileName = args.destination


outputFile = open(outputFileName, 'w')


# Track SFR addresses, in order to inject them into SBIT definitions
sfrAddresses = {}


replacements = OrderedDict([
	('UINT8X',	'__xdata uint8_t'),
	('UINT16X',	'__xdata uint16_t'),
	('UINT24X',	'__xdata uuint24_t'),
	('UINT32X',	'__xdata uuint32_t'),
	('INT8X',	'__xdata uint8_t'),
	('INT16X',	'__xdata uint16_t'),
	('INT24X',	'__xdata uint24_t'),
	('INT32X',	'__xdata uint32_t'),

	('UINT8C',	'__code uint8_t'),
	('UINT16C',	'__code uint16_t'),
	('UINT24C',	'__code uuint24_t'),
	('UINT32C',	'__code uuint32_t'),
	('INT8C',	'__code uint8_t'),
	('INT16C',	'__code uint16_t'),
	('INT24C',	'__code uint24_t'),
	('INT32C',	'__code uint32_t'),

	('UINT8I',	'__idata uint8_t'),
	('UINT16I',	'__idata uint16_t'),
	('UINT24I',	'__idata uuint24_t'),
	('UINT32I',	'__idata uuint32_t'),
	('INT8I',	'__idata uint8_t'),
	('INT16I',	'__idata uint16_t'),
	('INT24I',	'__idata uint24_t'),
	('INT32I',	'__idata uint32_t'),

	('UINT8',	'uint8_t'),
	('UINT16',	'uint16_t'),
	('UINT24',	'uint24_t'),
	('UINT32',	'uint32_t'),
	('INT8',	'int8_t'),
	('INT16',	'int16_t'),
	('INT24',	'int24_t'),
	('INT32',	'int32_t'),
])


with open(inputFileName) as inputFile:
	for line in inputFile:
		# do token replacment
		for i, j in replacements.iteritems():
        		line = line.replace(i, j)

		words = line.split()

		if len(words) == 0:
			outputFile.write('\n')

		elif words[0] == ('sfr'):
			# Check for an SFR declaration
			#
			# Input should be:
			# sfr IE              = 0xA8;         // interrupt enable
			#
			# And we want to convert it to:
			# SFR(IE, 0xA8); // interrupt enable

			sfrLine = 'SFR(' + words[1] + ',\t' + words[3].rstrip(';') + ');'
			sfrLine += '\t' + ' '.join(words[4:])

			outputFile.write(sfrLine + '\n')

			sfrAddresses[words[1]] = words[3].rstrip(';')

		elif words[0] == ('sfr16'):
			# Check for an SFR declaration
			#
			# Input should be:
			# sfr16 ROM_ADDR      = 0x84;         // address for flash-ROM, little-endian
			#
			# And we want to convert it to:
			# SFR16(IE, 0x84); // interrupt enable

			sfrLine = 'SFR16(' + words[1] + ',\t' + words[3].rstrip(';') + ');'
			sfrLine += '\t' + ' '.join(words[4:])

			outputFile.write(sfrLine + '\n')

		elif words[0] == ('sbit'):
			# Check for a SBIT declaration
			#
			# Input should be:
			#  sbit EA            = IE^7;         // enable global interrupts: 0=disable, 1=enable if E_DIS=0
			# 
			# And we want to convert it to:
			#    SBIT(EA,       0xA8, 7); // Global Interrupt Enable
			#
			# Note that we need to have already seen the address for the SFR that this bit is in,
			# so that it can be substituted into the output.

			sbitLine = '   SBIT(' + words[1] + ',\t' + sfrAddresses[words[3].split('^')[0]] + ', ' + words[3].split('^')[1].rstrip(';') + ');'
			sbitLine += '\t' + ' '.join(words[4:])

			outputFile.write(sbitLine + '\n')

		else:
			# We don't understand the line, so pass it through
			outputFile.write(line)
