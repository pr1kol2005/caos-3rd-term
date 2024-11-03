.intel_syntax noprefix

.text
.global dot_product

dot_product:
  # rdi - N
  # rsi - A (pointer)
  # rdx - B (pointer)

.main_loop:
  cmp rdi, 8
  jl .check_remainder

  # обрабатываем 8 координат в цикле (данные не выровнены)
  vmovups ymm1, ymmword ptr [rsi]
  vmovups ymm2, ymmword ptr [rdx]
  vmulps ymm1, ymm1, ymm2
  vaddps ymm0, ymm0, ymm1

  add rsi, 32
  add rdx, 32
  sub rdi, 8
  jmp .main_loop

.check_remainder:
  test rdi, rdi
  je .return

.process_rest:
  # обрабатываем по одной координате в цикле
  movss xmm8, dword ptr [rsi]
  movss xmm9, dword ptr [rdx]
  mulss xmm8, xmm9
  addss xmm0, xmm8

  add rsi, 4
  add rdx, 4
  dec rdi
  jne .process_rest

.return:
  # схлопываем 256 в 128
  vhaddps ymm0, ymm0, ymm0
  vhaddps ymm0, ymm0, ymm0

  # переносим результат в xmm0
  vextractf128 xmm8, ymm0, 1
  addps xmm0, xmm8

  ret
