#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();
extern void raise_intr(uint8_t NO,vaddr_t ret_addr);

make_EHelper(lidt) {  
  rtl_li(&t0, id_dest->addr);
  cpu.idtr.limit = vaddr_read(t0, 2);  // ��ȡIDT����  
  cpu.idtr.base = vaddr_read(t0+2, 4);  // ��ȡIDT�׵�ַ
  if(decoding.is_operand_size_16)
    cpu.idtr.base &= 0x00ffffff;

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
  raise_intr(id_dest->val,decoding.seq_eip);

  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {  // �ָ�eip��cs��eflags
  rtl_pop(&decoding.jmp_eip);
  rtl_pop(&cpu.cs);
  rtl_pop(&t1);
  memcpy(&cpu.eflags,&t1,sizeof(cpu.eflags));

  decoding.is_jmp = 1;
  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  t1 = pio_read(id_src->val,id_dest->width);
  operand_write(id_dest,&t1);

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
