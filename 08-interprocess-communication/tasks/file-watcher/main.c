#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct Counter {
  char filename[PATH_MAX];
  uint64_t counter;
  struct Counter* next;
} Counter;

typedef struct Counters {
  struct Counter* head;
} Counters;

void increment(Counters* counters, char* filename, int value) {
  Counter* current = counters->head;
  while (current != NULL) {
    if (strncmp(current->filename, filename, PATH_MAX) == 0) {
      current->counter += value;
      return;
    }
    current = current->next;
  }
  Counter* new_head = malloc(sizeof(Counter));
  new_head->next = counters->head;
  new_head->counter = value;
  strncpy(new_head->filename, filename, PATH_MAX - 1);
  counters->head = new_head;
}

void print(Counters* counters) {
  Counter* current = counters->head;
  while (current != NULL) {
    printf("%s:%ld\n", current->filename, current->counter);
    current = current->next;
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    return EXIT_FAILURE;
  }

  Counters* counters = malloc(sizeof(Counter));
  counters->head = NULL;
  int child_pid;
  int child_status;

  // creating child process
  if ((child_pid = fork()) == 0) {
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
      return EXIT_FAILURE;
    }

    if (execvp(argv[1], argv + 1) == -1) {
      return EXIT_FAILURE;
    }
  } else if (child_pid == -1) {
    return EXIT_FAILURE;
  }

  waitpid(child_pid, &child_status, 0);
  ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD);

  while (!WIFEXITED(child_status)) {
    ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
    waitpid(child_pid, &child_status, 0);

    // stopped with system call trap
    if (WIFSTOPPED(child_status) &&
        WSTOPSIG(child_status) == (SIGTRAP | 0x80)) {
      struct user_regs_struct registers;
      ptrace(PTRACE_GETREGS, child_pid, 0, &registers);

      // if write had been called
      if (registers.orig_rax == SYS_write) {
        char descriptor_link[PATH_MAX];
        char file_path[PATH_MAX];
        size_t file_path_size;

        sprintf(descriptor_link, "/proc/%u/fd/%llu", child_pid,
                 registers.rdi);

        if ((file_path_size = readlink(descriptor_link, file_path, PATH_MAX)) == -1) {
          return EXIT_FAILURE;
        }

        increment(counters, file_path, registers.rdx);
      }

      ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
      waitpid(child_pid, &child_status, 0);
    }
  }

  print(counters);

  return EXIT_SUCCESS;
}
