# Convert a Keil C51 format platform file into SDCC format

import argparse
import shutil

parser = argparse.ArgumentParser(description='Convert a Keil C51 format platform file into SDCC format')
parser.add_argument('source', help='Source filename')
parser.add_argument('destination', help='Output filename')

args = parser.parse_args()

inputFileName = args.source
outputFileName = args.destination


outputFile = open(outputFileName, 'w')

sfrAddresses = {}

with open(inputFileName) as inputFile:
	for line in inputFile:
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
