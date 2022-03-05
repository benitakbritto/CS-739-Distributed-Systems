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
#define PERFORMANCE           0

#ifdef CRASH_TEST
#define crash() *((char*)0) = 0;
#else
#define crash() do {} while(0)
#endif

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
static void *fs_init(struct fuse_conn_info *conn,
		      struct fuse_config *cfg)
{
	//(void) conn;
	//cfg->use_ino = 1;        

	// Handle edge case crash when switching from log to newlog
	rename("/tmp/afs/newlog", "/tmp/afs/log");

	ifstream log;
	log.open("/tmp/afs/log", ios::in);
	if (log.is_open()) {
	  string line;
	  while (getline(log, line)) {
	    options.client->CloseFile(-1, line);
	    dbgprintf("READ LOG ON INIT %s \n", line.c_str());
	  }
	}

	//cfg->entry_timeout = 0;
	//cfg->attr_timeout = 0;
	//cfg->negative_timeout = 0;

	return NULL;
}

static int fs_mkdir(const char *path, mode_t mode)
{
	dbgprintf("fs_mkdir: Entered\n");
    char * rel_path = fs_relative_path((char *) path);
    
    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    return options.client->MakeDir(rel_path, mode);
}

// TODO: decide to keep or no
static int fs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
	dbgprintf("fs_utimens: Entered\n");
    return 0;
}


static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
	dbgprintf("fs_readdir: Entered\n");
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

    return options.client->ReadDir(rel_path, buf, filler);
}


static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	dbgprintf("fs_getattr: Entered\n");
	(void) fi;
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
	int ret  = options.client->GetFileStat(rel_path, stbuf);

	dbgprintf("fs_getattr: ret =  %d\n", ret);
	return ret;
}

static int fs_rmdir(const char *path)
{
	dbgprintf("fs_rmdir: Entered\n");
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    return options.client->RemoveDir(rel_path);
}

static int fs_unlink(const char *path)
{
	dbgprintf("fs_unlink: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	return options.client->DeleteFile(rel_path);
}

static int fs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
	dbgprintf("fs_fsync: Entered\n");
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	(void) path;
	(void) isdatasync;
	(void) fi;
	return 0;
}

// TODO: create grpc version
static int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	dbgprintf("fs_mknod: Entered\n");
	int res;
	char * rel_path = fs_relative_path((char *) path);

	dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	res = options.client->CreateFile(rel_path, mode, rdev);

	if (res == -1)
		return -errno;

	return 0;
}

// TODO: decide to keep or no
static int fs_access(const char *path, int dummy)
{
	dbgprintf("fs_access: Entered\n");
	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
	dbgprintf("fs_open: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
#if PERFORMANCE == 1
    res = options.client->OpenFileWithStream(rel_path);
#else
	res = options.client->OpenFile(rel_path);
#endif
	if (res == -1)
		return -res;

	fi->fh = res;
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	dbgprintf("fs_read: Entered\n");
	int fd;
	int res;
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	if (fi == NULL)
	{
	#if PERFORMANCE == 1
		fd = options.client->OpenFileWithStream(rel_path);
	#else
		fd = options.client->OpenFile(rel_path);
	#endif
	}
	else
		fd = fi->fh;
	
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	if (fi == NULL)
	{
	#if PERFORMANCE == 1
		options.client->CloseFileWithStream(fd, rel_path);
	#else
		options.client->CloseFile(fd, rel_path);
	#endif
	}
		
	return res;
}

static int fs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	dbgprintf("fs_write: Entered\n");
	int fd;
	int res;
	(void) fi;
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	if (fi == NULL)
	{
	#if PERFORMANCE == 1
		fd = options.client->OpenFileWithStream(rel_path);
	#else
		fd = options.client->OpenFile(rel_path);
	#endif	
	}	
	else
	{
		fd = fi->fh;
	}
	
	if (fd == -1)
		return -errno;

	// Update log to say this file was modified
	ofstream log;
	log.open("/tmp/afs/log", ios::out | ios::app);
	if (log.is_open())
		log << fs_relative_path((char* ) path) << endl;

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
	{
		res = -errno;
		return res;
	}

    if (fsync(fd) == -1)
		return -errno;

	if (fi == NULL)
	{
	#if PERFORMANCE == 1
		options.client->CloseFileWithStream(fd, rel_path);
	#else
		options.client->CloseFile(fd, rel_path);	
	#endif
	}
		

	return res;
}

static int fs_release(const char *path, struct fuse_file_info *fi)
{
	dbgprintf("fs_release: Entered\n");
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

#if PERFORMANCE == 1
	return options.client->CloseFileWithStream(fi->fh, rel_path);
#else
	return options.client->CloseFile(fi->fh, rel_path);
#endif
}

struct fuse_operations fsops = {
    .getattr = fs_getattr,
	.mknod = fs_mknod,
	.mkdir = fs_mkdir,
	.unlink = fs_unlink,
	.rmdir = fs_rmdir,
    .open = fs_open,
    .read = fs_read,
    .write = fs_write,
    .release = fs_release,
	.fsync = fs_fsync, 
	.readdir = fs_readdir,
	.init = fs_init,
	.access = fs_access,
	.utimens = fs_utimens,
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
