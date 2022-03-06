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
#include <filesystem>

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
#include <glob.h>
#include <stdexcept>
#include <sstream>

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
/******************************************************************************
 * NAMESPACE
 *****************************************************************************/

using namespace std;
namespace fs = std::filesystem;
using namespace FileSystemClient;

/******************************************************************************
 * GLOBALS
 *****************************************************************************/
static int fill_dir_plus = 0;
u_int64_t total_time;
struct timespec start, end;
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

/******************************************************************************
 * HELPER FUNCTIONS
 *****************************************************************************/
static void show_help(const char *progname)
{
	std::cout<<"usage: "<<progname<<" [-f] <mountpoint>\n\n";
}

// removes the '/' at the beginning
char * fs_relative_path(char * path)
{
    return path + 1;
}

void initgRPC()
{
	// make sure local path exists
	string command = string("mkdir -p ") + LOCAL_CACHE_PREFIX;
	dbgprintf("initgRPC: command %s\n", command.c_str());
	if (system(command.c_str()) != 0)
	{
		dbgprintf("initgRPC: system() failed\n");
	}

	// init grpc connection
	string target_address(SERVER_ADDR);
    options.client = new ClientImplementation(grpc::CreateChannel(target_address,
                                grpc::InsecureChannelCredentials()));
	dbgprintf("initgRPC: Client is contacting server: %s\n", SERVER_ADDR);
}

// filename pattern matcher
vector<std::string> glob(const std::string& pattern) {
    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);

    // collect all the filenames into a std::list<std::string>
    vector<string> filenames;
    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(string(glob_result.gl_pathv[i]));
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}

// For Crash Consistency
// Log v1
void init_single_log() {
  // Handle edge case crash when switching from log to newlog
  rename("/tmp/afs/newlog", "/tmp/afs/log");

  ifstream log;
  log.open("/tmp/afs/log", ios::in);
  if (log.is_open()) {
    string line;
    while (getline(log, line)) {
      options.client->CloseFile(-1, line);
      //dbgprintf("READ LOG ON INIT %s \n", line.c_str());
    }
  }
}

// For Crash Consistency
// Log v1
void write_single_log(const char *path){
  // Update log to say this file was modified
  ofstream log;
  log.open("/tmp/afs/log", ios::out | ios::app);
  if (log.is_open())
    log << fs_relative_path((char* ) path) << endl;
}

// For Crash Consistency
// Log v2
int createPendingFile(string rel_path)
{
	dbgprintf("createPendingFile: Entering function\n");
    string command = "touch " + options.client->to_flat_file(rel_path);
    dbgprintf("createPendingFile: command %s\n", command.c_str());
	dbgprintf("createPendingFile: Exiting function\n");
    return system(command.c_str());
}

// For Crash Consistency
// Log v2
int init_multi_log()
{
	dbgprintf("init_multi_log: Entering function\n");
	string pattern = string(LOCAL_CACHE_PREFIX) + "*.tmp";
	vector<string> to_remove = glob(pattern.c_str());
	dbgprintf("init_multi_log: to_remove size = %ld\n", to_remove.size());
	dbgprintf("init_multi_log: pattern = %s\n", pattern.c_str());
	for (auto file : to_remove)
	{
		dbgprintf("init_multi_log: file = %s\n", file.c_str());
		options.client->removePendingFile(file);
	}
	dbgprintf("init_multi_log: Exiting function\n");
    return 0;
}

/******************************************************************************
 * FUSE FUNCTIONS
 *****************************************************************************/

static void *fs_init(struct fuse_conn_info *conn,
		      struct fuse_config *cfg)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	//(void) conn;
	//cfg->use_ino = 1;        

	if (SINGLE_LOG) init_single_log(); // v1
	else init_multi_log(); // v2
	
	//cfg->entry_timeout = 0;
	//cfg->attr_timeout = 0;
	//cfg->negative_timeout = 0;
    clock_gettime(CLOCK_MONOTONIC, &end);
	total_time =  end.tv_nsec - start.tv_nsec;
    printf("init time = %lu nanoseconds \n", total_time);
	return NULL;
}

static int fs_mkdir(const char *path, mode_t mode)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_mkdir: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);
    
    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    res = options.client->MakeDir(rel_path, mode);
	
	if(res == -1) 
		return -errno;
	clock_gettime(CLOCK_MONOTONIC, &end);
	 total_time =  end.tv_nsec - start.tv_nsec;
    printf("mkdir time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_readdir: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

    return options.client->ReadDir(rel_path, buf, filler);
	
	if(res == -1) 
		return -errno;
		clock_gettime(CLOCK_MONOTONIC, &end);
	 total_time =  end.tv_nsec - start.tv_nsec;
    printf("readdir time = %lu nanoseconds \n", total_time);
	
	return 0;
}

static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_getattr: Entered\n");
	(void) fi;
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
	res = options.client->GetFileStat(rel_path, stbuf);

	dbgprintf("fs_getattr: res =  %d\n", res);
	if(res == -1) 
		return -errno;
	 clock_gettime(CLOCK_MONOTONIC, &end);
	 total_time =  end.tv_nsec - start.tv_nsec;
  printf("getattr time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_rmdir(const char *path)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_rmdir: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    res = options.client->RemoveDir(rel_path);
	
	if(res == -1) 
		return -errno;
	clock_gettime(CLOCK_MONOTONIC, &end);
	 total_time =  end.tv_nsec - start.tv_nsec;
    printf("rmdir time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_unlink(const char *path)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_unlink: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	res = options.client->DeleteFile(rel_path);
	
	if(res == -1) 
		return -errno;
	 clock_gettime(CLOCK_MONOTONIC, &end);
	 total_time =  end.tv_nsec - start.tv_nsec;
  printf("unlink time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_fsync: Entered\n");
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	(void) path;
	(void) isdatasync;
	(void) fi;
	 clock_gettime(CLOCK_MONOTONIC, &end);
	 total_time =  end.tv_nsec - start.tv_nsec;
     printf("fsync time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	dbgprintf("fs_mknod: Entered\n");
	int res;
	char * rel_path = fs_relative_path((char *) path);

	dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	res = options.client->CreateFile(rel_path, mode, rdev);

	if (res == -1)
		return -errno;
    clock_gettime(CLOCK_MONOTONIC, &end);
	total_time =  end.tv_nsec - start.tv_nsec;
  printf("mknod time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
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
		return -errno;

	fi->fh = res;
	clock_gettime(CLOCK_MONOTONIC, &end);
	total_time =  end.tv_nsec - start.tv_nsec;
    printf("open time = %lu nanoseconds \n", total_time);
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
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
	
	// Return -errorno or number of bytes read
	clock_gettime(CLOCK_MONOTONIC, &end);
	total_time =  end.tv_nsec - start.tv_nsec;
    printf("read time = %lu nanoseconds \n", total_time);
	return res;
}

static int fs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
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

	if (SINGLE_LOG) write_single_log(path); // v1 
	else createPendingFile(rel_path); // v2

	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

    if (res >= 0 && fsync(fd) == -1)
		res = -errno;

	if (fi == NULL)
	{
	#if PERFORMANCE == 1
		options.client->CloseFileWithStream(fd, rel_path);
	#else
		options.client->CloseFile(fd, rel_path);	
	#endif
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	total_time =  end.tv_nsec - start.tv_nsec;
  printf("write time = %lu nanoseconds \n", total_time);
	// Return -errorno or number of bytes written
	return res;
}

static int fs_release(const char *path, struct fuse_file_info *fi)
{
    clock_gettime(CLOCK_MONOTONIC, &start);
	int res;
	
	dbgprintf("fs_release: Entered\n");
	char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

#if PERFORMANCE == 1
	res = options.client->CloseFileWithStream(fi->fh, rel_path);
#else
	res = options.client->CloseFile(fi->fh, rel_path);
#endif
	
	if(res == -1) 
		return -errno;
		clock_gettime(CLOCK_MONOTONIC, &end);
	total_time =  end.tv_nsec - start.tv_nsec;
  printf("release time = %lu nanoseconds \n", total_time);
	return 0;
}

// TODO: decide to keep or no
static int fs_utimens(const char *path, const struct timespec tv[2], struct fuse_file_info *fi) {
	dbgprintf("fs_utimens: Entered\n");
    return 0;
}

// TODO: decide to keep or no
static int fs_access(const char *path, int dummy)
{
	dbgprintf("fs_access: Entered\n");
	return 0;
	
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

    initgRPC();
    
	return (fuse_main(argc, argv, &fsops, &options));
}
