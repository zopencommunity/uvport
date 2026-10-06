#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>

/* z/OS compatibility shims for APIs used optionally by uv dependencies. */
int *errno_location(void) { return __errno(); }

unsigned int get_cpu_num(void) {
#ifdef _SC_NPROCESSORS_ONLN
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? (unsigned int)n : 1U;
#else
    return 1U;
#endif
}

int posix_fadvise(int fd, off_t offset, off_t len, int advice) {
    (void)fd; (void)offset; (void)len; (void)advice;
    return 0; /* advisory operation */
}

int posix_fallocate(int fd, off_t offset, off_t len) {
    (void)fd; (void)offset; (void)len;
    return ENOSYS;
}

int futimens(int fd, const void *times) {
    (void)fd; (void)times;
    errno = ENOSYS;
    return -1;
}

ssize_t preadv(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
    (void)fd; (void)iov; (void)iovcnt; (void)offset;
    errno = ENOSYS;
    return -1;
}

ssize_t pwritev(int fd, const struct iovec *iov, int iovcnt, off_t offset) {
    (void)fd; (void)iov; (void)iovcnt; (void)offset;
    errno = ENOSYS;
    return -1;
}

int cfsetspeed(struct termios *termios_p, speed_t speed) {
    int rc = cfsetispeed(termios_p, speed);
    if (rc == 0) rc = cfsetospeed(termios_p, speed);
    return rc;
}

int setdomainname(const char *name, size_t len) {
    (void)name; (void)len;
    errno = ENOSYS;
    return -1;
}

void cfmakeraw(struct termios *t) {
    t->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    t->c_oflag &= ~OPOST;
    t->c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    t->c_cflag &= ~(CSIZE | PARENB);
    t->c_cflag |= CS8;
    t->c_cc[VMIN] = 1;
    t->c_cc[VTIME] = 0;
}
