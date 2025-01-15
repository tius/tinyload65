# tinyload65

small serial bootloader for 65xx systems

* convenient software upload for fast development cycles
* remote debugging
* system startup delayed by less than 1 second
* no additional hardware required
* small code size
  * ~170 bytes (software upload only)
  * ~220 bytes (full debug support)
* can be integrated into existing firmware

## status

* target component fully implemented
* basic functions of the host component implemented
* little tested so far, but everything seems to work

## overview

### bootloader (tinyload65)

* small resident application
* invoked by reset vector
* waits for host commands on start-up
* allow the host to read and write the target memory
* normal system boot after short timeout (0.7 s)

### in-system debugging

* hook nmi and/or brk vector
* give host full control of the target memory and processor state

### interface

* software-only serial interface
* 57600 baud, half-duplex @ 1MHz cpu clock
* two port lines required (rx and tx)
* reset controlled by dtr
* nmi controlled by rts (optional)
* interface can be used as terminal port after start-up
* 6-pin debug connector (rx, tx, reset, nmi, gnd, vcc)

### host-based uploader (tinydude65)

* small control application
* command line interface
* upload and download data
* test nmi functionality

### host-based debugger (tbd)

* interactive host application
* trigger nmi
* manage breakpoints
* memory dump
* dissassembler
* single-stepping
* debug log

## requirements

interface

* 1 port line for rx (bit 7 required)
* 1 port line for tx (bit 0 prefered)
* port may be re-used during normal operation

zeropage use (startup only)

* 3 bytes temporary data

ram usage (startup only)

* 256 byte ram for packet buffer
* can be reduced if required by using a smaller packet size

## basic features

* identify bootloader version
* write target memory
* write reset, nmi and irq vector

## optional features

* identify target system
* read target memory
* read and write reset, nmi and irq vectors
* read and write registers

## protocol

### general

* software serial receive requires very tight timing
* block size 256 bytes max.
* no checksums
* written data may be verified via read command if required

### packet format

* packet size (1 byte)
* packet data (1 .. 255 bytes)

### timeouts

* serial timeout 0.7 seconds
* timeout terminates bootloader

### error handling

* unknown commands are ignored
* invalid packets terminate bootloader
* no parameter checks

## basic messages

### reset >

    R <version_lo> <version_hi>  [ <device_data ...> ]

* signal reset signal to host system
* send protocol version
* send target device data (optional)
* target is now ready to receive commands

### write <

    W <addr16> <data ...> 

* write data to target memory (252 bytes max.)
* target system responds with ack

### ack >

    . 

* acknowledge host command

## extended messages

### nmi >

    N <addr16>

* signal nmi signal to host system
* _addr16_ points to register values (sp, y, x, a, st, pc)
* host system may read and modify registers

### brk >

    B <addr16>

* signal brk signal to host system
* _addr16_ points to register values (sp, y, x, a, st, pc)
* host system may read and modify registers

### read <

    R <addr16> <size8>

* read memory at _addr16_
* target system responds with message containing requested data
* may also be used to prevent bootloader timeout

## bootloader api

* tx_byte: send one byte via serial interface

## wiring

    pa0     out     serial      tx          use bit 0 for optimized code
    pa1     
    pa2     
    pa3     
    pa4     
    pa5     
    pa6     
    pa7     in      serial      rx          bit 7 required for 57600 baud
