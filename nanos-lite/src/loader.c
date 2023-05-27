#include "common.h"

#define min(a, b) ((a)<(b)?(a):(b))

void ramdisk_read(void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

size_t fs_filesz(int fd);
int fs_open(const char *pathname, int flags, int mod);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

void* new_page(void);
void _map(_Protect *p, void *va, void *pa);

#define DEFAULT_ENTRY ((void *)0x8048000)


uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0);

  int size = fs_filesz(fd);

  void *pa = NULL;
  void *va = DEFAULT_ENTRY;
  while (size >= 0) {
    pa = new_page();
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);

    va += PGSIZE;
    size -= PGSIZE;
  }
  fs_close(fd);

  return (uintptr_t)DEFAULT_ENTRY;
}
