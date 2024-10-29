#include "utf8_file.h"

int encode_utf8(uint32_t symbol, uint8_t* buffer) {
  if (symbol <= 0x7F) {
    buffer[0] = symbol;
    return 1;
  } else if (symbol <= 0x7FF) {
    buffer[0] = 0xC0 | (symbol >> 6);
    buffer[1] = 0x80 | (symbol & 0x3F);
    return 2;
  } else if (symbol <= 0xFFFF) {
    buffer[0] = 0xE0 | (symbol >> 12);
    buffer[1] = 0x80 | ((symbol >> 6) & 0x3F);
    buffer[2] = 0x80 | (symbol & 0x3F);
    return 3;
  } else if (symbol <= 0x10FFFF) {
    buffer[0] = 0xF0 | (symbol >> 18);
    buffer[1] = 0x80 | ((symbol >> 12) & 0x3F);
    buffer[2] = 0x80 | ((symbol >> 6) & 0x3F);
    buffer[3] = 0x80 | (symbol & 0x3F);
    return 4;
  } else if (symbol <= 0x7FFFFFFF) {
    buffer[0] = 0xF8 | (symbol >> 24);
    buffer[1] = 0x80 | ((symbol >> 18) & 0x3F);
    buffer[2] = 0x80 | ((symbol >> 12) & 0x3F);
    buffer[3] = 0x80 | ((symbol >> 6) & 0x3F);
    buffer[4] = 0x80 | (symbol & 0x3F);
    return 5;
  } else if (symbol <= 0xFFFFFFFF) {
    buffer[0] = 0xFC | (symbol >> 30);
    buffer[1] = 0x80 | ((symbol >> 24) & 0x3F);
    buffer[2] = 0x80 | ((symbol >> 18) & 0x3F);
    buffer[3] = 0x80 | ((symbol >> 12) & 0x3F);
    buffer[4] = 0x80 | ((symbol >> 6) & 0x3F);
    buffer[5] = 0x80 | (symbol & 0x3F);
    return 6;
  } else {
    return -1;
  }
}

int utf8_write(utf8_file_t* f, const uint32_t* str, size_t count) {
  uint8_t buffer[6];
  size_t written_count = 0;

  for (size_t i = 0; i < count; i++) {
    uint32_t symbol = str[i];
    int octet_count = encode_utf8(symbol, buffer);

    if (octet_count == -1) {
      errno = EILSEQ;
      return -1;
    }

    int bytes_written = write(f->fd, buffer, octet_count);

    if (bytes_written == -1) {
      return -1;
    }

    written_count++;
  }

  return written_count;
}

int decode_utf8(uint32_t* symbol, uint8_t* buffer) {
  if (buffer[0] <= 0x7F) {
    *symbol = buffer[0];
    return 1;
  } else if ((buffer[0] & 0xE0) == 0xC0) {
    *symbol = (buffer[0] & 0x1F) << 6 | (buffer[1] & 0x3F);
    return 2;
  } else if ((buffer[0] & 0xF0) == 0xE0) {
    *symbol =
        (buffer[0] & 0x0F) << 12 | (buffer[1] & 0x3F) << 6 | (buffer[2] & 0x3F);
    return 3;
  } else if ((buffer[0] & 0xF8) == 0xF0) {
    *symbol = (buffer[0] & 0x07) << 18 | (buffer[1] & 0x3F) << 12 |
              (buffer[2] & 0x3F) << 6 | (buffer[3] & 0x3F);
    return 4;
  } else if ((buffer[0] & 0xFC) == 0xF8) {
    *symbol = (buffer[0] & 0x03) << 24 | (buffer[1] & 0x3F) << 18 |
              (buffer[2] & 0x3F) << 12 | (buffer[3] & 0x3F) << 6 |
              (buffer[4] & 0x3F);
    return 5;
  } else if ((buffer[0] & 0xFE) == 0xFC) {
    *symbol = (buffer[0] & 0x01) << 30 | (buffer[1] & 0x3F) << 24 |
              (buffer[2] & 0x3F) << 18 | (buffer[3] & 0x3F) << 12 |
              (buffer[4] & 0x3F) << 6 | (buffer[5] & 0x3F);
    return 6;
  } else {
    return -1;
  }
}

int utf8_read(utf8_file_t* f, uint32_t* res, size_t count) {
  uint8_t buffer[6];
  size_t read_count = 0;

  while (read_count < count) {
    int bytes_read = read(f->fd, buffer, 1);
    if (bytes_read == 0) {
      break;
    } else if (bytes_read < 0) {
      return -1;
    }

    size_t len = 1;
    while ((buffer[0] & (0x80 >> len)) && len < 6) {
      bytes_read = read(f->fd, buffer + len, 1);
      if (bytes_read < 0) {
        return -1;
      }
      len++;
    }

    uint32_t symbol;
    int octet_count = decode_utf8(&symbol, buffer);
    if (octet_count == -1) {
      errno = EILSEQ;
      return -1;
    }

    res[read_count] = symbol;
    read_count++;
  }

  return read_count;
}

utf8_file_t* utf8_fromfd(int fd) {
  if (fd < 0) {
    errno = EINVAL;
    return NULL;
  }

  utf8_file_t* file = (utf8_file_t*)malloc(sizeof(utf8_file_t));

  file->fd = fd;
  return file;
}
