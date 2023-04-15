#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_HEX, TK_DEC, TK_REG, TK_EQ, TK_NEQ, TK_AND, TK_OR, TK_RS, TK_LS, TK_GEQ, TK_LEQ, TK_DEREF, TK_NEG
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},                          // spaces
  {"0x[0-9A-Fa-f][0-9A-Fa-f]*", TK_HEX},      // hex number
  {"0|[1-9][0-9]*", TK_DEC},                  // dec number
  {"\\$(eax|ecx|edx|ebx|esp|ebp|esi|edi|eip|ax|cx|dx|bx|sp|bp|si|di|al|cl|dl|bl|ah|ch|dh|bh)", TK_REG},
  {"\\+", '+'},                               // add
  {"-", '-'},                                 // sub
  {"\\*", '*'},                               // mul
  {"\\/", '/'},                               // div
  {"\\(", '('},                               // lparen
  {"\\)", ')'},                               // rparen
  {"==", TK_EQ},                              // equal
  {"!=", TK_NEQ},                             // not equal
  {"&&", TK_AND},                             // and
  {"\\|\\|", TK_OR},                          // or
  {">>", TK_RS},                              // right shift
  {"<<", TK_LS},                              // left shift
  {"<=", TK_LEQ},                             // less equal
  {">=", TK_GEQ},                             // great equal
  {"<", '<'},                                 // less 
  {">", '>'}                                  // great
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
          case TK_NOTYPE:
            break;
          default:
            // if length is greater than buffer size
            if(substr_len >= 32) {
              printf("Token length is too long!\n");
              return false;
            }
            tokens[nr_token].type = rules[i].token_type;
            memset(tokens[nr_token].str, 0, 32);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            nr_token++;
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

bool check_parentheses(int p,int q){
  int cnt = 0;
  for(int i = p;i < q; i++){
    if(tokens[i].type == '('){
      cnt++;
    }
    else if(tokens[i].type==')'){
      cnt--;
    }
    if(cnt<=0){
      if(cnt<0){  //brankets unmatched
        printf("( and ) unmatched!\n");
        assert(0);
      }
      else return false;  //not in the (<expr>) format
    }
  }
  if(cnt != 1 || tokens[q].type!=')'){
    printf("( and ) unmatched!\n");
    assert(0);
  }
  return true;
}

int get_operator_priority(int type) {
  switch(type){
    case TK_NOTYPE:
    case ')':
      return 0;
    case TK_OR:
      return 1;
    case TK_AND:
      return 2;
    case TK_EQ:
    case TK_NEQ:
      return 3;
    case '<':
    case '>':
    case TK_LEQ:
    case TK_GEQ:
      return 4;
    case TK_LS:
    case TK_RS:
      return 5;
    case '+':
    case '-':
      return 6;
    case '*':
    case '/':
      return 7;
    case TK_NEG:
    case TK_DEREF:
      return 8;
    case '(':
      return 9;
  }
  assert(0);
  return 0;
}

bool is_number_token(int type) {
  if(type == TK_DEC || type == TK_HEX || type == TK_REG) {
    return true;
  }
  return false;
}

int get_token_value(Token token) {
  int ret_value = 0;
    if(token.type == TK_DEC) {
      sscanf(token.str, "%d", &ret_value);
      return ret_value;
    }
    else if(token.type == TK_HEX) {
      sscanf(token.str, "%x", &ret_value);
      return ret_value;
    }
    else if(token.type == TK_REG) {
      char reg[4] = {0};
      sscanf(token.str, "$%s", reg);
      for(int i = 0; i < 8; i++) {
        if(strcasecmp(reg, regsl[i]) == 0) {
          return cpu.gpr[i]._32;
        }
        else if(strcasecmp(reg, regsw[i]) == 0) {
          return cpu.gpr[i]._16;
        }
        else if(strcasecmp(reg, regsb[i]) == 0) {
          return cpu.gpr[i % 4]._8[i / 4];
        }
        else if(strcasecmp(reg, "eip") == 0) {
          return cpu.eip;
        }
        else {
          printf("Illegal reg!\n");
          assert(0);
        }
      }
    }
    else {
      printf("Illegal value token!\n");
      assert(0);
    }
    return 0;
}

int eval(int p, int q, bool* success) {
  if(p > q) {
    printf("Bad Expression!\n");
    success = false;
    return 0;
  }
  // only one token
  if(p == q) {
    return get_token_value(tokens[p]);
  }
  // else if(check_parentheses(p, q) == true){
  //   return eval(p + 1, q - 1);
  // }
  else {
    tokens[++q].type = TK_NOTYPE;
    // operand stack and operator stack
    int operands_stack[32];
    Token operators_stack[32];
    int operands_index = -1, operators_index = 0;
    operators_stack[0].type = TK_NOTYPE;
    for(int i = p; i <= q; i++) {
      // if token is number push into operand stack
      if(is_number_token(tokens[i].type)) {
        operands_stack[++operands_index] = get_token_value(tokens[i]);
      }
      // if token is operator
      else {
        // check priority of the top of stack
        // if current operator's priority is greater than the top of stack
        // push the token into operator stack
        if(get_operator_priority(tokens[i].type) > get_operator_priority(operators_stack[operators_index].type)) {
          if(tokens[i].type =='(') {
            operators_stack[++operators_index].type = ')';
          }
          else {
            operators_stack[++operators_index] = tokens[i];
          }
        }
        // else calculate the tmp result
        else {
          while(operators_stack[operators_index].type != TK_NOTYPE &&
                get_operator_priority(tokens[i].type) <= get_operator_priority(operators_stack[operators_index].type)) {
            if(operators_stack[operators_index].type == ')') {
              operators_index--;
              break;
            }
            else {
              switch(operators_stack[operators_index--].type) {
                case '+': operands_stack[operands_index - 1] = operands_stack[operands_index] + operands_stack[operands_index - 1];operands_index--;break;
                case '-': operands_stack[operands_index - 1] = operands_stack[operands_index] - operands_stack[operands_index - 1];operands_index--;break;
                case '*': operands_stack[operands_index - 1] = operands_stack[operands_index] * operands_stack[operands_index - 1];operands_index--;break;
                case '/': operands_stack[operands_index - 1] = operands_stack[operands_index] / operands_stack[operands_index - 1];operands_index--;break;
                case '<': operands_stack[operands_index - 1] = operands_stack[operands_index] < operands_stack[operands_index - 1];operands_index--;break;
                case '>': operands_stack[operands_index - 1] = operands_stack[operands_index] > operands_stack[operands_index - 1];operands_index--;break;
                case TK_EQ: operands_stack[operands_index - 1] = operands_stack[operands_index] == operands_stack[operands_index - 1];operands_index--;break;
                case TK_NEQ: operands_stack[operands_index - 1] = operands_stack[operands_index] != operands_stack[operands_index - 1];operands_index--;break;
                case TK_AND: operands_stack[operands_index - 1] = operands_stack[operands_index] && operands_stack[operands_index - 1];operands_index--;break;
                case TK_OR: operands_stack[operands_index - 1] = operands_stack[operands_index] || operands_stack[operands_index - 1];operands_index--;break;
                case TK_LS: operands_stack[operands_index - 1] = operands_stack[operands_index] << operands_stack[operands_index - 1];operands_index--;break;
                case TK_RS: operands_stack[operands_index - 1] = operands_stack[operands_index] >> operands_stack[operands_index - 1];operands_index--;break;
                case TK_LEQ: operands_stack[operands_index - 1] = operands_stack[operands_index] <= operands_stack[operands_index - 1];operands_index--;break;
                case TK_GEQ: operands_stack[operands_index - 1] = operands_stack[operands_index] >= operands_stack[operands_index - 1];operands_index--;break;
                case TK_NEG: operands_stack[operands_index] = -operands_stack[operands_index];break;
                case TK_DEREF: operands_stack[operands_index] = paddr_read(operands_stack[operands_index], 4);break;
                default:success = false;return 0;
              }
            }
          }
          if(tokens[i].type != ')' && tokens[i].type != TK_NOTYPE) {
            operators_stack[++operators_index] = tokens[i];
          }
        }
      }
    }
    if(operators_index != 0) {
      *success = false;
      return 0;
    }
    return operands_stack[0];
  }
}


uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  // deal with one operand operator
  for(int i = 0; i < nr_token; i++) {
    if(tokens[i].type == '*' || tokens[i].type == '-') {
      if(i == 0 || 
        (tokens[i - 1].type != TK_DEC && 
         tokens[i - 1].type != TK_HEX &&
         tokens[i - 1].type != TK_REG &&
         tokens[i - 1].type != ')')) {
        tokens[i].type = tokens[i].type == '*' ? TK_DEREF : TK_NEG;
      }
    }
  }
  // return the calculated result
  int res = eval(0, nr_token - 1, success);
  return res;
}