#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  decoding.is_jmp=1;
  rtl_push(&decoding.seq_eip);  // 当前pc压栈
  //cpu.eip=decoding.jmp_eip;  // eip指向跳转目标地址

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {  
  rtl_pop(&t1);
  decoding.jmp_eip=t1;  // 跳转到栈顶记录的返回地址
  decoding.is_jmp=1;

  print_asm("ret");
}

make_EHelper(call_rm) {
  decoding.is_jmp=1;
  decoding.jmp_eip = id_dest->val;
  rtl_push(&decoding.seq_eip);

  print_asm("call *%s", id_dest->str);
}
