#include "cpu/exec.h"

make_EHelper(test) {  // ִ�а�λ�룬������־�Ĵ�����λ����д�룩
  rtl_and(&t0,&id_dest->val,&id_src->val);
  t1=0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);
  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template2(test);
}

make_EHelper(and) {  // ִ�а�λ�룬������־�Ĵ�����λ��д�룩
  rtl_and(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);
  t1=0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);
  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);
  rtl_update_ZFSF(&t0,id_dest->width);
  
  t1=0;  // CF��OF���ó�0
  rtl_set_CF(&t1);  
  rtl_set_OF(&t1);

  print_asm_template2(xor);
}

make_EHelper(or) {  // ִ�а�λ�򣬲�����־�Ĵ�����λ��д�룩
  rtl_or(&t0,&id_dest->val,&id_src->val);
  operand_write(id_dest,&t0);
  t1=0;
  rtl_set_CF(&t1);
  rtl_set_OF(&t1);
  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template2(or);
}

make_EHelper(sar) {  // ��������
  
  // unnecessary to update CF and OF in NEMU
  rtl_sext(&t1, &id_dest->val, id_dest->width);
  rtl_sar(&t1, &t1, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {  // �߼�����
  rtl_shl(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {  // �߼�����
  rtl_shr(&t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  rtl_update_ZFSF(&t1, id_dest->width);
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {  // ��
  rtl_not(&id_dest->val);
  operand_write(id_dest,&id_dest->val);
  print_asm_template1(not);
}

make_EHelper(shld) {
  rtl_shl(&t0, &id_dest->val, &id_src->val);
  rtl_li(&t2, id_src2->width);
  rtl_shli(&t2, &t2, 3); // bit length
  rtl_subi(&t2, &t2, id_src->val);
  rtl_shr(&t2, &id_src2->val, &t2);
  rtl_or(&t0, &t0, &t2);
  operand_write(id_dest, &t0);

  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template3(shld);
}

make_EHelper(shrd) {
  rtl_shr(&t0, &id_dest->val, &id_src->val);
  rtl_li(&t2, id_src2->width);
  rtl_shli(&t2, &t2, 3); // bit length
  rtl_subi(&t2, &t2, id_src->val);
  rtl_shl(&t2, &id_src2->val, &t2);
  rtl_or(&t0, &t0, &t2);
  operand_write(id_dest, &t0);

  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template3(shrd);
}

