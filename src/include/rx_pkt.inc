;   rx_pkt.inc
;
;   bit-bang 57600 baud software serial packet receiver
;
;   features:
;       - line speed 57600 baud 8n1 @ 1 MHz CPU clock
;       - receive packet with len byte at line speed
;       - timeout 0.72 s per byte
;       - half-duplex only
;
;   config:
;       SERIAL_RX_PORT                  input register
;       SERIAL_RX_PORT_PIN              port pin (must be 7)
;
;   requirements:
;       - port pin must initialized to input
;       - timing requires input on bit 7
;
;   general remarks:
;       - very tight timing requirements
;       - large jitter by start bit detection
;       - code alignment is critical for correct timing
;
;   bit timing:
;       - nominal bit time is 17.36 cycles
;       - tuned sampling timing 26.5/17/17/18/17/17/18/17 for reliable rx
;       - large jitter by start bit detection, 
;         7 cycles (without timeout) or 11 cycles (with timeout)
;       - substract jitter/2 from start-bit delay (26.5)
;
;   byte timing:
;       - nominial byte time is 173.6 cycles
;       - 170 cycles total processing time per byte max.
;       - this allows up to 2.1% baud rate tolerance
;       
;   credits: 
;       - https://forum.6502.org/viewtopic.php?f=2&t=2063&start=45#p98249
;         (clever hack for efficient bit time tuning)
;
;   references:
;       - see https://github.com/tius/tinylib65 for more code snippets
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
.include "macros_delay.inc"
.include "macros_align.inc"

;==============================================================================
.macro RX_PKT buffer, on_timeout
;------------------------------------------------------------------------------
;   read block with len byte at wire speed, timeout 0.72 s per byte
;
;   requriments:
;       buffer      256 bytes
;
;   output:
;       Y           0
;       X           no. of bytes received
;       buffer      received bytes
;
;------------------------------------------------------------------------------
.local @loop, @wait_start, @read_byte, @l1, @l2

    ldx #0                              ; byte counter
    ldy #0                              ; 0.72 s initial timeout

@loop:    
;------------------------------------------------------------------------------
;   wait for start bit with timeout (0.72s with Y=0)
;
;   7 cycles minimal + 11 cycles jitter (12.5 cycles average)

@wait_start:    
    bit SERIAL_RX_PORT                  ; 4
    ASSERT_BRANCH_PAGE bpl , @read_byte ; 3/2
    dec                                 ; 2
    bne @wait_start                     ; 3/2       
                                        ; 2815  total (11 * 256 - 1)

    bit SERIAL_RX_PORT                  ; 4
    bpl @read_byte                      ; 3/2
    dey                                 ; 2 
    ASSERT_BRANCH_PAGE bne ,@wait_start ; 3/2       
    on_timeout

;------------------------------------------------------------------------------
;   initial processing (14 cycles)

@read_byte:
    inx                                 ; 2
    phx                                 ; 3

    ldx #$7f                            ; 2
    ldy #$08                            ; 2     
    lda #%00100100                      ; 2     tuning bits
    bra @l2                             ; 3

;------------------------------------------------------------------------------
;   data bit loop (17/18 cycles per bit, 140 cycles total)

@l1:
    nop                                 ; 2
    nop                                 ; 2
    bcs @l2                             ; 3/2   adjust bit time, controlled by tuning bits
@l2:        
    cpx SERIAL_RX_PORT                  ; 4
    ror                                 ; 2
    dey                                 ; 2    
    ASSERT_BRANCH_PAGE bne, @l1         ; 3/2

;------------------------------------------------------------------------------
;   process data byte (18 cycles)

    eor #$FF                            ; 2     
    plx                                 ; 4
    sta buffer - 1, x                   ; 5

    cpx buffer                          ; 4     1st byte is packet size
    ASSERT_BRANCH_PAGE bne, @loop       ; 3/2

;------------------------------------------------------------------------------
;   total loop time 170 cycles per byte excluding jitter

.endmacro
;==============================================================================
