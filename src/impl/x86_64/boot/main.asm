; OS entry point
global start

extern long_mode_start

section .text
bits 32 ; still 32bit mode
start:
    mov esp, stack_top ; set the stack pointer to the empty stack

    ; turn on long mode
    call check_multiboot
    call check_cpuid
    call check_long_mode

    ; enable paging
    call setup_page_tables
    call enable_paging

    lgdt [gdt64.pointer]
    jmp gdt64.code_segement:long_mode_start

    hlt ; halt cpu


check_multiboot:
    cmp eax, 0x36d76289
    jne .no_multiboot
    ret

.no_multiboot:
    mov al, "M" ; set the error code 'M', no multiboot
    jmp error

check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    cmp eax, ecx

    je .no_cpuid
    ret

.no_cpuid:
    mov al, "C" ; set the error code 'C', no cpuid
    jmp error

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    
    ret

.no_long_mode:
    mov al, "L" ; set the error code 'L', no long mode
    jmp error

setup_page_tables:
    mov eax, page_table_l3 ; first 12 bits cpu flags
    or eax, 0b11 ; enable present, writable flags
    mov [page_table_l4], eax

    mov eax, page_table_l2 ; first 12 bits cpu flags
    or eax, 0b11 ; enable present, writable flags
    mov [page_table_l3], eax

    ; for loop
    mov ecx, 0 ; counter
.loop:

    mov eax, 0x200000 ; 2MiB
    mul ecx ; next page address
    or eax, 0b10000011 ; present, writable, huge page
    mov [page_table_l2 + ecx * 8], eax

    inc ecx
    cmp ecx, 512 ; if whole table is mapped
    jne .loop ; if not loop

    ret

enable_paging:
    ; pass page table location to cpu
    mov eax, page_table_l4
    mov cr3, eax

    ; enable physical address extension
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    ret

error:
    ; print "ERR: {ERROR CODE}"
    mov dword [0xb8000], 0x4f524f45
    mov dword [0xb8004], 0x4f3a4f52
    mov dword [0xb8008], 0x4f204f20
    mov byte [0xb800a], al
    hlt ; halt cpu

section .bss
align 4096
page_table_l4:
    resb 4096
page_table_l3:
    resb 4096
page_table_l2:
    resb 4096
stack_bottom:
    resb 4096 * 4 ; reserve 16KB memory
stack_top:


section .rodata
gdt64:
    dq 0 ; zero entry
.code_segement: equ $ - gdt64
    ;  executable,             present     64-bit
    dq (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) ; code segment

.pointer:
    dw $ - gdt64-1
    dq gdt64
