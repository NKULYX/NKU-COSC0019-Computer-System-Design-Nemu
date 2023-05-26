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

static int cmd_si(char *args) {
  if (!args) {
    cpu_exec(1);
    return 0;
  }

  int v;
  int ret = sscanf(args, "%d", &v);
  if (ret != 1) {
    printf("invalid param '%s'\n", args);
    return 0;
  }

  cpu_exec(v);
  return 0;
}

static int cmd_info(char *args) {
  if (!args) {
    printf("need param\n");
    return 0;
  }
  switch (*args) {
    case 'r':
      for (int i = 0; i < 4; i++) {
        printf("%4s = 0x%08x    %4s = 0x%08x\n",
            regsl[i], reg_l(i), regsl[i + 4], reg_l(i + 4));
      }
      printf("%4s = 0x%08x\n", "eip", cpu.eip);
      printf("\n");
      break;
    case 'w':
      for(WP *wp = wp_head(); wp; wp = wp->next) {
        printf("%2d: %s\n", wp->NO, wp->expr);
      }
      break;
    default:
      printf("invalid param '%s'\n", args);
      break;
  }
  return 0;
}

static int cmd_p(char *args) {
  bool valid;
  int result = expr(args, &valid);
  if (!valid) {
    printf("invalid expression\n");
    return 0;
  }
  printf("%d\n"
         "0x%08x\n", result, result);
  return 0;
}

static int cmd_x(char *args) {
  int n;
  if (sscanf(args, "%d", &n) != 1) {
    printf("invalid param '%s'\n", args);
    return 0;
  }

  char *p = strchr(args, ' ');
  if (!p || !p[1]) {
    printf("invalid param '%s'\n", args);
    return 0;
  }

  bool valid;
  int addr = expr(p + 1, &valid);
  if (!valid) {
    printf("invalid expression\n");
    return 0;
  }
  addr &= ~0xF;

  int cnt = 0;
  for (int i = 0; i < (n + 3) / 4; i++) {
    printf ("0x%08x : ", addr + i * 0x10);
    for (int j = 0; j < 4 && cnt < n; j++) {
      printf("%08x  ", vaddr_read(addr + cnt++, 4));
    }
    printf("\n");
  }
  return 0;
}

static int cmd_w(char *args) {
  if (!args) {
    printf("need param\n");
    return 0;
  }
  WP *alloc = new_wp();
  strcpy(alloc->expr, args);
  return 0;
}

static int cmd_d(char *args) {
  int delete_ID;
  if (sscanf(args, "%d", &delete_ID) != 1) {
    printf("invalid param '%s'\n", args);
    return 0;
  }
  for(WP *wp = wp_head(); wp; wp = wp->next) {
    if (wp->NO == delete_ID) {
      free_wp(wp);
      return 0;
    }
  }
  printf("invalid ID: %d\n", delete_ID);
  return 0;
}

static int cmd_help(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Single step", cmd_si },
  { "info", "Show nemu states", cmd_info },
  { "p", "Evaluate an expression", cmd_p },
  { "x", "Memory inspection", cmd_x },
  { "w", "Add a watchpoint", cmd_w },
  { "d", "Delete a watchpoint", cmd_d },
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
