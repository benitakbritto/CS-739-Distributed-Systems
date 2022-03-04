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
//#define SERVER_ADDR         "20.69.154.6:50051"
#define SERVER_ADDR           "0.0.0.0:50051"

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
char *mount = "/home/benitakbritto/CS-739-Distributed-Systems/P2/src/build/dir_2";

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

// wrong
static void show_help(const char *progname)
{
	std::cout<<"usage: "<<progname<<" [-s -d] <mountpoint>\n\n";
}


// Helper Functions
static void fs_fullpath(char fpath[PATH_MAX], const char *path)
{
    strcpy(fpath, mount);
    strncat(fpath, path, PATH_MAX);
}


// FUSE functions
static int fs_mkdir(const char *path, mode_t mode)
{
    char fpath[PATH_MAX];
    fs_fullpath(fpath, path);

    return options.client->MakeDir(fpath, mode);
}


static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    char fpath[PATH_MAX];
    fs_fullpath(fpath, path);

    return options.client->ReadDir(fpath, buf, filler);
}


static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
	(void) fi;
	int res;
    char fpath[PATH_MAX];
    fs_fullpath(fpath, path);
	
	return options.client->GetFileStat(fpath, stbuf);
}

static int fs_rmdir(const char *path)
{
    char fpath[PATH_MAX];
    fs_fullpath(fpath, path);
    return options.client->RemoveDir(fpath);
}

static int fs_unlink(const char *path)
{
	int res;
	char fpath[PATH_MAX];
    
	fs_fullpath(fpath, path);

	return options.client->DeleteFile(fpath);
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

struct fuse_operations fsops = {
    .getattr = fs_getattr,
    .mkdir = fs_mkdir,
    .unlink = fs_unlink,
    .rmdir = fs_rmdir,
    .readdir = fs_readdir,
};

int
main(int argc, char *argv[])
{   
    //mount = realpath(argv[2], NULL);
    //printf("mount: %s \n", mount);
    //argc--;
	umask(0);

    // Init grpc
    string target_address(SERVER_ADDR);
    options.client = new ClientImplementation(grpc::CreateChannel(target_address,
                                grpc::InsecureChannelCredentials()));
    
    printf("Calling fuse_main\n");
	return (fuse_main(argc, argv, &fsops, &options));
}
