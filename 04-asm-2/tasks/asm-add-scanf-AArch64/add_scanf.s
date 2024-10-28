  .text
  .global add_scanf

  .macro push Xn
    sub sp, sp, 16
    str \Xn, [sp]
  .endm

  .macro pop Xn
    ldr \Xn, [sp]
    add sp, sp, 16
  .endm

add_scanf:
  stp x29, x30, [sp, -16]!

  sub sp, sp, 16
  mov x1, sp
  add x2, sp, 8
  ldr x0, =scanf_format_string

  bl scanf

  ldr x0, [sp]
  ldr x1, [sp, 8]
  add x0, x0, x1

  add sp, sp, 16
  ldp x29, x30, [sp], 16
  ret

  ret

.section .rodata, "a"

scanf_format_string:
  .string "%lld %lld"
