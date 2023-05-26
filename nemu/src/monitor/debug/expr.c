#include "nemu.h"

#if 0
/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ

  /* TODO: Add more token types */

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ}         // equal
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  TODO();

  return 0;
}
#endif // if 0

#include <stdio.h>
#include <stdint.h>
#include <string.h>

static int isspace(int v) {
  return v == ' ' || v == '\t';
}

static int isnumber(int v) {
  return v >= '0' && v <= '9';
}

static int isalpha(int v) {
  return (v >= 'a' && v <= 'z') ||
         (v >= 'A' && v <= 'Z');
}

typedef struct {
  const char *buffer;
  size_t idx;
} ParserCtx;

static ParserCtx ctx;

static void next() {
  while (isspace(ctx.buffer[++ctx.idx]));
}

static void nexti(int i) {
  while (i--) {
    next();
  }
}

static char chr() {
  return ctx.buffer[ctx.idx];
}

static const char *pchr() {
  return ctx.buffer + ctx.idx;
}

typedef struct {
  int32_t value;
  uint8_t valid;
} Option;

static Option option() {
  return (Option) {0, 0};
}

static Option make_value(int32_t v) {
  return (Option) {v, 1};
}

static Option oadd(Option a, Option b) {
  Option result;
  result.value = a.value + b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option osub(Option a, Option b) {
  Option result;
  result.value = a.value - b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option omul(Option a, Option b) {
  Option result;
  result.value = a.value * b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option odiv(Option a, Option b) {
  // sanity check
  if (b.value == 0) {
    return option();
  }
  Option result;
  result.value = a.value / b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option onot(Option v) {
  v.value = !v.value;
  return v;
}

static Option olt(Option a, Option b) {
  Option result;
  result.value = a.value < b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option ogt(Option a, Option b) {
  Option result;
  result.value = a.value > b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option ole(Option a, Option b) {
  Option result;
  result.value = a.value <= b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option oge(Option a, Option b) {
  Option result;
  result.value = a.value >= b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option oeq(Option a, Option b) {
  Option result;
  result.value = a.value == b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option one(Option a, Option b) {
  Option result;
  result.value = a.value != b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option oand(Option a, Option b) {
  Option result;
  result.value = a.value && b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option oor(Option a, Option b) {
  Option result;
  result.value = a.value || b.value;
  result.valid = a.valid && b.valid;
  return result;
}

static Option lor_expr();

static Option land_expr();

static Option eq_expr();

static Option rel_expr();

static Option add_expr();

static Option mul_expr();

static Option factor();

static Option number();

static Option variable();

static Option dereference(Option p);

static Option lor_expr() {
  Option result = land_expr();
  while (chr() == '|' && pchr()[1] == '|') {
    nexti(2);
    result = oor(result, land_expr());
  }
  return result;
}

static Option land_expr() {
  Option result = eq_expr();
  while (chr() == '&' && pchr()[1] == '&') {
    nexti(2);
    result = oand(result, eq_expr());
  }
  return result;
}

static Option eq_expr() {
  Option result = rel_expr();
  while ((chr() == '=' || chr() == '!') && pchr()[1] == '=') {
    switch (chr()) {
      case '=':
        nexti(2);
        result = oeq(result, rel_expr());
        break;
      case '!':
        nexti(2);
        result = one(result, rel_expr());
        break;
    }
  }
  return result;
}

static Option rel_expr() {
  Option result = add_expr();
  while (chr() == '<' || chr() == '>') {
    switch (chr()) {
      case '<':
        if (pchr()[1] == '=') {
          nexti(2);
          result = ole(result, add_expr());
        } else {
          next();
          result = olt(result, add_expr());
        }
        break;
      case '>':
        if (pchr()[1] == '=') {
          nexti(2);
          result = oge(result, add_expr());
        } else {
          next();
          result = ogt(result, add_expr());
        }
        break;
    }
  }
  return result;
}

static Option add_expr() {
  Option result = mul_expr();
  while (chr() == '+' || chr() == '-') {
    switch (chr()) {
      case '+':
        next();
        result = oadd(result, mul_expr());
        break;
      case '-':
        next();
        result = osub(result, mul_expr());
        break;
    }
  }
  return result;
}

static Option mul_expr() {
  Option result = factor();
  while (chr() == '*' || chr() == '/') {
    switch (chr()) {
      case '*':
        next();
        result = omul(result, factor());
        break;
      case '/':
        next();
        result = odiv(result, factor());
        break;
    }
  }
  return result;
}

static Option factor() {
  switch (chr()) {
    case '+':
      next();
      return factor();
    case '-':
      next();
      return osub(make_value(0), factor());
    case '!':
      next();
      return onot(factor());
    case '*':
      next();
      return dereference(factor());
  }

  if (chr() == '(') {
    next();
    Option result = lor_expr();
    if (chr() != ')') {
      return option();
    }
    next();
    return result;
  } else if (isnumber(chr())) {
    return number();
  } else if (chr() == '$') {
    return variable();
  }
  return option();
}

static Option number() {
  int32_t result = 0;
  int32_t factor;
  if (chr() == '0' && pchr()[1] == 'x') {
    nexti(2);
    factor = 16;
  } else if (chr() == '0' && pchr()[1] == 'b') {
    nexti(2);
    factor = 2;
  } else if (chr() == '0') {
    next();
    factor = 8;
  } else {
    factor = 10;
  }

  while ((chr() >= '0' && chr() <= '9') ||
         (chr() >= 'a' && chr() <= 'f') ||
         (chr() >= 'A' && chr() <= 'F')) {
    int32_t v = chr();
    next();

    if (v >= 'a' && v <= 'f') {
      v = v - 'a' + 10;
    } else if (v >= 'A' && v <= 'F') {
      v = v - 'A' + 10;
    } else {
      v = v - '0';
    }
    result = result * factor + v;
  }
  return make_value(result);
}

static Option variable() {
  if (chr() != '$') {
    return option();
  }
  next();

  char buf[64];
  int32_t i = 0;

  while (isalpha(chr()) && i < 64) {
    buf[i++] = chr();
    next();
  }
  buf[i] = '\0';

  for (int i = 0; i < 8; i++) {
    if (strcmp(buf, regsl[i]) == 0) {
      return make_value(reg_l(i));
    }
    if (strcmp(buf, regsw[i]) == 0) {
      return make_value(reg_w(i));
    }
    if (strcmp(buf, regsb[i]) == 0) {
      return make_value(reg_b(i));
    }
  }
  if (strcmp(buf, "eip") == 0) {
    return make_value(cpu.eip);
  }
  return option();
}

static Option dereference(Option p) {
  if (!p.valid) {
    return option();
  }
  return make_value(vaddr_read(p.value, 4));
}

uint32_t expr(char *e, bool *success) {
  ctx.buffer = e;
  ctx.idx = 0;
  Option opt = lor_expr();
  *success = opt.valid;
  return opt.value;
}

// dummy function
void init_regex() {}