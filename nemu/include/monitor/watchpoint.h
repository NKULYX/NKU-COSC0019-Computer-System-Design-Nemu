#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

#define MAX_LEN 128

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  /* TODO: Add more members if necessary */
  char expr[MAX_LEN];
  uint32_t value;
} WP;

WP* new_wp();
void free_wp(int no);
bool check_wp();
void show_wp();

#endif