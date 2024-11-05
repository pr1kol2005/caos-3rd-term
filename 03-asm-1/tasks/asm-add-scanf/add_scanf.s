.intel_syntax noprefix

.text
.global add_scanf

add_scanf:
  push rbp
  mov rbp, rsp

  sub rsp, 16

  mov rdi, [scanf_format_string]
  mov rsi, [rsp]
  mov rdx, [rsp + 8]

  call scanf

  mov rax, [rsp]
  add rax, [rsp + 8]

  mov rsp, rbp
  pop rbp
  ret

  .section .rodata

scanf_format_string:
  .string "%lld %lld"
