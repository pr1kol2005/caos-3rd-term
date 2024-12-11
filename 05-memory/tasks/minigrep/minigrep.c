#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <pcre.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

enum { OVECSIZE = 3 };

void search_pattern(pcre* pattern, char* path);

void process_directory(pcre* pattern, char* path) {
  struct dirent* entry;
  DIR* dir;

  if ((dir = opendir(path)) == NULL) {
    exit(EXIT_FAILURE);
  }

  while ((entry = readdir(dir)) != NULL) {
    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      search_pattern(pattern, fullpath);
    }
  }

  if (closedir(dir) == -1) {
    exit(EXIT_FAILURE);
  }
}

void process_file_line(pcre* pattern, char* path, char* file_mapping,
                       size_t index, size_t line_start, size_t line_number) {
  char* current_line = strndup(file_mapping + line_start, index - line_start);

  int ovector[OVECSIZE];
  memset(ovector, 0, OVECSIZE * sizeof(int));

  int search_result = pcre_exec(pattern, NULL, current_line, index - line_start,
                                0, 0, ovector, OVECSIZE);

  if (search_result > 0) {
    printf("%s:%zu: %s\n", path, line_number, current_line);
  }

  free(current_line);
}

void process_file(pcre* pattern, char* path) {
  struct stat file_stat;
  char* file_mapping;
  int fd = open(path, O_RDONLY);
  size_t line_start = 0;
  size_t line_number = 1;

  if (fstat(fd, &file_stat) == -1) {
    exit(EXIT_FAILURE);
  }

  if ((file_mapping = mmap(NULL, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd,
                           0)) == MAP_FAILED) {
    close(fd);
    exit(EXIT_FAILURE);
  }

  for (size_t index = 0; index < file_stat.st_size; ++index) {
    if (file_mapping[index] == '\n') {
      process_file_line(pattern, path, file_mapping, index, line_start,
                        line_number);
      line_start = index + 1;
      ++line_number;
    }
  }

  process_file_line(pattern, path, file_mapping, file_stat.st_size, line_start,
                    line_number);

  munmap(file_mapping, file_stat.st_size);
}

void search_pattern(pcre* pattern, char* path) {
  struct stat path_stat;

  if (lstat(path, &path_stat) == -1) {
    exit(EXIT_FAILURE);
  }

  if (S_ISDIR(path_stat.st_mode)) {
    process_directory(pattern, path);
  } else {
    process_file(pattern, path);
  }
}

pcre* compile_pattern(const char* pattern) {
  const char* err_ptr;
  int err_offset;
  pcre* compiled_pattern =
      pcre_compile(pattern, 0, &err_ptr, &err_offset, NULL);

  if (compiled_pattern == NULL) {
    fprintf(stderr, "error: %s\n", err_ptr);
    exit(EXIT_FAILURE);
  }

  return compiled_pattern;
}

int main(int argc, char* argv[]) {
  const char* regex = argv[1];
  char* dir = argv[2];

  pcre* pattern = compile_pattern(regex);

  search_pattern(pattern, dir);

  pcre_free(pattern);

  return EXIT_SUCCESS;
}
