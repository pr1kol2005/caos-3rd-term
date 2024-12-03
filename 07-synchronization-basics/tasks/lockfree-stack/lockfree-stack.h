#pragma once

#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node {
  struct node *next;
  uintptr_t value;
} node_t;

typedef struct lfstack {
  _Atomic(node_t *) top;
} lfstack_t;

// _Bool atomic_cas(A* obj, C* expected, C desired) {
//     // Псевдокод!
//     atomic {
//         if (*obj == *expected) {
//             *obj = desired;
//             return true
//         }
//         *expected = *obj;
//         return false;
//     }
// }

int lfstack_init(lfstack_t *stack) {
  stack->top = NULL;
  return EXIT_SUCCESS;
}

int lfstack_push(lfstack_t *stack, uintptr_t value) {
  node_t *new_node;
  if ((new_node = calloc(1, sizeof(node_t))) == NULL) {
    return EXIT_FAILURE;
  }

  new_node->value = value;

  node_t *old_top = stack->top;
  while (true) {
    old_top = stack->top;
    new_node->next = old_top;
    if (atomic_compare_exchange_strong(&stack->top, &old_top, new_node)) {
      break;
    }
  }
  return EXIT_SUCCESS;
}

int lfstack_pop(lfstack_t *stack, uintptr_t *value) {
  node_t *top = stack->top;

  while (top != NULL) {
    if (atomic_compare_exchange_strong(&stack->top, &top, top->next)) {
      break;
    }
  }

  if (top == NULL) {
    *value = 0;
  } else {
    *value = top->value;
    free(top);
  }
  return EXIT_SUCCESS;
}

int lfstack_destroy(lfstack_t *stack) {
  return stack == NULL ? EXIT_SUCCESS : EXIT_FAILURE;
}
