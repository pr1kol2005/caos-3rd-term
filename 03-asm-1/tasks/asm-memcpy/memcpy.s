  .intel_syntax noprefix

  .text
  .global my_memcpy

my_memcpy:
  # rdi - dest
  # rsi - src
  # edx - count

  test edx, edx
  je .return

.loop:
  cmp edx, 8
  jl .copy_rest

  mov rax, [rsi]
  mov [rdi], rax

  add rsi, 8
  add rdi, 8
  sub edx, 8
  jmp .loop

.copy_rest:
  test edx, edx
  je .return

  lea rbp, [edx]
  xor rbx, rbx

.copy_byte:
  mov al, [rsi + rbx]
  mov [rdi + rbx], al
  inc rbx
  dec rbp
  jne .copy_byte

.return:
  lea rax, [rdi]
  ret

  ret
