#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

extern void game_change();
size_t events_read(void *buf, size_t len) {
  int key = _read_key();
  bool is_down = false;
  if(key & 0x8000 ) {
    key ^= 0x8000;
    is_down = true;
  }
  // 先判断时钟事件
  if(key == _KEY_NONE) {
    uint32_t ut = _uptime();
    sprintf(buf, "t %d\n", ut);
  } 
  // 再判断按键事件
  else {
    sprintf(buf, "%s %s\n", is_down ? "kd" : "ku", keyname[key]);
    // 如果按下F12，交换当前运行的程序
    if(key==_KEY_F12 && is_down){
      game_change();
    }
  }
  return strlen(buf);
  return 0;
}

static char dispinfo[128] __attribute__((used));

// 把offset开始的len字节写到buf中
void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf,dispinfo+offset,len);
}

// 把len字节写到offset处
void fb_write(const void *buf, off_t offset, size_t len) {
  int r = (offset/4) / _screen.width;
  int c = (offset/4) % _screen.width;
  _draw_rect((uint32_t*)buf, c, r, len/4, 1);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  int h = _screen.height;
  int w = _screen.width;
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", w, h);
}
