bits 16

global x86_Video_WriteCharTeletype
x86_Video_WriteCharTeletype:
    push bp
    mov bp, sp

    push bx

    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]
    
    int 10h

    pop bx

    mov sp, bp
    pop bp

    ret

global x86_Disk_CheckExetensionsPresent
x86_Disk_CheckExetensionsPresent:
    push bp
    mov bp, sp

    push bx

    mov ah, 41h
    mov dl, [bp + 4]
    mov bx, 055AAh

    int 13h

    mov ax, 1
    sbb ax, 0
    
    pop bx

    mov sp, bp
    pop bp

    ret

global x86_Disk_ExtendedRead
x86_Disk_ExtendedRead:
    push bp
    mov bp, sp

    push ds
    push si
    push bx

    mov ah, 42h
    mov dl, [bp + 4]
    mov si, [bp + 6]

    int 13h

    mov ax, 1
    sbb ax, 0
    
    pop bx
    pop si
    pop ds

    mov sp, bp
    pop bp

    ret

global x86_Disk_Reset
x86_Disk_Reset:
    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp + 4]

    int 13h

    mov ax, 1
    sbb ax, 0

    mov sp, bp
    pop bp

    ret