.globl save_context, load_context, call_with_new_stack, co_memcpy
# void save_context(ctx *save, void *new_stack, void (*handler)(void))
save_context:
    movq    %rbx,    (%rdi)
    movq    %rbp,   8(%rdi)
    movq    %r12,  16(%rdi)
    movq    %r13,  24(%rdi)
    movq    %r14,  32(%rdi)
    movq    %r15,  40(%rdi)
#    popq    %rax            # return ptr
    lea     retaddr(%rip),  %rax
    movq    %rsp,  48(%rdi)
    movq    %rax,  56(%rdi)
    stmxcsr        64(%rdi)
    fnstcw         68(%rdi)
    movq    %rsi, %rsp
    jmpq   *%rdx
retaddr:
    ret


# void load_context(ctx *saved)
load_context:
    movq      (%rdi), %rbx
    movq     8(%rdi), %rbp
    movq    16(%rdi), %r12
    movq    24(%rdi), %r13
    movq    32(%rdi), %r14
    movq    40(%rdi), %r15
    movq    48(%rdi), %rsp
    movq    56(%rdi), %rax # return ptr
    ldmxcsr 64(%rdi)
    fldcw   68(%rdi)
    jmpq      *%rax

# void call_with_new_stack(void *new_stack, void (*handler)(void))
call_with_new_stack:
    movq    %rdi, %rsp
    jmpq   *%rsi

# void co_memcpy(void *dst, void *src, size_t len);
co_memcpy:
    movq    %rdx, %rcx
    rep     movsb (%rsi), (%rdi)
    retq
