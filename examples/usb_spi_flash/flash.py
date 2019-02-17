#!/usr/bin/python3

# Pad a file to a given size

import argparse
import os
import serial
import time

parser = argparse.ArgumentParser(description='Pad a file to a sector size with 0xFF')
parser.add_argument('-s', dest='pageSize', type=int, default=256, help='Page size')
parser.add_argument('-p', dest='port', default='/dev/ttyACM0', help='Port to write to')
parser.add_argument('file', help='Filename to write to')

args=parser.parse_args()

#check if file exists

size = os.path.getsize(args.file)

padding = args.pageSize - (size % args.pageSize)

paddedSize = size+padding

print("size:", size)
print("padded size:", paddedSize)

#f = open(args.file, mode='ab')
#
#for i in range(0,padding):
#    f.write(b'\xFF')
#
#exit(1)
#
#sizeBytes = (hex(paddedSize)[2:])
#ser.write(b'u0000 ' + sizeBytes.encode("utf-8") + b'\n')

startTime = time.time()

ser = serial.Serial(args.port)

ser.write(b't')  # Put FPGA in reset, and take control of SPI bus
ser.flush()
ser.write(b'w')  # Start writing data to SPI Flash
ser.flush()

ser.write(bytes([(paddedSize >> 0 ) & 0xFF]))
ser.flush()
ser.write(bytes([(paddedSize >> 8 ) & 0xFF]))
ser.flush()
ser.write(bytes([(paddedSize >> 16) & 0xFF]))
ser.flush()
ser.write(bytes([(paddedSize >> 24) & 0xFF]))
ser.flush()


with open(args.file, mode='rb') as file:
    data = bytearray(file.read())

    bytesWritten = 0;

    for i in range(0,padding):
        data.append(255)    # Fuck you python 3

    chunkSize = 64 # pageSize must be a multiple of this

    if len(data)%chunkSize != 0:
        print("Bad chunk size")
        exit(1)

    chunkCount = (len(data)//chunkSize)

    print('[', end='', flush=True)

    for chunk in range(0,chunkCount):
        written = ser.write(data[chunk*chunkSize:(chunk+1)*chunkSize])
        bytesWritten += written

        if written != chunkSize:
            print("wrote:", written, "chunk:", chunk)

        #time.sleep(.002)

        if (bytesWritten % args.pageSize) == 0:
            print('#', end='', flush=True)
            time.sleep(.002)    # TODO: Fix timing bug in firmware?

    time.sleep(.1)
    ser.write(b'T')  # Release SPI bus, and take FPGA out of reset mode

endTime = time.time()

print("]")
print("Wrote %d bytes in %0.2f seconds (%0.2fKB/s)"%(bytesWritten, endTime-startTime,bytesWritten/(endTime-startTime)/1024))
