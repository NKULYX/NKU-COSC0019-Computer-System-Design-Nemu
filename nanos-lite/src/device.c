#include "common.h"

#define KEYDOWN_MASK 0x8000

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

#define MIN(a, b) ((a)<(b)?(a):(b))

extern int current_game;
size_t events_read(void *buf, size_t len) {
  int ori_key_id = _read_key();
  int key_id = ori_key_id & (KEYDOWN_MASK - 1);
  bool is_down = !!(ori_key_id & (KEYDOWN_MASK));
  Log("key id = %d, is down = %d", key_id, is_down);

  if (key_id == _KEY_NONE) {
    snprintf(buf, len, "t %d\n", _uptime());
  } else {
    snprintf(buf, len, "%s %s\n", is_down ? "kd" : "ku", keyname[key_id]);
    if (key_id == _KEY_F12 && is_down) {
      current_game = (~current_game) & 1;
    }
  }
  return strlen(buf);
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
  memcpy(buf, dispinfo + offset, len);
}

void fb_write(const void *buf, off_t offset, size_t len) {
  // _draw_rect(
  //   (const uint32_t *) buf,
  //   (offset / 4) % _screen.width,
  //   (offset / 4) / _screen.width,
  //   len / 4,
  //   1
  // );
  memcpy((uint8_t *)_get_fb() + offset, buf, len);
}

void init_device() {
  _ioe_init();

  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d\n", _screen.width, _screen.height);
  Log("%s", dispinfo);
}
