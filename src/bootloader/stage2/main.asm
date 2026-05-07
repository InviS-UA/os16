bits 16

section .entry

extern cstart_
global start

start:
    ; store bootDrive from dl
    mov [boot_drive], dl

    cli
    ; setup stack
    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp

    ; enable A20
    call EnableA20
    sti

    ; expect boot drive in dl, send it as argument to cstart function
    xor dx, dx
    mov dl, [boot_drive]
    push dx
    call cstart_

    cli
    hlt

EnableA20:
    [bits 16]
    ; disable keyboard
    call A20WaitInput
    mov al, KbdControllerDisableKeyboard
    out KbdControllerCommandPort, al

    ; read control output port
    call A20WaitInput
    mov al, KbdControllerReadCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitInput
    in al, KbdControllerDataPort
    push eax

    ; write control output port
    call A20WaitInput
    mov al, KbdControllerWriteCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitInput
    pop eax
    or al, 2                                    ; bit 2 = A20 bit
    out KbdControllerDataPort, al

    ; enable keyboard
    call A20WaitInput
    mov al, KbdControllerEnableKeyboard
    out KbdControllerCommandPort, al

    call A20WaitInput

    ret

A20WaitInput:
    [bits 16]
    ; wait until status bit 2 (input buffer) is 0
    ; by reading from command port, we read status byte
    in al, KbdControllerCommandPort
    test al, 2
    jnz A20WaitInput

    ret

A20WaitOutput:
    [bits 16]
    ; wait until status bit 1 (output buffer) is 1 so it can be read
    in al, KbdControllerCommandPort
    test al, 1
    jz A20WaitOutput

    ret

KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

boot_drive:                         db 0