#include "common.h"
#include "syscall.h"

extern void _halt(int);

static inline _RegSet* sys_none(_RegSet *r){
  SYSCALL_ARG1(r) = 1;
  return NULL;
}

static inline _RegSet* sys_exit(_RegSet *r){
  _halt(SYSCALL_ARG2(r)); 
  return NULL;
}

static inline _RegSet* sys_write(_RegSet *r){
  int fd = (int)SYSCALL_ARG2(r);
  char *buf = (char *)SYSCALL_ARG3(r);
  size_t count = (int)SYSCALL_ARG4(r);
  uintptr_t i = 0;
  if(fd == 1 || fd == 2) {
    printf("!");
    for(;i < count; i++) {
      _putc(buf[i]);
    }
    SYSCALL_ARG1(r) = count;
    return NULL;
  }
  SYSCALL_ARG1(r) = -1;
  return NULL;
}

_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
    case SYS_none:return sys_none(r);
    case SYS_exit:return sys_exit(r);
    case SYS_write:return sys_write(r);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
