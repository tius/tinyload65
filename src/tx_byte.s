;   tx_byte.s
;
;   bit-bang 57600 baud software serial output
;
;   config:
;       SERIAL_TX_PORT             output register
;       SERIAL_TX_PORT_PIN         output pin number (0 for fastest code)
;       SERIAL_TX_PORT_IDLE        output register default state
;
;   requirements:
;       - data direction register set to output for SERIAL_TX_BIT
;       - output register initialized for output high
;
;   remarks:
;       - half-duplex only
;       - correct bit time is 17.36 cycles, tight timing is required
;       - simple timing 19/17/17/17/17/17/17/17/18 is precise enough for tx
;       - maximum waveform timing error is 1.64 cycles
;       - branches must not cross pages for correct timing 
;       - using bit 0 allows faster and smaller code
;       - not keeping port output state allows further optimization
;
;   references:
;       - see https://github.com/tius/tinylib65 for latest version
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
.include "config.inc"
.include "macros_align.inc"

.if SERIAL_TX_USE_MIN
    .include "tx_byte_min.inc"
.endif
.if SERIAL_TX_USE_OPT
    .include "tx_byte_opt.inc"
.endif
.if SERIAL_TX_USE_DEF
    .include "tx_byte_def.inc"
.endif
.if SERIAL_TX_USE_SAFE
    .include "tx_byte_safe.inc"                     ; not yet implemented
.endif
