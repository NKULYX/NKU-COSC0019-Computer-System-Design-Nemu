#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp() {
  assert(free_ != NULL);
  WP *alloc = free_;
  free_ = free_->next;
  alloc->next = head;
  head = alloc;

  alloc->has_prev = false;
  alloc->prev_value = 0;
  alloc->expr[0] = '\0';
  return alloc;
}

void free_wp(WP *wp) {
  bool found = false;
  WP *prev = NULL;
  for(WP *p = head; p; prev = p, p = p->next) {
    if (p == wp) {
      found = true;
      break;
    }
  }
  assert(found);
  if (!prev) {
    head = wp->next;
  } else {
    prev->next = wp->next;
  }

  wp->next = free_;
  free_ = wp;
}

WP *wp_head() {
  return head;
}
