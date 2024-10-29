#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int kMaxFileSize = 4096;

void tail_f(const char* filename) {
  int fd = open(filename, O_RDONLY);

  if (fd == -1) {
    exit(1);
  }

  char data[kMaxFileSize];
  size_t bytes_read;

  while (1) {
    bytes_read = read(fd, data, kMaxFileSize);

    if (bytes_read != 0) {
      data[bytes_read] = '\0';
      printf("%s", data);
    }

    fflush(stdout);
  }

  close(fd);
}

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    return 1;
  }

  tail_f(argv[1]);

  return 0;
}
