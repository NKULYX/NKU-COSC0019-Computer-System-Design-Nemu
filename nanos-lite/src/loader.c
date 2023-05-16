#include "common.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern void ramdisk_read(void*, off_t, size_t);
extern size_t get_ramdisk_size();

extern void _map(_Protect *p, void *va, void *pa);
extern void* new_page(void);

extern int fs_open(const char *pathname, int flags, int mode);
extern size_t fs_filesz(int fd);
extern ssize_t fs_read(int fd, void *buf, size_t len);
extern int fs_close(int fd);

uintptr_t loader(_Protect *as, const char *filename) {
  // size_t len = get_ramdisk_size();
  // ramdisk_read(DEFAULT_ENTRY, 0, len);
  // return (uintptr_t)DEFAULT_ENTRY;
  int fd = fs_open(filename, 0, 0);
  int file_size = fs_filesz(fd);
  // Log("Load [%d] %s with size: %d", fd, filename, file_size);
  fs_read(fd, DEFAULT_ENTRY, file_size);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
