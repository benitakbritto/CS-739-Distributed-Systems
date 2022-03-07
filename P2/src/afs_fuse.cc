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
#include <chrono>

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
	string command = string("mkdir -p ") + cache_root;
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
	
	std::chrono::nanoseconds ping_time;
	options.client->Ping(&ping_time);
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
  
  ofstream log;
  log.open(cache_root + "log", ios::out | ios::trunc);
  
  //Close-log init implementation:
  // Handle edge case crash when switching from log to newlog
  /*ifstream check_log;
  check_log.open(cache_root + "log", ios::in);
  if (!check_log.is_open())
    rename(cache_root + "newlog", cache_root + "log");
  ifstream log;
  log.open(cache_root + "log", ios::in);
  if (log.is_open()) {
    string line;
    while (getline(log, line)) {
      options.client->CloseFile(-1, line);
      //dbgprintf("READ LOG ON INIT %s \n", line.c_str());
    }
  }*/
}

// For Crash Consistency
// Log v1
void write_single_log(const char *path){
  // Update log to say this file was modified
  ofstream log;
  log.open(cache_root + "log", ios::out | ios::app);
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
	// This would close all dirty writes
	// dbgprintf("init_multi_log: Entering function\n");
	// string pattern = cache_root + "*.tmp";
	// vector<string> to_remove = glob(pattern.c_str());
	// dbgprintf("init_multi_log: to_remove size = %ld\n", to_remove.size());
	// dbgprintf("init_multi_log: pattern = %s\n", pattern.c_str());
	// for (auto file : to_remove)
	// {
	// 	dbgprintf("init_multi_log: file = %s\n", file.c_str());
	// 	options.client->CloseFile(-1, from_flat_file(file));
	// }
	// dbgprintf("init_multi_log: Exiting function\n");
    // return 0;
	// This would delete all .tmp files
	dbgprintf("init_multi_log: Entering function\n");
	string command = "rm -rf " + cache_root + "*.tmp";
	if (system(command.c_str()) != 0)
	{
		dbgprintf("init_multi_log: system() failed\n");
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
	//(void) conn;
	//cfg->use_ino = 1;        

	if (SINGLE_LOG) init_single_log(); // v1
	else init_multi_log(); // v2
	
	//cfg->entry_timeout = 0;
	//cfg->attr_timeout = 0;
	//cfg->negative_timeout = 0;

	return NULL;
}

static int fs_mkdir(const char *path, mode_t mode)
{ 
    auto start = std::chrono::steady_clock::now();
	dbgprintf("fs_mkdir: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);
    
    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    res = options.client->MakeDir(rel_path, mode);
	
	if(res == -1) 
		return -errno;
	
  auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "mkdir time: " << ns.count() << "nanoseconds";
	return 0;
}

static int fs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags)
{
    auto start = std::chrono::steady_clock::now();
	dbgprintf("fs_readdir: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

    return options.client->ReadDir(rel_path, buf, filler);
	
	if(res == -1) 
		return -errno;

  auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "readdir time: " << ns.count() << "nanoseconds";
	return 0;
}

static int fs_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
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
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "getattr time: " << ns.count() << "nanoseconds";
  
	return 0;
}

static int fs_rmdir(const char *path)
{
    auto start = std::chrono::steady_clock::now();
	dbgprintf("fs_rmdir: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);
    
    res = options.client->RemoveDir(rel_path);
	
	if(res == -1) 
		return -errno;
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "rmdir time: " << ns.count() << "nanoseconds";
	return 0;
}

static int fs_unlink(const char *path)
{
    auto start = std::chrono::steady_clock::now();
	dbgprintf("fs_unlink: Entered\n");
	int res;
    char * rel_path = fs_relative_path((char *) path);

    dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	res = options.client->DeleteFile(rel_path);
	
	if(res == -1) 
		return -errno;
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "unlink time: " << ns.count() << "nanoseconds";
	return 0;
}

static int fs_fsync(const char *path, int isdatasync, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
	dbgprintf("fs_fsync: Entered\n");
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	(void) path;
	(void) isdatasync;
	(void) fi;
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "fsync time: " << ns.count() << "nanoseconds";
  return 0;
}

static int fs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    auto start = std::chrono::steady_clock::now();
	dbgprintf("fs_mknod: Entered\n");
	int res;
	char * rel_path = fs_relative_path((char *) path);

	dbgprintf("path = %s\n", path);
    dbgprintf("rel_path = %s\n", rel_path);

	res = options.client->CreateFile(rel_path, mode, rdev);

	if (res == -1)
		return -errno;
    auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "mknod time: " << ns.count() << "nanoseconds";
	return 0;
}

static int fs_open(const char *path, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
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
	
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "open time: " << ns.count() << "nanoseconds";
	return 0;
}

static int fs_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
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
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "read time: " << ns.count() << "nanoseconds";
	// Return -errorno or number of bytes read
	return res;
}

static int fs_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
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

	res = pwrite(fd, buf, size, offset);
	
	if (SINGLE_LOG) write_single_log(path); // v1 
	else createPendingFile(rel_path); // v2

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
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "write time: " << ns.count() << "nanoseconds";
	// Return -errorno or number of bytes written
	return res;
}

static int fs_release(const char *path, struct fuse_file_info *fi)
{
    auto start = std::chrono::steady_clock::now();
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
		
	auto end = std::chrono::steady_clock::now();
 std::chrono::nanoseconds ns = end-start;
  std::cout << "release time: " << ns.count() << "nanoseconds";
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
	if(argc != 4) {
		printf("usage: %s -f [network directory] [cache path]\n", argv[0]);
		return 1;
	}
	
	auto cache_root_fs = std::filesystem::weakly_canonical(argv[3]);
	
	cache_root = cache_root_fs.string();
	
	// Ensure cache path ends with slash (to match legacy macro)
	if(cache_root[cache_root.length()-1] != '/') {
		cache_root += "/";
	}
	
	printf("Initializing with cache path %s\n", cache_root.c_str());
	
	argc--; // hide last arg
	
	umask(0);
	
	initgRPC();
	
	return (fuse_main(argc, argv, &fsops, NULL));
}