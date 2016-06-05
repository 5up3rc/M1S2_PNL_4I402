#define _GNU_SOURCE
#include <unistd.h>
#include <dlfcn.h>

typedef ssize_t (*orig_read_f_type)(int, void *, size_t);

ssize_t read(int fd, void *buf, size_t count)
{
	orig_read_f_type orig_read = (orig_read_f_type)dlsym(RTLD_NEXT, "read");
	ssize_t ret = orig_read(fd, buf, count);

  /* TODO : replace any 'r' by 'i' */

	return ret;
}
