#ifndef __UNISTD_H__
#define __UNISTD_H__

#define __NR_read 0
#define __NR_write 1


int write(int fd, const void *buf, int count);

#endif /* __UNISTD_H__ */
