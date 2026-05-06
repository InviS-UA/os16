org 0x500
bits 16

%define ENDL 0xA, 0xD

start:
    mov si, msg
    call print

    cli
    hlt

print:
    ; save registers we will modify
    push si
    push ax
    push bx

.loop:
    lodsb               ; loads next character in al
    or al, al           ; verify if next character is null?
    jz .done

    mov ah, 0xE         ; call bios interrupt
    mov bh, 0           ; set page number to 0
    int 0x10

    jmp .loop

.done:
    pop bx
    pop ax
    pop si

    ret

msg: db 'Hello world from stage2!', ENDL, 0