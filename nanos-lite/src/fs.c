#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_open(const char *filepath, int flags, int mode) {
  int i = 0;
  for(; i < NR_FILES; i++) {
    if(strcmp(file_table[i].name, filepath) == 0) {
      Log("Open file:%s in file table at index:%d", filepath, i);
      return i;
    }
  }
  Log("Can not open file:%s", filepath);
  assert(0);
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	ssize_t fs_size = fs_filesz(fd);
	//if(file_table[fd].open_offset >= fs_size) //实际上不会出现这情况
		//return 0;
	if (file_table[fd].open_offset + len > fs_size) //偏移量不可以超过文件边界 超出部分舍弃
		len = fs_size - file_table[fd].open_offset;
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
		case FD_STDIN:
			return 0;
		case FD_EVENTS:
			len = events_read((void *)buf, len);
			break;
		case FD_DISPINFO:
			dispinfo_read(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;	
			break;
		default:
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	ssize_t fs_size = fs_filesz(fd);
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
			// call _putc()
			// 串口已被抽象成stdout stderr
			for(int i = 0; i < len; i++) {
				_putc(((char*)buf)[i]);
			}
			break;
		case FD_FB:
			// write to frame buffer 显存
			// device.c:fb_write buff中len字节输出到屏幕上offest处
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
			// write to ramdisk
			//if(file_table[fd].open_offset >= fs_size)
				//return 0;	
			if(file_table[fd].open_offset + len > fs_size)
				len = fs_size - file_table[fd].open_offset;
			// 对文件的真正读写
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			//Log("offset = %d", file_table[fd].open_offset);
			break;
	}
	return len;// 参见man 返回值
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	off_t result = -1;
	// fs.h
	// man 2 lseek 同时注意边界问题
	switch(whence) {
		case SEEK_SET:
			if (offset >= 0 && offset <= file_table[fd].size) {
				file_table[fd].open_offset = offset;
				result = file_table[fd].open_offset;
			}
			break;
		case SEEK_CUR:
			if ((offset + file_table[fd].open_offset >= 0) && (offset + file_table[fd].open_offset <= file_table[fd].size)) {
				file_table[fd].open_offset += offset;
				result = file_table[fd].open_offset;
			}
			break;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
			result = file_table[fd].open_offset;
			break;
	}
	
	return result;
}

int fs_close(int fd) {
	return 0;
}

