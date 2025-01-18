;   tx_byte_def.inc
;       
;   caveats:
;       - sets all other output pins to idle state
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
;==============================================================================
.include "macros_delay.inc"

.out "using default tx_byte"

_OUT_LO := SERIAL_TX_PORT_IDLE ^ (1 << SERIAL_TX_PORT_PIN) 
_OUT_HI := SERIAL_TX_PORT_IDLE | (1 << SERIAL_TX_PORT_PIN)

.code
;==============================================================================
tx_byte:
;   in:
;       A       byte to transmit
;------------------------------------------------------------------------------
    phx                                 ; 3

    ldx #_OUT_LO                        ; 2
    stx SERIAL_TX_PORT                  ; 4     start bit
    sec                                 ; 2     end of byte marker
    ror                                 ; 2
    nop                                 ; 2
    nop                                 ; 2
    nop                                 ; 2
                                        ; 19    total

;   repeat 8 times  
@l0:	    
    bcc @l1		                        ; 3/2 	
    DELAY1                              ; 1     hack, 65c02 only
    ldx #_OUT_HI                        ; 2
    stx	SERIAL_TX_PORT                  ; 4
    bcs @l2		                        ; 3
@l1:		                    
    ldx #_OUT_LO                        ; 2
    stx SERIAL_TX_PORT                  ; 4			
    bit $0                              ; 3
@l2:		                    
    lsr			                        ; 2
    ASSERT_BRANCH_PAGE bne, @l0		    ; 3/2
                                        ; 17    total 135 = 17 * 8 - 1

    nop                                 ; 2
    bit $0                              ; 3
    ldx #_OUT_HI                        ; 2
    stx SERIAL_TX_PORT                  ; 4     stop bit

    plx                                 ; 4
    rts                                 ; 6
                                        ; 21
;   total 175 = 19 + 135 + 21

;==============================================================================
