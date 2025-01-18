;   tx_byte_opt.inc
;
;   requirements:
;       - SERIAL_TX_PORT_PIN must be 0
;       
;   remarks:
;       - state of other output pins is not changed
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
.if SERIAL_TX_PORT_PIN <> 0
    .error "SERIAL_TX_PORT_PIN must be 0 for optimized tx_byte"
.endif

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
    lda SERIAL_TX_PORT                  ; 4     
    dec                                 ; 2
    sta SERIAL_TX_PORT                  ; 4     start bit
    sec                                 ; 2     end of byte marker
    ror _tmp                            ; 5     _tmp = $80, c = 0
    nop                                 ; 2
    nop                                 ; 2
    nop                                 ; 2
                                        ; 26    

    ;   repeat 8 times          
@l0:        
    adc #$00                            ; 2
    sta SERIAL_TX_PORT                  ; 4     
    and #$FE                            ; 2
    lsr a:_tmp                          ; 6     use absolute addressing (!)
    ASSERT_BRANCH_PAGE bne, @l0         ; 3/2   zero after last data bit
                                        ; 17    total 135 = 17 * 8 - 1

    ora #$01                            ; 2     stop bit
    nop                                 ; 2
    sta SERIAL_TX_PORT                  ; 4     
    rts                                 ; 6
                                        ; 14
;   total 175 = 26 + 135 + 14                                                

;==============================================================================
