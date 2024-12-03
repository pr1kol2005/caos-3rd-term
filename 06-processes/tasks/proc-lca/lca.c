#include "lca.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

const size_t MAX_PATH_LENGTH = 1024;
const size_t MAX_LINE_LENGTH = 1024;
const size_t PPID_LINE_NUMBER_POSITION = 6;

pid_t get_parent_pid(pid_t pid) {
  pid_t parent_id = 0;

  char proc_filename[MAX_PATH_LENGTH];
  sprintf(proc_filename, "/proc/%d/status", pid);

  FILE* process_information = fopen(proc_filename, "r");
  if (process_information != NULL) {
    char line[MAX_LINE_LENGTH];
    while (fgets(line, MAX_LINE_LENGTH, process_information) != NULL) {
      if (strncmp(line, "PPid", 4) == 0) {
        parent_id = (pid_t)atoi(line + PPID_LINE_NUMBER_POSITION);
        break;
      }
    }
  }
  fclose(process_information);

  return parent_id;
}

pid_t find_lca(pid_t x, pid_t y) {
  pid_t x_parents[MAX_TREE_DEPTH];
  pid_t y_parents[MAX_TREE_DEPTH];
  pid_t* x_traversal = &x_parents[0];
  pid_t* y_traversal = &y_parents[0];

  while (x != 0) {
    *x_traversal = x;
    x = get_parent_pid(x);
    ++x_traversal;
  }

  while (y != 0) {
    *y_traversal = y;
    y = get_parent_pid(y);
    ++y_traversal;
  }

  --x_traversal;
  --y_traversal;

  while (x_traversal >= &x_parents[0] && y_traversal >= &y_parents[0] &&
          *x_traversal == *y_traversal) {
    --x_traversal;
    --y_traversal;
  }

  ++x_traversal;

  return *x_traversal;
}
