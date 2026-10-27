#include <sys/types.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <termios.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

/* z/OS compatibility shims for APIs used optionally by uv dependencies. */
int *errno_location(void) { return __errno(); }

/* Persistently mark an ASCII text file as ISO8859-1 (CCSID 819). Rust's raw
 * open/write calls bypass LE AUTOTAG, so generated files otherwise remain
 * untagged and are mis-converted as EBCDIC when _BPXK_AUTOCVT=ON. */
int zos_tag_text_path(const char *path) {
    attrib_t attr;
    memset(&attr, 0, sizeof(attr));
    attr.att_filetagchg = 1;
    attr.att_filetag.ft_ccsid = 819;
    attr.att_filetag.ft_txtflag = 1;

    /* The cross compiler does not apply the pragma that redirects __chattr to
     * its ASCII entry point. Convert the pathname for the EBCDIC entry point
     * explicitly; the file contents remain untouched ASCII. */
    char *ebcdic_path = strdup(path);
    if (ebcdic_path == NULL) {
        errno = ENOMEM;
        return -1;
    }
    __a2e_l(ebcdic_path, strlen(ebcdic_path));
    int rc = __chattr(ebcdic_path, &attr, sizeof(attr));
    int saved_errno = errno;
    free(ebcdic_path);
    errno = saved_errno;
    return rc;
}

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
