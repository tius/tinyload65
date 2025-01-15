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
.include "macros_delay.inc"
.include "macros_align.inc"

;------------------------------------------------------------------------------
_OUT_LO := SERIAL_TX_PORT_IDLE ^ (1 << SERIAL_TX_PORT_PIN) 
_OUT_HI := SERIAL_TX_PORT_IDLE | (1 << SERIAL_TX_PORT_PIN)

;==============================================================================
.if SERIAL_TX_PORT_PIN = 0

.out "using optimized tx_byte for bit 0"
;------------------------------------------------------------------------------
.zeropage

_tmp:   .res 1

.code
;==============================================================================
tx_byte:
;------------------------------------------------------------------------------
;   in:
;       A       byte to transmit
;------------------------------------------------------------------------------
    sta _tmp                            ; 3
    lda #_OUT_LO                        ; 2
    sta SERIAL_TX_PORT                  ; 4     start bit
    sec                                 ; 2     end of byte marker
    ror _tmp                            ; 5     _tmp = $80, c = 0
    DELAY6                              ; 6     
                                        ; 22    

    ;   repeat 8 times          
@l0:        
    adc #$00                            ; 2
    sta SERIAL_TX_PORT                  ; 4     
    lda #_OUT_LO                        ; 2
    lsr a:_tmp                          ; 6     use absolute addressing (!)
    ASSERT_BRANCH_PAGE bne, @l0         ; 3/2   zero after last data bit
                                        ; 17    total 135 = 17 * 8 - 1

    lda #_OUT_HI                        ; 2     stop bit
    nop                                 ; 2
    sta SERIAL_TX_PORT                  ; 4     
    rts                                 ; 6
                                        ; 14
;   total 171 = 22 + 135 + 14                                                

;==============================================================================
.else

.out "using default tx_byte"
;------------------------------------------------------------------------------
;   in:
;       A       byte to transmit
;------------------------------------------------------------------------------
    phx                                 ; 3

    ;   load x and y with bit masks for hi and lo output
    ldx #_OUT_LO                        ; 2
    stx SERIAL_TX_PORT                  ; 4     start bit
    sec                                 ; 2     end of byte marker
    ror                                 ; 2
    DELAY6                              ; 6 
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
    DELAY3                              ; 3
@l2:		                    
    lsr			                        ; 2
    ASSERT_BRANCH_PAGE bne, @l0		    ; 3/2
                                        ; 17    total 135 = 17 * 8 - 1

    DELAY5                              ; 5
    ldx #_OUT_HI                        ; 2
    stx SERIAL_TX_PORT                  ; 4     stop bit

    plx                                 ; 4
    rts                                 ; 6
                                        ; 21
;   total 175 = 19 + 135 + 21

.endif
;==============================================================================


