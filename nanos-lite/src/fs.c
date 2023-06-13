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

//��ʼ��FD_FB�Ĵ�С
void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size=_screen.height*_screen.width*4;
}


// ����fd��Ӧ�ļ���С
size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
	int i;
	for (i = 0; i < NR_FILES; i++) {
    // �ҵ�Ŀ���ļ������ض�Ӧ��fd
		if (strcmp(file_table[i].name, pathname) == 0) {
      file_table[i].open_offset=0;  // ����ָ�뵽�ļ���ͷ
			return i;
		}
	}
 // û�ҵ�Ŀ���ļ�������
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	ssize_t fsize = fs_filesz(fd);
	if (file_table[fd].open_offset + len > fsize) // ����ƫ���������ļ����޵Ĳ���
		len = fsize - file_table[fd].open_offset;
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

int fs_close(int fd) {
	return 0;  // Ĭ�ϳɹ���ֱ�ӷ���0
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	ssize_t fsize = fs_filesz(fd);
	switch(fd) {
		case FD_STDOUT:
		case FD_STDERR:
			for(int i = 0; i < len; i++) {
				_putc(((char*)buf)[i]);
			}
			break;
		case FD_FB:
			// �������Ļ
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:	
			if(file_table[fd].open_offset + len > fsize)
				len = fsize - file_table[fd].open_offset;
			// ��д�ļ�
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
	}
	return len; // ���ز������� 
}

// ����open_offsetƫ��
off_t fs_lseek(int fd, off_t offset, int whence) {
	off_t res = -1;
	switch(whence) {
		case SEEK_SET:
			if (offset >= 0 && offset <= file_table[fd].size) {
				file_table[fd].open_offset = offset;
				res = file_table[fd].open_offset;
			}
			break;
		case SEEK_CUR:
			if ((offset + file_table[fd].open_offset >= 0) && (offset + file_table[fd].open_offset <= file_table[fd].size)) {
				file_table[fd].open_offset += offset;
				res = file_table[fd].open_offset;
			}
			break;
		case SEEK_END:
			file_table[fd].open_offset = file_table[fd].size + offset;
			res = file_table[fd].open_offset;
			break;
	}
	return res;
}
