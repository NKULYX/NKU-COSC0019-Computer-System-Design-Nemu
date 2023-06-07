#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  uint32_t cs_wide = cpu.cs;
  uint32_t idt_entry[2] = {0};
  uint32_t routine_addr = 0;

  // 'NO' sanity check
  assert(NO >= 0 && NO * 8 < cpu.idtr.limit);

  idt_entry[0] = vaddr_read(cpu.idtr.base + 8 * NO, 4);
  idt_entry[1] = vaddr_read(cpu.idtr.base + 8 * NO + 4, 4);
  
  // 'present' bit check
  assert(idt_entry[1] & 0x8000);

  // get routine address from idt entry
  routine_addr = (idt_entry[1] & 0xffff0000) | (idt_entry[0] & 0xffff);

  rtl_push(&cpu.eflags.val);
  rtl_push(&cs_wide);
  rtl_push(&ret_addr);

  cpu.eflags.IF = false;
  decoding.jmp_eip = routine_addr;
  decoding.is_jmp = true;
}

void dev_raise_intr() {
  cpu.INTR = true;
}
