#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

_Area segments[] = {      // Kernel memory mappings
  {.start = (void*)0,          .end = (void*)PMEM_SIZE}
};

#define NR_KSEG_MAP (sizeof(segments) / sizeof(segments[0]))

void _pte_init(void* (*palloc)(), void (*pfree)(void*)) {
  palloc_f = palloc;
  pfree_f = pfree;

  int i;

  // make all PDEs invalid
  for (i = 0; i < NR_PDE; i ++) {
    kpdirs[i] = 0;
  }

  PTE *ptab = kptabs;
  for (i = 0; i < NR_KSEG_MAP; i ++) {
    uint32_t pdir_idx = (uintptr_t)segments[i].start / (PGSIZE * NR_PTE);
    uint32_t pdir_idx_end = (uintptr_t)segments[i].end / (PGSIZE * NR_PTE);
    for (; pdir_idx < pdir_idx_end; pdir_idx ++) {
      // fill PDE
      kpdirs[pdir_idx] = (uintptr_t)ptab | PTE_P;

      // fill PTE
      PTE pte = PGADDR(pdir_idx, 0, 0) | PTE_P;
      PTE pte_end = PGADDR(pdir_idx + 1, 0, 0) | PTE_P;
      for (; pte < pte_end; pte += PGSIZE) {
        *ptab = pte;
        ptab ++;
      }
    }
  }

  set_cr3(kpdirs);
  set_cr0(get_cr0() | CR0_PG);
}

void _protect(_Protect *p) {
  PDE *updir = (PDE*)(palloc_f());
  p->ptr = updir;
  // map kernel space
  for (int i = 0; i < NR_PDE; i ++) {
    updir[i] = kpdirs[i];
  }

  p->area.start = (void*)0x8000000;
  p->area.end = (void*)0xc0000000;
}

void _release(_Protect *p) {
}

void _switch(_Protect *p) {
  set_cr3(p->ptr);
}

void print(const char *s) {
  for (; *s; s ++) {
    _putc(*s);
  }
}

void printd(uint32_t v) {
  if (v == 0) {
    return;
  }
  printd(v >> 8);
  char map[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
  _putc(map[(v>>4)&0xf]);
  _putc(map[v&0xf]);
}

void _map(_Protect *p, void *va, void *pa) {
  PDE *ppde = (PDE *)p->ptr + PDX(va);
  if (!(*ppde & PTE_P)) {
    *ppde = (uint32_t)palloc_f() | PTE_P;
  }

  PTE *ppte = (PTE *)PTE_ADDR(*ppde) + PTX(va);
  *ppte = PTE_ADDR(pa) | PTE_P;
}

void _unmap(_Protect *p, void *va) {
}

#define push(v) *(--ptr)=(v)

_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  uint32_t *ptr = ustack.end;

  for (int i = 0; i < 8; i++) {
    push(0);
  }

  push(0x202); 
  push(0x8);
  push((uint32_t) entry);
  push(0);
  push(0x81);
  for (int i = 0; i < 8; i++) {
    push(0);
  }

  return (_RegSet *)ptr;
}
