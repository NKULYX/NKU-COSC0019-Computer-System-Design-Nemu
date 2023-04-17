#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

// in control.c
make_EHelper(call);
make_EHelper(ret);
make_EHelper(jcc);
make_EHelper(jmp); 

// in data-mov.c
make_EHelper(push);
make_EHelper(pop);
make_EHelper(lea);
make_EHelper(movzx);
make_EHelper(movsx);
make_EHelper(pusha);
make_EHelper(popa);
make_EHelper(cltd);
make_EHelper(leave);

// in arith.c
make_EHelper(sub);
make_EHelper(add);
make_EHelper(cmp); 
make_EHelper(dec);
make_EHelper(inc);
make_EHelper(neg);
make_EHelper(adc);
make_EHelper(imul1);
make_EHelper(imul2);
make_EHelper(imul3);
make_EHelper(idiv);
make_EHelper(sbb); 

// in logic.c
make_EHelper(xor);
make_EHelper(and);
make_EHelper(setcc); 
make_EHelper(test);
make_EHelper(sar);
make_EHelper(shl);
make_EHelper(shr);
make_EHelper(or); 
make_EHelper(not);

// in special.c
make_EHelper(nop);