#define FUSE_USE_VERSION 31

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifdef linux
/* For pread()/pwrite()/utimensat() */
#define _XOPEN_SOURCE 700
#endif

#include <limits.h>
#include <stdlib.h>

#include <iostream>

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

static void fs_cachepath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, mount);
    char temp[strlen(path)];
    strcpy(temp, path);
    for (int i = 1; i < (int) strlen(temp); i++)
      if (temp[i] == '/')
        temp[i] = '-';
    strncat(fpath, temp, PATH_MAX);
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags)
{
	//MAKE GRPC CALL?
	
	//Local version left in for now, needs to be replaced with GRPC:
	char fpath[PATH_MAX];
	DIR *dp;
	struct dirent *de;
	(void) offset;
	(void) fi;
	(void) flags;

	fs_cachepath(fpath, path);

	dp = opendir(fpath);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0, (fuse_fill_dir_flags) fill_dir_plus))
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

	fs_cachepath(fpath, path);

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
    
	fs_cachepath(fpath, path);
        
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

	fs_cachepath(fpath, path);
	
	res = lstat(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_rename(const char *from, const char *to, unsigned int flags)
{
	//MAKE GRPC CALL?
	
	//Local version left in for now, needs to be replaced with GRPC:
	int res;
	char fpath_from[PATH_MAX];
	char fpath_to[PATH_MAX];

	fs_cachepath(fpath_from, from);
	fs_cachepath(fpath_to, to);

	if (flags)
		return -EINVAL;

	res = rename(fpath_from, fpath_to);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_truncate(const char *path, off_t size,
			struct fuse_file_info *fi)
{
	//MAKE GRPC CALL?
	//This might not be supported by a server operation
	
	//Local version left in for now, needs to be replaced with GRPC:
	int res;
	char fpath[PATH_MAX];

	fs_cachepath(fpath, path);

	if (fi != NULL)
		res = ftruncate(fi->fh, size);
	else
		res = truncate(fpath, size);
	if (res == -1)
		return -errno;

	return 0;
}

static int fs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	printf("fs_write invoked\n");
	int fd;
	int res;
	(void) fi;
	char fpath[PATH_MAX];

	fs_cachepath(fpath, path);

	if(fi == NULL)
		fd = open(fpath, O_WRONLY);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	if(fi == NULL)
		close(fd);
	return res;
}

static int fs_fsync(const char *path, int isdatasync,
		     struct fuse_file_info *fi)
{

	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */



	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

static int fs_create(const char *path, mode_t mode,
		      struct fuse_file_info *fi)
{
	//MAKE GRPC CALL?
	//Might not be supported by server operation or necessary
	
	//Local version left in for now, needs to be replaced with GRPC:
	
	int res;
	char fpath[PATH_MAX];

	fs_cachepath(fpath, path);
	
	res = open(fpath, fi->flags, mode);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
}

static int fs_statfs(const char *path, struct statvfs *stbuf)
{
	
	//Not sure how to handle this one
	
	int res;
	char fpath[PATH_MAX];

	fs_cachepath(fpath, path);
	
	res = statvfs(fpath, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

struct fuse_operations fsops = {
	.getattr = fs_getattr,
	.rename = fs_rename,
	.truncate = fs_truncate,
	.open = fs_open,
	.read = fs_read,
	.write = fs_write,
	.statfs = fs_statfs,
	.fsync = fs_fsync,	
	.readdir = fs_readdir,
	.create = fs_create,
};

int
main(int argc, char *argv[])
{   
	mount = realpath(argv[2], NULL);
	argc--;
	std::cout << mount << std::endl;
	umask(0);
	return (fuse_main(argc, argv, &fsops, NULL));
}
