;   config.inc
;
;------------------------------------------------------------------------------
;   MIT License
;
;   Copyright (c) 1978-2025 Matthias Waldorf, https://tius.org
;
;   Permission is hereby granted, free of charge, to any person obtaining a copy
;   of this software and associated documentation files (the "Software"), to deal
;   in the Software without restriction, including without limitation the rights
;   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;   copies of the Software, and to permit persons to whom the Software is
;   furnished to do so, subject to the following conditions:
;
;   The above copyright notice and this permission notice shall be included in all
;   copies or substantial portions of the Software.
;
;   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;   SOFTWARE.
;------------------------------------------------------------------------------

;==============================================================================
;   assembler settings
;------------------------------------------------------------------------------
.pc02                                   ; allow 65c02 opcodes
.feature string_escapes                 ; allow \r \n ...

;==============================================================================
;   general
;------------------------------------------------------------------------------
;   bootloader version

VERSION_LO  = 01
VERSION_HI  = 25

;==============================================================================
;   bootloader features
;------------------------------------------------------------------------------
FEAT_READ   = 1                     ; enable read support
FEAT_NMI    = 1                     ; enable NMI support
FEAT_BRK    = 0                     ; enable BRK support

;------------------------------------------------------------------------------
;   ram memory map

DATA_ZP     = $0080                  ; start address for zero page data
DATA_BSS    = $0200                  ; start address for other data
APP_START   = $1000                  ; start address for application

;==============================================================================
;   hardware settings
;------------------------------------------------------------------------------
via1        := $e810

via1_orb    := via1 
via1_ira    := via1 + 1
via1_ora    := via1 + 1
via1_ddrb   := via1 + 2
via1_ddra   := via1 + 3
via1_pcr    := via1 + 12

;------------------------------------------------------------------------------
;   pin assignments
;
;   remarks:
;       - using default values for output pins allows optimized code
;       - care must be taken to avoid unintended changing other output lines
;------------------------------------------------------------------------------
;   port    dir     module      usage       default 
;
;   pa0     out     serial      tx          hi
;   pa1 
;   pa2 
;   pa3     
;   pa4     
;   pa5     
;   pa6     
;   pa7     in      serial      rx

ORA_MASK    = %00000001                ; output pins
ORA_IDLE    = %00000001                ; output idle values (tx high)

;==============================================================================
;   modules
;------------------------------------------------------------------------------
;   serial_rx

SERIAL_RX_PORT      = via1_ira
SERIAL_RX_PORT_PIN  = 7

;------------------------------------------------------------------------------
;   serial_tx

SERIAL_TX_PORT      = via1_ora
SERIAL_TX_PORT_IDLE = ORA_IDLE
SERIAL_TX_PORT_SAVE = 0
SERIAL_TX_PORT_PIN  = 0

.global     tx_byte
;------------------------------------------------------------------------------

