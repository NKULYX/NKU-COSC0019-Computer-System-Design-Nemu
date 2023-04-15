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
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_p(char* args);
static int cmd_x(char* args);
static int cmd_w(char* args);
static int cmd_d(char* args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step to the next instruction", cmd_si},
  { "info", "Print state of regs using 'r' or watchpoint using 'w'", cmd_info},
  { "p", "Calculate the result of target experssion", cmd_p},
  { "x", "Scan the target address of memory and show the value in the address", cmd_x},
  { "w", "Set watchpoint", cmd_w},
  { "d", "Delete watchpoint", cmd_d}
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

static int cmd_si(char *args) {
  int steps = strtok(args, " ") == NULL ? 1 : atoi(strtok(args, " "));
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args) {
  char* subcmd = strtok(args, " ");
  if(subcmd == NULL) {
    printf("Please input subcmd to show information\n");
  }
  else {
    if(strcmp(subcmd, "r") == 0) {
      for(int i = 0; i < 8; i++) {
        printf("%s : 0x%x\n", reg_name(i, 4), reg_l(i));
      }
    }
    else if(strcmp(subcmd, "w") == 0) {
      show_wp();
    }
    else {
      printf("Undefined subcmd\n");
    }
  }
  return 0;
}

static int cmd_p(char* args) {
  char* expression = strtok(args, " ");
  bool success = true;
  int value = expr(expression, &success);
  if(success) {
    printf("ans = 0x%08x\n", value);
  }
  else {
    printf("Illegal Expression!\n");
  }
  return 0;
}

static int cmd_x(char* args) {
  char* arg1 = strtok(args, " ");
  if(arg1 == NULL) {
    printf("Pleas input N for continuous n address\n");
    return 0;
  }
  int n = atoi(arg1);
  char* arg2 = args + strlen(arg1) + 1;
  arg2 = strtok(arg2, " ");
  if(arg2 == NULL) {
    printf("Pleas input Expr for beginning address\n");
    return 0;
  }
  bool success = true;
  int begin_address = expr(arg2, &success);
  if(success) {
    begin_address = strtoul(arg2, NULL, 16);
  }
  else {
    printf("Illegal Addrerss Expression!\n");
  }
  for(int i = 0; i < n; i++) {
    uint32_t value = paddr_read(begin_address, 4);
    printf("0x%x : 0x%08x\n", begin_address, value);
    begin_address += 4;
  }
  return 0;
}

static int cmd_w(char* args) {
  WP* wp = new_wp();
  char* arg = strtok(NULL, "\n");
  strcpy(wp->expr, arg);
  bool success = true;
  wp->value = expr(arg, &success);
  if(success){
    printf("watchpoint %d : %s\n", wp->NO, wp->expr);
  }
  else{
    printf("Illegal Expression!\n");
    free_wp(wp->NO);
  }
  return 0;
}

static int cmd_d(char* args) {
  char* arg = strtok(NULL, "\n");
  int no = atoi(arg);
  free_wp(no);
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