#include "common.h"

void ramdisk_read(void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

size_t fs_filesz(int fd);
int fs_open(const char *pathname, int flags, int mod);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

#define DEFAULT_ENTRY ((void *)0x4000000)

// PA3.1 impl

// uintptr_t loader(_Protect *as, const char *filename) {
//   ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
//   return (uintptr_t)DEFAULT_ENTRY;
// }

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0);

  int size = fs_filesz(fd);

  ssize_t read = fs_read(fd, DEFAULT_ENTRY, size);
  assert(read == size);

  return (uintptr_t)DEFAULT_ENTRY;
}
