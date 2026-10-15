// Adapted from Ultimate Doom Launcher 3DS, revision in sources.lock.json.
/* POSIX process/user calls newlib declares but the 3DS cannot provide. Subsystems sheet: posix_stubs.
 * Callers: cmdlib.cpp NicePath (~user expansion), music_midi_timidity.cpp (external TiMidity process).
 * Every stub reports failure, so the engine takes its existing "not available" paths. */
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <wordexp.h>
#include <sys/wait.h>
#include <fcntl.h>
uid_t geteuid(void) { return 0; }
int fchown(int fd, uid_t owner, gid_t group)
{ (void)fd; (void)owner; (void)group; errno = ENOSYS; return -1; }
ssize_t pread(int fd, void *buf, size_t count, off_t offset)
{
    off_t previous = lseek(fd, 0, SEEK_CUR);
    if (previous == (off_t)-1 || lseek(fd, offset, SEEK_SET) == (off_t)-1) return -1;
    ssize_t n = read(fd, buf, count);
    int saved = errno;
    if (lseek(fd, previous, SEEK_SET) == (off_t)-1) return -1;
    errno = saved;
    return n;
}
#include "sys/mman.h"
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{ (void)addr; (void)length; (void)prot; (void)flags; (void)fd; (void)offset; errno = ENOSYS; return MAP_FAILED; }
int munmap(void *addr, size_t length)
{ (void)addr; (void)length; errno = ENOSYS; return -1; }

uid_t getuid (void) { return 0; }
struct passwd *getpwuid (uid_t uid) { (void)uid; return NULL; }
struct passwd *getpwnam (const char *name) { (void)name; return NULL; }
int pipe (int fildes[2]) { (void)fildes; errno = ENOSYS; return -1; }
int execvp (const char *file, char * const argv[]) { (void)file; (void)argv; errno = ENOSYS; return -1; }
pid_t waitpid (pid_t pid, int *status, int options) { (void)pid; (void)status; (void)options; errno = ECHILD; return -1; }
int wordexp (const char *__restrict words, wordexp_t *__restrict we, int flags)
{
	(void)words; (void)flags;
	we->we_wordc = 0;
	we->we_wordv = NULL;
	return WRDE_NOSPACE;
}
void wordfree (wordexp_t *we) { (void)we; }
