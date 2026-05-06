org 0x7C00
bits 16

%define ENDL 0xA, 0xD

;
; FAT12 header
;

jmp short start
nop

bpb_oem:                    db 'MSWIN4.1'
bpb_bytes_per_sector:       dw 512
bpb_sectors_per_cluster:    db 1
bpb_reserved_sectors:       dw 1
bpb_fat_count:              db 2
bpb_dir_entries_count:      dw 0E0h
bpb_total_sectors:          dw 2880
bpb_media_descriptor_type:  db 0F0h
bpb_sectors_per_fat:        dw 9
bpb_sectors_per_track:      dw 18
bpb_heads:                  dw 2
bpb_hidden_sectors:         dd 0
bpb_large_sector_count:     dd 0

; extended boot record
ebr_drive_number:           db 0
                            db 0            ; reserved
ebr_signature:              db 0x29
ebr_volume_id:              db 1, 2, 3, 4
ebr_volume_label:           db 'OS 16      '
ebr_system_id:              db 'FAT12   '

times 90-($-$$) db 0

;
; Code goes here
;

start:
    ; setup data segments
    mov ax, 0
    mov ds, ax
    mov es, ax
    
    ; setup stack
    mov ss, ax
    mov sp, 0x7C00              ; stack grows downwards from where we are loaded in memory

    ; some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
    ; expected location
    push es
    push word .after

    retf

.after:
    ; read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    ; check extensions present
    mov ah, 0x41
    mov bx, 0x55AA
    stc
    int 13h

    jc .no_disk_extensions
    cmp bx, 0xAA55
    jne .no_disk_extensions

    ; extensions are present
    mov byte [have_extensions], 1
    jmp .after_disk_extensions_check

.no_disk_extensions:
    mov byte [have_extensions], 0

.after_disk_extensions_check:
    ; load stage2
    mov si, stage2_location

    mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
    mov es, ax

    mov bx, STAGE2_LOAD_OFFSET

.loop:
    mov eax, [si]
    add si, 4
    mov cl, [si]
    inc si

    cmp eax, 0
    je .read_finish

    call disk_read

    xor ch, ch
    shl cx, 5
    mov di, es
    add di, cx
    mov es, di

    jmp .loop

.read_finish:
    ; jump to our stage2
    mov dl, [ebr_drive_number]          ; boot device in dl

    mov ax, STAGE2_LOAD_SEGMENT         ; set segment registers
    mov ds, ax
    mov es, ax

    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

    jmp wait_key_and_reboot             ; should never happen

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

disk_error:
    mov si, msg_read_failed
    call print
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h
    jmp 0FFFFh:0

;
; Reads sectors from a disk
; Parameters:
;   - eax: LBA address
;   - cl: number of sectors to read (up to 128)
;   - dl: drive number
;   - es:bx: memory address where to store read data
;
disk_read:
    push eax                            ; save registers we will modify
    push bx
    push cx
    push dx
    push si
    push di

    cmp byte [have_extensions], 1
    jne disk_error

    ; with extensions
    mov [extensions_dap.lba], eax
    mov [extensions_dap.segment], es
    mov [extensions_dap.offset], bx
    mov [extensions_dap.count], cl

    mov ah, 0x42
    mov si, extensions_dap
    mov di, 3                           ; retry count

.retry:
    pusha                               ; save all registers, we don't know what bios modifies
    stc                                 ; set carry flag, some BIOS'es don't set it
    int 13h                             ; carry flag cleared = success
    jnc .done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp disk_error

.done:
    popa

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop eax                            ; restore registers modified

    ret

;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc disk_error
    popa

    ret

msg_read_failed: db 'Read from disk failed!', ENDL, 0

have_extensions:        db 0
extensions_dap:
    .size:              db 10h
                        db 0
    .count:             dw 0
    .offset:            dw 0
    .segment:           dw 0
    .lba:               dq 0

STAGE2_LOAD_SEGMENT     equ 0x0
STAGE2_LOAD_OFFSET      equ 0x500

times 510-30-($-$$) db 0

stage2_location:        times 30 db 0

dw 0AA55h

buffer: