#
#Save all registers in the stack.
#Push all registers except RSP,RIP,CS,DS,ES,SS,which are automatically pushed by call
#

.globl context_switch
context_switch:
    push    %r15
    push    %r14
    push    %r13
    push    %r12
    push    %rbp
    push    %rbx
    sub     $4, %rsp
    fnstcw  (%rsp)
    sub     $4, %rsp
    stmxcsr (%rsp)
    mov     %rsp, (%rsi)
    mov     (%rdi), %rsp
    ldmxcsr (%rsp)
    add     $4, %rsp
    fldcw   (%rsp)
    add     $4, %rsp
    pop     %rbx
    pop     %rbp
    pop     %r12
    pop     %r13
    pop     %r14
    pop     %r15
    ret
