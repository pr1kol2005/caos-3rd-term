#include "falloc.h"

#include <errno.h>
#include <fcntl.h>
#include <memory.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

uint64_t GetPageBit(file_allocator_t* allocator, size_t index) {
  return (allocator->page_mask[index >> 6] & (1ull << (index & 0x3F)));
}

void SetPageBit(file_allocator_t* allocator, size_t index, bool value) {
  if (value) {
    allocator->page_mask[index >> 6] |= (1ull << (index & 0x3F));
    ++(allocator->curr_page_count);
  } else {
    allocator->page_mask[index >> 6] &= ~(1ull << (index & 0x3F));
    --(allocator->curr_page_count);
  }
}

void falloc_init(file_allocator_t* allocator, const char* filepath,
                 uint64_t allowed_page_count) {
  int fd = 0;
  bool file_already_existed = true;
  void* base_addr = NULL;

  if ((fd = open(filepath, O_RDWR)) == -1) {
    if (errno == ENOENT) {
      file_already_existed = false;
      fd = open(filepath, O_RDWR | O_CREAT, S_IRWXU);
    } else {
      perror("illegal filepath");
      return;
    }
  }

  if (fd == -1) {
    perror("failed to open file");
    return;
  }

  if (ftruncate(fd, allowed_page_count * PAGE_SIZE + PAGE_MASK_SIZE_IN_BYTES) ==
      -1) {
    perror("failed to truncate file");
    close(fd);
    return;
  }

  if ((base_addr = mmap(NULL, allowed_page_count * PAGE_SIZE + PAGE_MASK_SIZE,
                        PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) ==
      MAP_FAILED) {
    perror("failed to map file");
    close(fd);
    return;
  }

  allocator->fd = fd;
  allocator->page_mask = base_addr;
  allocator->base_addr = base_addr + PAGE_MASK_SIZE;
  allocator->curr_page_count = 0;
  allocator->allowed_page_count = allowed_page_count;

  if (!file_already_existed) {
    memset(allocator->page_mask, 0, PAGE_MASK_SIZE);
  } else {
    for (size_t index = 0; index < allocator->allowed_page_count; ++index) {
      if (GetPageBit(allocator, index) != 0) {
        ++(allocator->curr_page_count);
      }
    }
  }
}

void falloc_destroy(file_allocator_t* allocator) {
  msync(allocator->base_addr,
        allocator->allowed_page_count * PAGE_SIZE + PAGE_MASK_SIZE, MS_SYNC);

  munmap(allocator->base_addr,
         allocator->allowed_page_count * PAGE_SIZE + PAGE_MASK_SIZE);

  close(allocator->fd);

  allocator->base_addr = NULL;
  allocator->page_mask = NULL;
  allocator->curr_page_count = 0;
}

void* falloc_acquire_page(file_allocator_t* allocator) {
  for (size_t index = 0; index < allocator->allowed_page_count; ++index) {
    if (GetPageBit(allocator, index) == 0) {
      SetPageBit(allocator, index, true);
      return allocator->base_addr + (index * PAGE_SIZE);
    }
  }

  return NULL;
}

void falloc_release_page(file_allocator_t* allocator, void** addr) {
  if (addr == NULL) {
    return;
  }

  size_t index = (*addr - allocator->base_addr) / PAGE_SIZE;

  SetPageBit(allocator, index, false);

  *addr = NULL;
}
