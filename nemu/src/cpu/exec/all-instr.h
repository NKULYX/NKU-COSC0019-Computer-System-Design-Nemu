#include "cpu/exec.h"

make_EHelper(mov);

make_EHelper(operand_size);

make_EHelper(inv);
make_EHelper(nemu_trap);

// in control.c
make_EHelper(call);
make_EHelper(ret);

// in data-mov.c
make_EHelper(push);
make_EHelper(pop);

// in arith.c
make_EHelper(sub);

// in logic.c
make_EHelper(xor);