#include "common.h"

#define DEFAULT_ENTRY ((void *)0x8048000)
extern size_t get_ramdisk_size();
extern void ramdisk_read(void *, off_t, size_t);
extern void* new_page(void);

int fs_open(const char* path, int flags, int mode);
size_t fs_filesz(int fd);
ssize_t fs_read(int fd, void* buf, size_t len);
int fs_close(int fd);

// 打开文件->读到内存->关闭文件
uintptr_t loader(_Protect *as, const char *filename) {
  int fd=fs_open(filename,0,0);
  Log("filename=%s,fd=%d",filename,fd);
  int f_size = fs_filesz(fd);
  fs_close(fd);
  void* pa = DEFAULT_ENTRY;
  void* va = DEFAULT_ENTRY;
  while(f_size > 0){  // 不断地申请物理页，建立映射，读物理页
    pa = new_page();
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);
    va += PGSIZE;
    f_size -= PGSIZE;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
