#define FUSE_USE_VERSION 31

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <limits.h>
#include <stdlib.h>

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#ifdef __FreeBSD__
#include <sys/socket.h>
#include <sys/un.h>
#endif
#include <sys/time.h>
#ifdef HAVE_SETXATTR
#include <sys/xattr.h>
#endif

static int fill_dir_plus = 0;
char *mount;

static void fs_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, mount);
    strncat(fpath, path, PATH_MAX);
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags)
{

	char fpath[PATH_MAX];
	DIR *dp;
	struct dirent *de;
	(void) offset;
	(void) fi;
	(void) flags;

	fs_fullpath(fpath, path);

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0, fill_dir_plus))
			break;
	}

	closedir(dp);
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	char fpath[PATH_MAX];
	int fd;
	int res;

	fs_fullpath(fpath, path);

	if(fi == NULL)
		fd = open(fpath, O_RDONLY);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	if(fi == NULL)
		close(fd);
	return res;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
	char fpath[PATH_MAX];
	int res;
    
	fs_fullpath(fpath, path);
        
	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
}

static int fs_getattr(const char *path, struct stat *stbuf,
		       struct fuse_file_info *fi)
{
	char fpath[PATH_MAX];
	(void) fi;
	int res;

	fs_fullpath(fpath, path);
	
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_rename(const char *from, const char *to, unsigned int flags)
{
	int res;
	char fpath_from[PATH_MAX];
	char fpath_to[PATH_MAX];

	fs_fullpath(fpath_from, from);
	fs_fullpath(fpath_to, to);

	if (flags)
		return -EINVAL;

	res = rename(fpath_from, fpath_to);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_unlink(const char *path)
{
	int res;
	char fpath[PATH_MAX];
    
	fs_fullpath(fpath, path);

	res = unlink(fpath);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_truncate(const char *path, off_t size,
			struct fuse_file_info *fi)
{
	int res;
	char fpath[PATH_MAX];

	fs_fullpath(fpath, path);

	if (fi != NULL)
		res = ftruncate(fi->fh, size);
	else
		res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}


struct fuse_operations fsops = {
	.readdir = fs_readdir,
	.read = fs_read,
	.open = fs_open,
	.getattr = fs_getattr,
	.unlink	= fs_unlink,
	.rename = fs_rename,
	.truncate = fs_truncate,
};

int
main(int argc, char *argv[])
{   
	mount = realpath(argv[2], NULL);
        argc--;
	umask(0);
	return (fuse_main(argc, argv, &fsops, NULL));
}
