#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NEQ, TK_NUM, TK_HEXNUM, TK_REG, TK_AND, TK_OR, TK_DEREF, TK_NEG

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
  {"==", TK_EQ},         // equal 
  {"!=", TK_NEQ},	// 不等于
  {"\\+", '+'},         // plus
  {"\\-", '-'},		// 减
  {"\\*", '*'},		// 乘
  {"\\/", '/'},		// 除
   {"0[Xx][a-fA-F0-9]+", TK_HEXNUM}, // 16进制数字;要放在TK_NUM的上面！！
  {"[0-9]|[1-9][0-9]*", TK_NUM}, // 数字
 
  {"\\(", '('},		// 左括号
  {"\\)", ')'},		// 右括号
  {"&&", TK_AND},	// 与
  {"\\|\\|", TK_OR},	// 或;不能直接用||！！
  {"!", '!'},		// 非
  {"\\$(eax|ebx|ecx|edx|esp|ebp|esi|edi|eip|ax|bx|cx|dx|sp|bp|si|di|al|bl|cl|dl|ah|bh|ch|dh)", TK_REG}
  
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
	if(substr_len>31)assert(0);	// 溢出报错，照手册上写
	if(rules[i].token_type==TK_NOTYPE)break; //空格不处理
        
	      
        switch (rules[i].token_type) {
	  case TK_NUM:{		// 数字挨个复制
	    for(int i=0;i<substr_len;i++)
		tokens[nr_token].str[i]=substr_start[i];	
	    tokens[nr_token].str[substr_len]='\0';	
	    break;  
  	  }
	  case TK_HEXNUM:{	// 16进制数字跳过0x进行复制
	    for(int i=2;i<substr_len;i++)
	      tokens[nr_token].str[i-2]=substr_start[i];
	    tokens[nr_token].str[substr_len-2]='\0';
	    break;
	  }
	  case TK_REG:{		// 寄存器跳过$进行复制
	    for(int i=1;i<substr_len;i++)
	      tokens[nr_token].str[i-1]=substr_start[i];
	    tokens[nr_token].str[substr_len-1]='\0';
	    break;
	  }
          default:break;
        }
	tokens[nr_token++].type=rules[i].token_type;  
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

int check_parentheses(int p, int q){	// 括号检查
  if(tokens[p].type!='(' || tokens[q].type!=')')return 0; //是否有边缘括号
  int leftp=0,rightp=0;
  for(int i=p+1;i<q;i++){
    if(tokens[i].type=='(')leftp++;
    else if(tokens[i].type==')'){
      if(leftp>rightp)rightp++;
      else return 0;	// 右括号多了，表达式出错
    }
  }
  if(leftp==rightp)return 1; // 正确
  else return 0;	// 右括号少了，表达式出错
}

int get_domi_oper(int p,int q){	  // 寻找主运算符
  int pos=-1,level=0;	// level越高，优先级越小
  int f=1;  // f表示是否在括号中
  for(int i=p;i<=q;i++){
    if(tokens[i].type=='('){f=0;continue;}
    else if(tokens[i].type==')'){f=1;continue;}
    if(!f)continue;  //在括号中必定找不到domi
    int token_level=0;
    if(tokens[i].type==TK_NEG||tokens[i].type==TK_DEREF||tokens[i].type=='!')token_level=1;  //各运算符优先级
    else if(tokens[i].type=='*'||tokens[i].type=='/')token_level=2;
    else if(tokens[i].type=='+'||tokens[i].type=='-')token_level=3;
    else if(tokens[i].type==TK_EQ||tokens[i].type==TK_NEQ)token_level=4;
    else if(tokens[i].type==TK_AND||tokens[i].type==TK_OR)token_level=5;
    if(token_level>=level){
      level=token_level;
      pos=i;
    }
  }
  return pos;
}

static int eval(int p,int q){	// 求值函数
  //printf("%d ",check_parentheses(p,q));
  if(p>q)assert(0);
  else if(p==q){
    int res;
    switch(tokens[q].type){
      case TK_NUM:
        sscanf(tokens[q].str,"%d",&res);
	return res;
      case TK_HEXNUM:
	sscanf(tokens[q].str,"%x",&res);
        return res;
      case TK_REG:
	if(strcmp(tokens[q].str,"eip")==0)return cpu.eip;
	for(int i=0;i<8;i++){
	  if(strcmp(tokens[q].str,regsl[i])==0)return reg_l(i);
	  else if(strcmp(tokens[q].str,regsw[i])==0)return reg_w(i);
	  else if(strcmp(tokens[q].str,regsb[i])==0)return reg_b(i);
	}
    }
  }
  else if(check_parentheses(p,q)==1)return eval(p+1,q-1); // 去除边缘括号
  else{
    int pos=get_domi_oper(p,q);
    //printf("%d ",pos);
    if(tokens[pos].type==TK_NEG)return -eval(p+1,q);
    else if(tokens[pos].type==TK_DEREF){
      int st=eval(p+1,q);
      uint32_t res = vaddr_read(st,4);
      return res;
    }
    else if(tokens[pos].type=='!'){
      int res=eval(p+1,q);
      res=res==0?1:0;
      return res;
    }
    int val1=eval(p,pos-1);
    int val2=eval(pos+1,q);
    //printf("%d %d ",val1,val2);
    switch(tokens[pos].type){	// 返回结果
      case '+':return val1+val2;
      case '-':return val1-val2;
      case '*':return val1*val2;
      case '/':return val1/val2;
      case TK_EQ:return val1==val2;
      case TK_NEQ:return val1!=val2;
      case TK_AND:return val1&&val2;
      case TK_OR:return val1||val2;
    }
  }
  return -1;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  for(int i=0;i<nr_token;i++){
    if(tokens[i].type=='-'){  // 负号的处理
      if(i==0||(tokens[i-1].type!=TK_NUM&&tokens[i-1].type!=TK_HEXNUM&&tokens[i-1].type!=TK_REG&&tokens[i-1].type!=')'))tokens[i].type=TK_NEG;
    }
    else if(tokens[i].type=='*'){  // 解引用的处理，同负号
      if(i==0||(tokens[i-1].type!=TK_NUM&&tokens[i-1].type!=TK_HEXNUM&&tokens[i-1].type!=TK_REG&&tokens[i-1].type!=')'))tokens[i].type=TK_DEREF;
    }
  }

  return eval(0,nr_token-1);
}
