#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  uint64_t steps=1; //默认为1
  if(args==NULL){;}
  else{
    sscanf(args,"%llu",&steps);	//格式转换
  }
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args){
  char op;
  if(args==NULL){;}
  else{
    sscanf(args,"%c",&op);
    if(op=='r'){  //打印寄存器状态
      printf("eip: 0x%x\n", cpu.eip); //eip
      for(int i=0;i<8;i++)
        printf("%s: 0x%x\n",regsl[i],reg_l(i)); //32位寄存器名称+当前存储的值
      for(int i=0;i<8;i++)
        printf("%s: 0x%x\n",regsw[i],reg_w(i)); //16位寄存器名称+当前存储的值
      for(int i=0;i<8;i++)
        printf("%s: 0x%x\n",regsb[i],reg_b(i)); //8位寄存器名称+当前存储的值
      printf("CR0: 0x%x\n", cpu.CR0); //CR0
      printf("CR3: 0x%x\n", cpu.CR3); //CR3
    }
    if(op=='w')show_wp();  // 打印监视点状态
  }
  return 0;
}

static int cmd_x(char *args){
  int len;
  vaddr_t st;
  char *c;
  len = strtoul(args,&c,10); // N存入len，EXPR存入&c
  bool succ;
  st=expr(c+1,&succ);
  printf("Memory:\n");
  for(int i=0;i<len;i++){
    printf("0x%08x: ",st);  //打印地址
    uint32_t data =vaddr_read(st,4);
    printf("0x%08x\n",data); //打印数据
    st+=4;
  }
  return 0;
}

static int cmd_p(char *args){
  bool succ=1;
  int res=expr(args,&succ);
  if(succ==0)printf("求值失败！\n");
  else printf("%d\n",res);
  return 0;
}

static int cmd_w(char *args){
  bool succ=1;
  WP *wp=new_wp();
  strcpy(wp->exprv,args);
  wp->value=expr(args,&succ);
  if(succ==0)printf("求值失败！\n");
  printf("已将%d号监视点设置于%s\n",wp->NO,args);
  return 0;
}

static int cmd_d(char *args){
  int no;
  sscanf(args,"%d",&no);
  free_wp(no);
  printf("释放%d号监视点\n",no);
  return 0;
}

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "si [N] 单步执行N条指令", cmd_si},
  { "info", "info r 打印寄存器状态", cmd_info},
  { "x", "x N EXPR 从EXPR开始输出N个四字节数据", cmd_x},
  { "p", "p EXPR 求出表达式EXPR的值", cmd_p},
  { "w", "w EXPR 当EXPR的值发生变化时，暂停程序", cmd_w},
  { "d", "d N 删除N号监视点", cmd_d},
  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
