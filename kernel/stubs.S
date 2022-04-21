extern exception_handler
extern irq_handler

%macro push_registers 0
    push QWORD r15
    push QWORD r14
    push QWORD r13
    push QWORD r12
    push QWORD r11
    push QWORD r10
    push QWORD r9
    push QWORD r8
    push QWORD rdi
    push QWORD rsi
    push QWORD rdx
    push QWORD rcx
    push QWORD rbx
    push QWORD rax
%endmacro

%macro pop_registers 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro isr_err_stub 1
isr_stub_%+%1:
    push QWORD %1
    push_registers
    call exception_handler
    pop_registers
    add rsp, 16
    iretq
%endmacro
; if writing for 64-bit, use iretq instead
%macro isr_no_err_stub 1
isr_stub_%+%1:
    cli
    push QWORD 0x180
    push QWORD %1
    push_registers
    call exception_handler
    pop_registers
    add rsp, 16
    iretq
%endmacro

%macro irq 2
global irq_%+%2
irq_%+%2:
    cli
    push QWORD 0x180
    push QWORD %1
    push_registers
    call irq_handler
    pop_registers
    add rsp, 16
    iretq
%endmacro


isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31
irq 32,0
irq 80,1
irq 0x80,2


global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32
    dq isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep