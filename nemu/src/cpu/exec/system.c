#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

extern void raise_intr(uint8_t, vaddr_t);

make_EHelper(lidt) {
  rtl_li(&t0,id_dest->addr);
  rtl_li(&t1,vaddr_read(t0,2));
  cpu.idtr.limit = t1;
  rtl_li(&t1,vaddr_read(t0+2,4));
  cpu.idtr.base = t1;
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  uint8_t NO = id_dest->val & 0xff;
  raise_intr(NO, decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  TODO();

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t2 = pio_read(id_src->val,id_dest->width);
  operand_write(id_dest,&t2);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  pio_write(id_dest->val,id_dest->width,id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
