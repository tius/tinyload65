;   tinyload65.s
;
;   small serial bootloader for 65xx systems
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
.include "rx_pkt.inc"

;==============================================================================
.zeropage
;------------------------------------------------------------------------------
.org DATA_ZP

;   read / write pointer
ptr:        .res 2

.code

;==============================================================================
.bss
;------------------------------------------------------------------------------
.org DATA_BSS

pkt_buffer: .res 256

;------------------------------------------------------------------------------
.org APP_START

res_vector: .res 2                  ; reset vector
nmi_vector: .res 2                  ; nmi vector
irq_vector: .res 2                  ; irq vector

.reloc

.code
;==============================================================================
res_handler:
;------------------------------------------------------------------------------
    ;   initialize hardware stack
    ldx #$ff                
    txs
    
    ;   initialize port 
    lda #ORA_IDLE
    sta via1_ora                        ; avoid glitches by setting ora first
    lda #ORA_MASK           
    sta via1_ddra

;------------------------------------------------------------------------------
;   send reset packet and run bootloader

    lda #'R'
    ldx #VERSION_LO
    ldy #VERSION_HI
    jsr bootloader                      ; inlining bootloader would save 5 bytes

    jmp (res_vector)

;==============================================================================
irq_handler:                            ; cld is not required for 65c02
;------------------------------------------------------------------------------
.if FEAT_BRK
    pha
    phx                                 ; do not save Y to minimize latency
    tsx                     
    lda $0103,x
    and #$10                            ; check pushed status byte for "B flag" 
    bne @brk
    jmp (irq_vector)

;------------------------------------------------------------------------------
@brk:   
    lda #'B'

.if FEAT_NMI
    bra _process_interrupt
.endif    

;------------------------------------------------------------------------------
.else
    jmp (irq_vector)

.endif
;==============================================================================
nmi_handler:
;------------------------------------------------------------------------------
.if FEAT_NMI
    pha
    lda #'N'
    phx

;------------------------------------------------------------------------------
;   handle nmi or brk
;
;   remarks:
;       - rti will resume _two_ bytes after the brk instruction
;       - the host may adjust the return address if required
;       - the host may also change other registers values
;       - if sp is changed, the stack content must be adjusted for rti

.if FEAT_NMI || FEAT_BRK
_process_interrupt:
    phy
    tsx
    phx

;------------------------------------------------------------------------------
;   send nmi or brk packet and run bootloader
;
;   A:      packet type
;   X/Y:    address of registers on stack

    ldy #$01                            
    jsr bootloader                      

;------------------------------------------------------------------------------
;   restore registers and resume

    pla                                 ; discard sp
    ply
    plx
    pla
    rti

.endif    

;------------------------------------------------------------------------------
.else
    jmp (nmi_vector)

.endif
;==============================================================================
bootloader:
;------------------------------------------------------------------------------
;   send inital packet 
;
;   A:  packet type
;   X:  1st parameter byte
;   Y:  2nd parameter byte

    pha
    lda #4
    jsr tx_byte
    pla
    jsr tx_byte
    txa

_tx_bytes_ay:
    jsr tx_byte
    tya
    jsr tx_byte

;------------------------------------------------------------------------------
;   process command packets

_wait_packet:
    stz pkt_buffer + 1                  ; clear packet type

    ;   receive packet, exit on timeout or invalid packets
    RX_PKT pkt_buffer, rts

    lda pkt_buffer + 1                  ; load packet type

.if FEAT_READ
    cmp #'R'
    beq _cmd_read
.endif    

    cmp #'W'

    ;   ignore unknown commands
    bne _wait_packet

;------------------------------------------------------------------------------
;   process write command 

_cmd_write:
    lda pkt_buffer + 2                  ; load address low
    sta ptr
    lda pkt_buffer + 3                  ; load address high   
    sta ptr + 1

    ;   store data (no parameter checks!)
    ;   X:  packet size
    ;   Y:  0
@loop:
    lda pkt_buffer + 4, y
    sta (ptr), y
    iny
    dex
    cpx #5
    bcs @loop 

    ;   send ack packet
    lda #2
    ldy #'.'
    bra _tx_bytes_ay

;------------------------------------------------------------------------------
;   process read command 

.if FEAT_READ

_cmd_read:
    lda pkt_buffer + 2                  ; load address low
    sta ptr
    lda pkt_buffer + 3                  ; load address high   
    sta ptr + 1

    lda pkt_buffer + 4                  ; load size
    inc
    jsr tx_byte

    ldy #0
@loop:
    lda (ptr), y
    jsr tx_byte
    iny
    cpy pkt_buffer + 4
    bne @loop

    bra _wait_packet

.endif    

;==============================================================================
.segment "VECTORS"
;------------------------------------------------------------------------------
;   hardware vectors

.word   nmi_handler
.word   res_handler
.word   irq_handler
