#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest,&t0);  // 必须通过operand_write才能写到id_dest的地址中，参见上面的mov

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  t1 = cpu.esp;
  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&t1);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&t1);  //esp不用记，会自己变
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  rtl_pop(&cpu.eax);

  print_asm("popa");
}

make_EHelper(leave) {  // 栈指针指向帧指针，再pop到帧指针
  rtl_mv(&cpu.esp,&cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {  // ax的16位扩展为32位
    rtl_msb(&t0,&cpu.eax,2);
    if(t0 == 1)cpu.edx = cpu.edx | 0xffff;
    else cpu.edx = 0;
  }
  else {  // eax的32位扩展为64位
    rtl_msb(&t0,&cpu.eax,4);
    if(t0 == 1)cpu.edx = cpu.edx | 0xffffffff;
    else cpu.edx = 0;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_sext(&t0, &cpu.eax, 1);
    cpu.eax = (cpu.eax & 0xffff0000) | (t0 & 0xffff);
  }
  else {
    rtl_sext(&cpu.eax, &cpu.eax, 2);
  }
  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}

make_EHelper(mov_store_cr){
  rtl_store_cr(id_dest->reg,&id_src->val); // 数据写入cr寄存器
  print_asm_template2(mov_store_cr);
}
