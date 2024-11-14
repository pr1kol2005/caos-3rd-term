#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>

bool is_same_file(const char* lhs_path, const char* rhs_path) {
  struct stat lhs_stat;
  struct stat rhs_stat;

  if (stat(lhs_path, &lhs_stat) == -1 || stat(rhs_path, &rhs_stat) == -1) {
    return false;
  }

  return(lhs_stat.st_ino == rhs_stat.st_ino) && (lhs_stat.st_dev == rhs_stat.st_dev);
}

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    return 1;
  }

  if (is_same_file(argv[1], argv[2])) {
    printf("yes\n");
  } else {
    printf("no\n");
  }

  return 0;
}
