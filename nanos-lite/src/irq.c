#include "common.h"

extern _RegSet* do_syscall(_RegSet *r);
extern _RegSet* schedule(_RegSet *prev);

static _RegSet* do_event(_Event e, _RegSet* r) {
  switch (e.event) {
    case _EVENT_SYSCALL:  // 对此事件调用do_syscall
      return do_syscall(r);
    case _EVENT_TRAP:
      printf("trap hit\n");
      return schedule(r);
    case _EVENT_IRQ_TIME:
      Log("timer hit");
      return schedule(r);
    default: panic("Unhandled event ID = %d", e.event);
  }
  
  return NULL;
}

void init_irq(void) {
  _asye_init(do_event);
}
