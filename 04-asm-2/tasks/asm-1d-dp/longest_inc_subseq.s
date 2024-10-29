  .text
  .global longest_inc_subseq

longest_inc_subseq:
  // x0 - array
  // x1 - help_array
  // x2 - size
  // x11 - lis length
  mov x11, #0

  cmp x2, #0
  beq end

  // x3 - init_loop counter i
  mov x3, #0

  // register that values 1
  mov x9, #1

init_loop:
  cmp x3, x2
  bge dp_recalculation
  str x9, [x1, x3, lsl #3]  // help_array[i] = 1
  add x3, x3, #1
  b init_loop

dp_recalculation:
  // x3 - outer counter i
  mov x3, #1
    
outer_loop:
  cmp x3, x2
  bge find_lis_len

  // x4 - inner counter j
  mov x4, #0
    
inner_loop:
  cmp x4, x3
  bge increase_i
  
  ldr x5, [x0, x3, lsl #3]  // x5 = array[i]
  ldr x6, [x0, x4, lsl #3]  // x6 = array[j]

  cmp x5, x6
  ble increase_j
  
  ldr x7, [x1, x3, lsl #3]  // x7 = help_array[i]
  ldr x8, [x1, x4, lsl #3]
  add x8, x8, #1  // x8 = help_array[j] + 1

  cmp x7, x8
  csel x7, x7, x8, gt // x7 = max(x7, x8)
  str x7, [x1, x3, lsl #3]  // help_array[i] = x7

increase_j:
  add x4, x4, #1
  b inner_loop

increase_i:
  add x3, x3, #1
  b outer_loop

find_lis_len:
  // i - find_lis_len_loop counter
  mov x3, #0

find_lis_len_loop:
  cmp x3, x2
  bge end
    
  ldr x10, [x1, x3, lsl #3] // x10 = help_array[x3]
  cmp x11, x10
  csel x11, x11, x10, gt  // x11 = max(x11, x10)
  add x3, x3, #1
  b find_lis_len_loop

end:
  mov x0, x11
  ret

  ret
