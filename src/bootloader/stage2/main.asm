bits 16

section .entry

extern cstart_
global start

start:
    cli
    ; setup stack
    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp
    sti

    ; expect boot drive in dl, send it as argument to cstart function
    xor dh, dh
    push dx
    call cstart_

    cli
    hlt