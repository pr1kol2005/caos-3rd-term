#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

const mode_t RWE = 0777;

void CreateParents(const char *path, mode_t mode) {
  char temp[PATH_MAX];
  snprintf(temp, sizeof(temp), "%s", path);
  temp[sizeof(temp) - 1] = '\0';

  char *parent = temp;
  while ((parent = strchr(parent, '/')) != NULL) {
    *parent = '\0';

    if (strlen(temp) > 0) {
      if (mkdir(temp, RWE) == -1 && errno != EEXIST) {
        exit(1);
      }
    }
    *parent = '/';
    parent++;
  }

  if (mkdir(temp, mode) == -1 && errno != EEXIST) {
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  int is_there_p = 0;
  mode_t mode = RWE;
  int current_option = 0;

  struct option long_options[] = {{"mode", required_argument, 0, 'm'}};

  while ((current_option =
              getopt_long(argc, argv, "pm:", long_options, NULL)) != -1) {
    if (current_option == 'p') {
      is_there_p = 1;
    } else if (current_option == 'm') {
      mode = strtol(optarg, NULL, 8);
      if (mode == 0 && strcmp(optarg, "0") != 0) {
        return 1;
      }
    } else {
      return 1;
    }
  }

  if (optind >= argc) {
    return 1;
  }

  for (int i = optind; i < argc; i++) {
    if (is_there_p) {
      CreateParents(argv[i], mode);
    } else {
      if (mkdir(argv[i], mode) == -1) {
        return 1;
      }
    }
  }

  return 0;
}
