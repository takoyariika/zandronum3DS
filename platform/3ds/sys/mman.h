// libctru has no memory-mapped files. Calls report ENOSYS (see posix_stubs_3ds.c).
#pragma once
#include <stddef.h>
#include <sys/types.h>
#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_FAILED ((void *)-1)
#ifdef __cplusplus
extern "C" {
#endif
void *mmap(void *, size_t, int, int, int, off_t);
int munmap(void *, size_t);
#ifdef __cplusplus
}
#endif
