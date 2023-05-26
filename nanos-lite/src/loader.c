#include "common.h"

void ramdisk_read(void *buf, off_t offset, size_t len);
size_t get_ramdisk_size();

size_t fs_filesz(int fd);
int fs_open(const char *pathname, int flags, int mod);
ssize_t fs_read(int fd, void *buf, size_t len);
ssize_t fs_write(int fd, const void *buf, size_t len);
off_t fs_lseek(int fd, off_t offset, int whence);
int fs_close(int fd);

#define DEFAULT_ENTRY ((void *)0x8048000)

extern void _map(_Protect *p, void *va, void *pa);
extern void* new_page(void);

// PA3.1 impl

// uintptr_t loader(_Protect *as, const char *filename) {
//   ramdisk_read(DEFAULT_ENTRY, 0, get_ramdisk_size());
//   return (uintptr_t)DEFAULT_ENTRY;
// }

uintptr_t loader(_Protect *as, const char *filename) {
  int fd = fs_open(filename, 0, 0);
  int bytes = fs_filesz(fd); //出错在之前为size_t

  Log("Load [%d] %s with size: %d", fd, filename, bytes);

  void *pa,*va = DEFAULT_ENTRY;
  while(bytes>0){
  	pa = new_page(); //申请空闲物理页
  	_map(as, va, pa);//该物理页映射到用户程序虚拟地址空间
  	fs_read(fd, pa, PGSIZE);  //读一页文件到该物理页

  	va += PGSIZE;
  	bytes -= PGSIZE;
  }
  //fs_read(fd,DEFAULT_ENTRY,bytes);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
