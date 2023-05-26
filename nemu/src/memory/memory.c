#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  int port = is_mmio(addr);
  if (port != -1) {
    return mmio_read(addr, len, port);
  }
  return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int port = is_mmio(addr);
  if (port != -1) {
    mmio_write(addr, len, data, port);
    return;
  }
  memcpy(guest_to_host(addr), &data, len);
}

uint32_t page_translate(uint32_t va, bool is_write) {
  if (!cpu.cr0.paging) {
    return va;
  }

  PDE pde, *pd;
  PTE pte, *pt;

  pd = (PDE *)(cpu.cr3.page_directory_base << PAGE_SHIFT);
  pde.val = paddr_read((uint32_t)&pd[PDX(va)], 4);
  assert(pde.present);
  pde.accessed = 1;
  paddr_write((uint32_t)&pd[PDX(va)], 4, pde.val);

  pt = (PTE *)(pde.page_frame << PAGE_SHIFT);
  pte.val = paddr_read((uint32_t)&pt[PTX(va)], 4);
  assert(pte.present);
  pte.accessed = 1;
  if (is_write) {
    pte.dirty = 1;
  }
  paddr_write((uint32_t)&pt[PTX(va)], 4, pte.val);

  return (pte.page_frame << PAGE_SHIFT) | (va & PAGE_MASK);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  uint32_t data = 0;
  uint8_t *mem = (uint8_t *)&data;

  for (int i = 0; i < len; i++) {
    mem[i] = paddr_read(page_translate(addr + i, false), 1);
  }

  return data;
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  uint8_t *mem = (uint8_t *)&data;

  for (int i = 0; i < len; i++) {
    paddr_write(page_translate(addr + i, true), 1, mem[i]);
  }
}
