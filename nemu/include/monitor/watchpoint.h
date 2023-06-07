#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
  bool has_prev;
  int prev_value;
  char expr[128];

} WP;

WP* new_wp();
void free_wp(WP *wp);
WP* wp_head();

#endif
