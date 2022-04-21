prepare_function:
    mov rax, [ebp + 8]
    mov rdx, [ebp + 16]
    ; int rax
loop:
    jmp loop
