#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
  return 0;
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf,dispinfo+offset,len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  int index_begin = offset >> 2;
  int x_begin = index_begin % _screen.width;
  int y_begin = index_begin / _screen.width;
  int index_end = (offset + len) >> 2;
  int x_end = index_end % _screen.width;
  int y_end = index_end / _screen.width;
  if(y_begin == y_end) {
    _draw_rect(buf, x_begin, y_begin, x_end - x_begin, 1);
  }
  else if(y_end - y_begin == 1) {
    _draw_rect(buf, x_begin, y_begin, _screen.width - x_begin, 1);
    _draw_rect(buf - x_end * 4, 0, y_end, x_end, 1);
  }
  else {
    _draw_rect(buf, x_begin, y_begin, _screen.width - x_begin, 1);
    _draw_rect(buf + (_screen.width - x_begin) * 4, 0, y_begin + 1, _screen.width, y_end - y_begin - 1);
    _draw_rect(buf - x_end, 0, y_end, x_end, 1);
  }

}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo,"WIDTH:%d\nHEIGHT:%d\n",_screen.width,_screen.height);
}
