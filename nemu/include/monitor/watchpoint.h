#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;
  int value;
  char exprv[33];

  /* TODO: Add more members if necessary */


} WP;

WP * new_wp();
void free_wp(int n);
void show_wp();
bool value_change();
#endif
