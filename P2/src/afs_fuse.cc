/*
* Reference: https://github.com/libfuse/libfuse/blob/master/example/passthrough.c
*/

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

#include "afs_client.h"

// MACROS
//#define SERVER_ADDR         "20.69.154.6:50051" // for testing on 2 machines
#define SERVER_ADDR           "0.0.0.0:50051" // for testing on 1 machine

// NAMESPACES
using namespace std;
using namespace FileSystemClient;

// GLOBALS
static int fill_dir_plus = 0;

static struct options {	
	ClientImplementation * client;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

// wrong - TODO
static void show_help(const char *progname)
{
	std::cout<<"usage: "<<progname<<" [-s -d] <mountpoint>\n\n";
}

// Helper Functions
// removes the '/' at the beginning
char * fs_relative_path(char * path)
{
    return path + 1;
}


// FUSE functions
static int fs_mkdir(const char *path, mode_t mode)
{
    char * rel_path = fs_relative_path((char *) path);
    
    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    return options.client->MakeDir(rel_path, mode);
}


static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

    return options.client->ReadDir(rel_path, buf, filler);
}


static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	(void) fi;
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
	return options.client->GetFileStat(rel_path, stbuf);
}

static int fs_rmdir(const char *path)
{
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    return options.client->RemoveDir(rel_path);
}

static int fs_unlink(const char *path)
{
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	return options.client->DeleteFile(rel_path);
}

static int fs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

// TODO - test
static int fs_open(const char *path, struct fuse_file_info *fi)
{
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
        
	res = options.client->OpenFile(rel_path);
	if (res == -1)
		return -res;

	fi->fh = res;
	return 0;
}

// TODO - test
static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd;
	int res;
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	if (fi == NULL) // file not open?
		fd = options.client->OpenFile(rel_path);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	if (fi == NULL)
		options.client->CloseFile(fd, rel_path);
	return res;
}

// TODO - test
static int fs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	int fd;
	int res;
	(void) fi;
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	if (fi == NULL)
		fd = options.client->OpenFile(rel_path);
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
	{
		res = -errno;
		return res;
	}

    if (fsync(fd) == -1)
		return -errno;

	if (fi == NULL)
		options.client->CloseFile(fd, rel_path);

	return res;
}

// TODO - test
static int fs_release(const char *path, struct fuse_file_info *fi)
{
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	return options.client->CloseFile(fi->fh, rel_path);
}


struct fuse_operations fsops = {
    .getattr = fs_getattr,
	.mkdir = fs_mkdir,
	.unlink = fs_unlink,
	.rmdir = fs_rmdir,
    .open = fs_open,
    .read = fs_read,
    .write = fs_write,
    .release = fs_release,
	.fsync = fs_fsync,
	.readdir = fs_readdir,
};

int
main(int argc, char *argv[])
{   
	umask(0);

    // Init grpc
    string target_address(SERVER_ADDR);
    options.client = new ClientImplementation(grpc::CreateChannel(target_address,
                                grpc::InsecureChannelCredentials()));
    
	return (fuse_main(argc, argv, &fsops, &options));
}
