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
  int bytes = fs_filesz(fd); //出错在之前为size_t

  Log("Load [%d] %s with size: %d", fd, filename, bytes);

  void *pa,*va = DEFAULT_ENTRY;
  while(bytes>0){
  	pa = new_page(); //申请空闲物理页
    Log("Here!");
  	_map(as, va, pa);//该物理页映射到用户程序虚拟地址空间
    Log("Here!!");
  	fs_read(fd, pa, PGSIZE);  //读一页文件到该物理页
    Log("Here!!!");

  	va += PGSIZE;
  	bytes -= PGSIZE;
  }
  //fs_read(fd,DEFAULT_ENTRY,bytes);
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
