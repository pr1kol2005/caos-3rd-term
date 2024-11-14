#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int RecursiveRemove(const char *path) {
  struct dirent *entry;
  DIR *dir = opendir(path);

  if (!dir) {
    return 1;
  }

  while ((entry = readdir(dir)) != NULL) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    struct stat path_stat;
    if (lstat(fullpath, &path_stat) == -1) {
      return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
      if (RecursiveRemove(fullpath) == 1) {
        return 1;
      };
      if (rmdir(fullpath) == -1) {
        return 1;
      }
    } else {
      if (remove(fullpath) == -1) {
        return 1;
      }
    }
  }
  closedir(dir);
  return 0;
}

int main(int argc, const char *argv[]) {
  int recursive = 0;
  int start = 1;

  if (argc < 2) {
    return 1;
  }

  if (strcmp(argv[1], "-r") == 0) {
    recursive = 1;
    start = 2;
  }

  for (int i = start; i < argc; i++) {
    struct stat path_stat;
    if (lstat(argv[i], &path_stat) == -1) {
      return 1;
    }

    if (S_ISDIR(path_stat.st_mode)) {
      if (recursive) {
        if (RecursiveRemove(argv[i]) == 1) {
          return 1;
        };
        if (rmdir(argv[i]) == -1) {
          return 1;
        }
      } else {
        return 1;
      }
    } else {
      if (remove(argv[i]) == -1) {
        return 1;
      }
    }
  }

  return 0;
}
