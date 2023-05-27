#include "proc.h"
#include "memory.h"

#define _ROUND_MASK(x, y) ((__typeof__(x))((y)-1))
#define ROUND_UP(x, y) ((((x)-1) | _ROUND_MASK(x, y))+1)
#define ROUND_DOWN(x, y) ((x) & ~_ROUND_MASK(x, y))

static void *pf = NULL;

void* new_page(void) {
  assert(pf < (void *)_heap.end);
  void *p = pf;
  pf += PGSIZE;
  return p;
}

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uint32_t new_brk) {
  if (current->cur_brk == 0) {
    current->cur_brk = current->max_brk = new_brk;
  }
  else {
    if (new_brk > current->max_brk) {
      uint32_t va = ROUND_UP(current->max_brk, PGSIZE);
      while (va <= new_brk) {
        _map(&current->as, (void *)va, new_page());
        va += PGSIZE;
      }
      current->max_brk = new_brk;
    }
    current->cur_brk = new_brk;
  }
  return 0;
}

void init_mm() {
  pf = (void *)PGROUNDUP((uintptr_t)_heap.start);
  Log("free physical pages starting from %p", pf);

  _pte_init(new_page, free_page);
}
