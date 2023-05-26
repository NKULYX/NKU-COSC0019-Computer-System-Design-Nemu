#include "nemu.h"
#include "memory/mmu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int p = is_mmio(addr);
  if(p == -1) {
    return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
  }
  else {
    return mmio_read(addr, len, p);
  }
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int p = is_mmio(addr);
  if(p == -1) {
    memcpy(guest_to_host(addr), &data, len);
  }
  else {
    mmio_write(addr, len, data, p);
  }
}

#define PDX(va)     (((uint32_t)(va) >> 22) & 0x3ff)
#define PTX(va)     (((uint32_t)(va) >> 12) & 0x3ff)
#define OFF(va)     ((uint32_t)(va) & 0xfff)
#define PTE_ADDR(pte)   ((uint32_t)(pte) & ~0xfff)


paddr_t page_translate(vaddr_t addr, bool is_write) {
    CR0 cr0 = (CR0) cpu.CR0;
    CR3 cr3 = (CR3) cpu.CR3;
    PDE pde, *pgdir;
    PTE pte, *pgtab;
    if (cr0.protect_enable && cr0.paging) {
	    pgdir = (PDE *)(PTE_ADDR(cr3.val));
      uint32_t pde_off = PDX(addr);
	    pde.val = paddr_read((paddr_t)&pgdir[pde_off], 4);
	    assert(pde.present);
	    pde.accessed = 1;

	    pgtab = (PTE *)(PTE_ADDR(pde.val));
      uint32_t pte_off = PTX(addr);
	    pte.val = paddr_read((paddr_t)&pgtab[pte_off], 4);
	    assert(pte.present);
	    pte.accessed = 1;

	    pte.dirty = is_write ? 1 : pte.dirty;

      uint32_t page = PTE_ADDR(pte.val);
      uint32_t page_off = OFF(addr);

      Log("va 0x%08x -> pa 0x%08x\n", addr, page | page_off);
	    return page | page_off; 
	}

  return addr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  uint32_t data = 0;
  uint8_t *mem = (uint8_t*) &data;
  for(int i = 0; i < len; i++) {
    paddr_t paddr = page_translate(addr + i, false);
    mem[i] = paddr_read(paddr, 1);
  }
  return data;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  uint8_t *mem = (uint8_t*) &data;
  for(int i = 0; i < len; i++) {
    paddr_t paddr = page_translate(addr + i, true);
    paddr_write(paddr, 1, mem[i]);
  }
}

// uint32_t vaddr_read(vaddr_t addr, int len) {
//   return paddr_read(addr, len);
// }

// void vaddr_write(vaddr_t addr, int len, uint32_t data) {
//   paddr_write(addr, len, data);
// }
