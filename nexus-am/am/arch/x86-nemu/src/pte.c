#include <x86.h>

#define PG_ALIGN __attribute((aligned(PGSIZE)))

static PDE kpdirs[NR_PDE] PG_ALIGN;
static PTE kptabs[PMEM_SIZE / PGSIZE] PG_ALIGN;
static void* (*palloc_f)();
static void (*pfree_f)(void*);

extern void *memcpy(void *,const void*,int);

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

// 虚拟空间映射到物理空间
void _map(_Protect *p, void *va, void *pa) {
  PDE *pgdir=(PDE*)p->ptr; // 页目录表基址
  PTE *pgtab=NULL; // 页表基址

  PDE *pde = pde=pgdir+PDX(va); 
  if(!(*pde&PTE_P)){ // 没有对应页表
    pgtab=(PTE*)(palloc_f()); // 进行申请
    *pde=(uintptr_t)pgtab|PTE_P; // 进行映射
  } 
  
  pgtab=(PTE*)PTE_ADDR(*pde);
  PTE *pte=pgtab+PTX(va);
  *pte=(uintptr_t)pa|PTE_P; // 进行映射
}

void _unmap(_Protect *p, void *va) {
}

// 参数入栈->初始化陷阱帧->返回陷阱帧指针
_RegSet *_umake(_Protect *p, _Area ustack, _Area kstack, void *entry, char *const argv[], char *const envp[]) {
  // 参数入栈，实际上并不会用到这些参数，设为0或NULL
  int arg1=0;
  char *arg2=NULL;
  memcpy((void*)ustack.end-4,(void*)arg2,4);
  memcpy((void*)ustack.end-8,(void*)arg2,4);
  memcpy((void*)ustack.end-12,(void*)arg1,4);
  memcpy((void*)ustack.end-16,(void*)arg1,4);
  
  // 初始化陷阱帧
  _RegSet tf;
  tf.cs=8;
  tf.eflags=0x202;
  tf.eip=(uintptr_t)entry; // 返回地址
  void *tf_addr=(void*)(ustack.end-16-sizeof(tf));
  memcpy(tf_addr,(void*)&tf,sizeof(tf));
  return (_RegSet*)tf_addr;
}
