  .intel_syntax noprefix

  .text
  .global my_memcpy

my_memcpy:
  # rdi - dest
  # rsi - src
  # edx - count

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
  je .end

  lea rcx, [edx]
  xor r8, r8

.copy_byte:
  mov al, [rsi + r8]
  mov [rdi + r8], al
  inc r8
  dec rcx
  jne .copy_byte

.end:
  lea rax, [rdi]
  ret

  ret
