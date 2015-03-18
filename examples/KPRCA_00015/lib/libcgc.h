#ifndef _LIBCGC_H
#define _LIBCGC_H

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define	NULL ((void *)0)

typedef long unsigned int size_t;
typedef long signed int ssize_t;

#define SSIZE_MAX	2147483647
#define SIZE_MAX	4294967295
#define	FD_SETSIZE	1024

typedef long int _fd_mask;

#define	_NFDBITS (8 * sizeof(_fd_mask))

typedef struct {
	_fd_mask _fd_bits[FD_SETSIZE / _NFDBITS];
} fd_set;

#define	FD_ZERO(set)							\
	do {								\
		int __i;						\
		for (__i = 0; __i < (FD_SETSIZE / _NFDBITS); __i++)	\
			(set)->_fd_bits[__i] = 0;				\
	} while (0)
#define	FD_SET(b, set) \
	((set)->_fd_bits[b / _NFDBITS] |= (1 << (b & (_NFDBITS - 1))))
#define	FD_CLR(b, set) \
	((set)->_fd_bits[b / _NFDBITS] &= ~(1 << (b & (_NFDBITS - 1))))
#define	FD_ISSET(b, set) \
	((set)->_fd_bits[b / _NFDBITS] & (1 << (b & (_NFDBITS - 1))))

struct timeval {
	int tv_sec;
	int tv_usec;
};

#define	EBADF		1
#define	EFAULT		2
#define	EINVAL		3
#define	ENOMEM		4
#define	ENOSYS		5
#define	EPIPE		6

void _terminate(unsigned int status) __attribute__ ((regparm(0)));
int transmit(int fd, const void *buf, size_t count, size_t *tx_bytes) __attribute__ ((regparm(0)));
int receive(int fd, void *buf, size_t count, size_t *rx_bytes) __attribute__ ((regparm(0)));
int fdwait(int nfds, fd_set *readfds, fd_set *writefds,
	   const struct timeval *timeout, int *readyfds) __attribute__ ((regparm(0)));
int allocate(size_t length, int is_X, void **addr) __attribute__ ((regparm(0)));
int deallocate(void *addr, size_t length) __attribute__ ((regparm(0)));
int random(void *buf, size_t count, size_t *rnd_bytes) __attribute__ ((regparm(0)));

#endif /* _LIBCGC_H */
