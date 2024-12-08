#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

enum { FILE_SIZE = 4096 };

static int signal_fd_;

void signalfd_action(int sig, siginfo_t* unused_1, void* unused_2) {
  size_t file_offset = lseek(signal_fd_, 0, SEEK_CUR);

  if (file_offset == -1) {
    return;
  }

  lseek(signal_fd_, 0, SEEK_END);

  if (write(signal_fd_, &sig, sizeof(int)) == -1) {
    perror("write");
    return;
  };

  lseek(signal_fd_, file_offset, SEEK_SET);
}

int signalfd() {
  signal_fd_ = open("userspace_signalfd_file", O_RDWR | O_CREAT, S_IRWXU);

  if (signal_fd_ == -1) {
    perror("create");
    return -1;
  }

  unlink("userspace_signalfd_file");

  mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, signal_fd_, 0);

  struct sigaction sigaction_config;

  sigaction_config.sa_flags = SA_SIGINFO;
  sigaction_config.sa_sigaction = signalfd_action;
  sigfillset(&sigaction_config.sa_mask);

  for (int sig = 1; sig < NSIG; ++sig) {
    if (sig != SIGKILL && sig != SIGSTOP) {
      sigaction(sig, &sigaction_config, NULL);
      if (errno != 0) {
        perror("sigaction");
        return -1;
      }
    }
  }

  return signal_fd_;
}
