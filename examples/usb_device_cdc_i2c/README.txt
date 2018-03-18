This folder contains a USB-I2C bridge, currently it just supports I2C write at fixed 100kHz SCLK frequency.

It uses P3.3 as SCLK and P3.4 as SDAT, and it supports CH551. Transport is limited to 16byte. 

This firmware and commandline tools provided you a cheap and neat (driverless) way to access the I2C bus.

BTW, it contains a program to configure a SI5351 on the bus. This example will let you know this protocol more easily.

And this commandline program can be use as SI5351 HF local oscillator tuner, it can turn your USB-I2C bridge, TCXO and SI5351 into a VFO.

Protocol is very simple, and I will describe it as follows. All response will contains <CR> <LF> for EOT.

Q for request target clock generator's reference frequency.

V for get firmware version.

E for escape this device to internal bootloader for in-application re-programming.

B for get baudrate, virtually, just for debug.

T for an I2C transmission. Format is T<1-byte length(<64)> <AR & data, must be short than 64-byte>. If success, you will get "OK\r\n" message, if it fails, you will get "Fnn\r\n" response. nn describes what stage the error occurs.

Notice that if the highest bit on address is set, A transmission will NOT to send a I2C stop.

R for an I2C reception. Format is R<AR><Length(<64)>, if there's no response from bus on send address, the device will sent "FAIL\r\n" message to your serial port. You would get length-bytes message from device.

Authors:

Zhiyuan Wan <h@iloli.bid>

Copyleft, MIT license, 2018-3-17.
