// grpc code here

#include <grpcpp/grpcpp.h>
#include <string>
#include <chrono>
#include <ctime>
#include "filesystemcomm.grpc.pb.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <utime.h>
#include <fstream>
#include <sstream>

/******************************************************************************
 * MACROS
 *****************************************************************************/
#define DEBUG                 1                                     // for debugging
#define dbgprintf(...)        if (DEBUG) { printf(__VA_ARGS__); }   // for debugging
#define MAX_RETRY             5                                     // rpc retry
#define RETRY_TIME_START      1                                     // in seconds
#define RETRY_TIME_MULTIPLIER 2                                     // for rpc retry w backoff
#define LOCAL_CACHE_PREFIX    "/tmp/afs/"                           // location of local files
#define CHUNK_SIZE            1024                                  // for streaming
//#define SERVER_ADDR         "52.151.53.152:50051"                 // Server: VM1
//#define SERVER_ADDR         "20.69.154.6:50051"                   // Server: VM2
//#define SERVER_ADDR         "20.69.94.59:50051"                   // Server: VM3
#define SERVER_ADDR           "0.0.0.0:50051"                       // Server: self
#define PERFORMANCE           0                                     // set to 1 to run performant functions
#define CRASH_TEST                                                  //Remove to disable all crashes
#define SINGLE_LOG            1                                     // Turns on single log functionality

#ifdef CRASH_TEST
#define crash(...) if (__VA_ARGS__) *((char*)0) = 0; else do {} while(0)
#else
#define crash() do {} while(0)
#endif

/******************************************************************************
 * NAMESPACES
 *****************************************************************************/
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using filesystemcomm::FileSystemService;
using filesystemcomm::PingMessage;
using filesystemcomm::FetchRequest;
using filesystemcomm::FetchResponse;
using filesystemcomm::GetFileStatRequest;
using filesystemcomm::GetFileStatResponse;
using filesystemcomm::ListDirRequest;
using filesystemcomm::ListDirResponse;
using filesystemcomm::MakeDirRequest;
using filesystemcomm::MakeDirResponse;
using filesystemcomm::MknodRequest;
using filesystemcomm::MknodResponse;
using filesystemcomm::RemoveDirRequest;
using filesystemcomm::RemoveDirResponse;
using filesystemcomm::RemoveRequest;
using filesystemcomm::RemoveResponse;
using filesystemcomm::RenameRequest;
using filesystemcomm::RenameResponse;
using filesystemcomm::StoreRequest;
using filesystemcomm::StoreResponse;
using filesystemcomm::TestAuthRequest;
using filesystemcomm::TestAuthResponse;
using filesystemcomm::DirectoryEntry;
using filesystemcomm::Timestamp;
using filesystemcomm::FileStat;
using grpc::Status;
using grpc::StatusCode;
using namespace std;
using std::ifstream;
using std::ostringstream;
using grpc::ClientWriter;
using grpc::ClientReader;

// Globals
struct TestAuthReturnType
{
  Status status;
  TestAuthResponse response;
  TestAuthReturnType(Status status,
                    TestAuthResponse response) :
                    status(status),
                    response(response)
                    {}
};  

namespace FileSystemClient
{
    class ClientImplementation 
    {
        public:
            ClientImplementation(std::shared_ptr<Channel> channel)
                : stub_(FileSystemService::NewStub(channel)) {}
            
            // Convert an RPC failure status code to something like an errno
            int transform_rpc_err(grpc::StatusCode code) {
                switch(code) {
                    case StatusCode::OK:
                        return 0;
                    case StatusCode::INVALID_ARGUMENT:
                        return EINVAL;
                    case StatusCode::NOT_FOUND:
                        return ENOENT;
                    case StatusCode::DEADLINE_EXCEEDED:
                    case StatusCode::UNAVAILABLE:
                        return ETIMEDOUT;
                    case StatusCode::ALREADY_EXISTS:
                        return EEXIST;
                    case StatusCode::PERMISSION_DENIED:
                    case StatusCode::UNAUTHENTICATED:
                        return EPERM;
                    default:
                        return EIO; // fall back to IO err for unexpected issues
                }
            }
            
            int Ping(std::chrono::nanoseconds * round_trip_time) {
                auto start = chrono::steady_clock::now();
                
                PingMessage request;
                PingMessage reply;
                Status status;
                uint32_t retryCount = 0;
                
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    dbgprintf("Ping: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->Ping(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    
                    dbgprintf("Ping: RPC Success\n");
                    auto end = chrono::steady_clock::now();
                    *round_trip_time = end-start;
                    #if DEBUG
                    std::chrono::duration<double,std::ratio<1,1>> seconds = end-start;
                    dbgprintf("Ping: Exiting function (took %fs)\n",seconds.count());
                    #endif
                    return 0;
                }
                else
                {
                    dbgprintf("Ping: RPC failure\n");
                    dbgprintf("Ping: Exiting function\n");
                    return -1;
                }
                
                
            }
            
            int MakeDir(std::string path, mode_t mode) 
            {
                dbgprintf("MakeDir: Entering function\n");
                MakeDirRequest request;
                MakeDirResponse reply;
                Status status;
                uint32_t retryCount = 0;
                
                request.set_pathname(path);
                request.set_mode(mode);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("MakeDir: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->MakeDir(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("MakeDir: RPC Success\n");
                    dbgprintf("MakeDir: Exiting function\n");
                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("MakeDir: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                    
                    // making the dir in cache directory, hierarchical if necessary
                    create_path(path, false);
                    return 0;
                }
                else
                {
                    dbgprintf("MakeDir: RPC failure\n");
                    dbgprintf("MakeDir: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }
            }

            int CreateFile(std::string path, mode_t mode, dev_t rdev)
            {
                dbgprintf("CreateFile: Entering function\n");
                MknodRequest request;
                MknodResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);
                request.set_mode(mode);
                request.set_dev(rdev);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("CreateFile: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->Mknod(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("CreateFile: RPC Success\n");
                    dbgprintf("CreateFile: Exiting function\n");
                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("CreateFile: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }

                    // adding file to local cache
                    mknod(get_cache_path(path).c_str(), mode, rdev);
                    return 0;
                } 
                else 
                {
                    dbgprintf("CreateFile: RPC Failure\n");
                    dbgprintf("CreateFile: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }
            }

            
            int RemoveDir(std::string path) 
            {
                dbgprintf("RemoveDir: Entering function\n");
                RemoveDirRequest request;
                RemoveDirResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("RemoveDir: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->RemoveDir(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("RemoveDir: RPC Success\n");
                    dbgprintf("RemoveDir: Exiting function\n");

                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("RemoveDir: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                    
                    // removing directory from cache
                    rmdir(get_cache_path(path).c_str());
                    return 0;

                } 
                else 
                {
                    dbgprintf("RemoveDir: RPC Failure\n");
                    dbgprintf("RemoveDir: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }
            }

            int ReadDir(std::string path, void *buf, fuse_fill_dir_t filler) 
            {
                dbgprintf("ListDir: Entering function\n");
                ListDirRequest request;
                ListDirResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("ListDir: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->ReadDir(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // std::cout << "count = " << reply.entries().size() << std::endl;

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("ListDir: RPC Success\n");
                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("ListDir: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                } 
                else 
                {
                    // std::cout << status.error_code() << ": " << status.error_message()
                    //           << std::endl;
                    //PrintErrorMessage(status.error_code(), status.error_message(), "ListDir");
                    dbgprintf("ListDir: RPC Failure\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }

                dbgprintf("ListDir: Exiting function\n");
                for (auto itr = reply.entries().begin(); itr != reply.entries().end(); itr++)
                {
                    struct stat st;
                    memset(&st, 0, sizeof(st));
                    st.st_ino = itr->size();
                    st.st_mode = itr->mode();
                    if (filler(buf, itr->file_name().c_str() , &st, 0, static_cast<fuse_fill_dir_flags>(0)))
                        break;
                }
                return 0;
            }

            int GetFileStat(std::string path, struct stat *stbuf) 
            {
                dbgprintf("GetFileStat: Entering function\n");
                GetFileStatRequest request;
                GetFileStatResponse reply;
                Status status;
                uint32_t retryCount = 0;

                request.set_pathname(path);
                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("GetFileStat: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->GetFileStat(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("GetFileStat: RPC Success\n");
                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("GetFileStat: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                    
                    stbuf->st_ino = reply.status().ino();
                    stbuf->st_mode = reply.status().mode();
                    stbuf->st_nlink = reply.status().nlink();
                    stbuf->st_uid = reply.status().uid();
                    stbuf->st_gid = reply.status().gid();
                    stbuf->st_size = reply.status().size();
                    stbuf->st_blksize = reply.status().blksize();
                    stbuf->st_blocks = reply.status().blocks();
                    stbuf->st_atime = reply.status().atime();
                    stbuf->st_mtime = reply.status().mtime();
                    stbuf->st_ctime = reply.status().ctime();
                    dbgprintf("GetFileStat: Exiting function\n");
                    return 0;
                } 
                else
                {
                    dbgprintf("GetFileStat: RPC Failed\n");
                    dbgprintf("GetFileStat: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }
            }

            int DeleteFile(std::string path) 
            {
                dbgprintf("DeleteFile: Entered function\n");
                RemoveRequest request;
                RemoveResponse reply;
                Status status;
                uint32_t retryCount = 0;
                
                request.set_pathname(path);
                
                // Make RPC 
                // Retry with backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("DeleteFile: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->Remove(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE );
                

                // Checking RPC Status 
                if (status.ok()) 
                {
                    dbgprintf("DeleteFile: RPC success\n");
                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("DeleteFile: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                    
                    dbgprintf("DeleteFile: Exiting function\n");

                    // remove from local cache
                    if (FileExists(get_cache_path(path)))
                    {
                        unlink(get_cache_path(path).c_str());
                    }

                    return 0;
                } 
                else
                {
                    dbgprintf("DeleteFile: RPC failure\n");
                    dbgprintf("DeleteFile: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }                
            }

            // TODO - deal with path having hierarchy -- need to call mkdir (eg. open(a/b/c.txt), where a/ and b/ not present locally)
            // TODO - how to deal w creat request
            int OpenFile(std::string path) 
            {
                dbgprintf("OpenFile: Inside function\n");
                int file;
                FetchRequest request;
                FetchResponse reply;
                Status status;
                //struct utimbuf ubuf;
                uint32_t retryCount = 0;
            
                // Note: TestAuth will internally call get_cache_path
                auto test_auth_result = TestAuth(path);
                
                if (!test_auth_result.status.ok() || test_auth_result.response.has_changed())
                {  
                    #if DEBUG
                    if(test_auth_result.status.ok()) {
                        dbgprintf("OpenFile: TestAuth reports changed\n");
                    } else {
                        dbgprintf("OpenFile: TestAuth RPC failed\n");
                    }
                    #endif
                    
                    request.set_pathname(path);

                    // Make RPC
                    // Retry with backoff
                    do 
                    {
                        ClientContext context;
                        reply.Clear();
                        dbgprintf("OpenFile: Invoking RPC\n");
                        sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                        status = stub_->Fetch(&context, request, &reply);
                        retryCount++;
                    } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE );
            
                    // Checking RPC Status
                    if (status.ok()) 
                    {
                        dbgprintf("OpenFile: RPC Success\n");
                        uint server_errno = reply.fs_errno();
                        if(server_errno) {
                            dbgprintf("...but error %d on server\n", server_errno);
                            dbgprintf("OpenFile: Exiting function\n");
                            errno = server_errno;
                            return -1;
                        }
                        

                        // create directory tree if not exists, as it exists on the server
                        if (create_path(path, true) != 0)
                        {
                            dbgprintf("create_path: failed\n");
                            return -1;
                        }

                        dbgprintf("OpenFile: create_path() Success\n");

                        file = open(get_cache_path(path).c_str(), O_RDWR | O_TRUNC | O_CREAT, 0666);
                        if (file == -1)
                        {
                            dbgprintf("OpenFile: open() failed\n");
                            return -1;
                        }
                    
                        //dbgprintf("OpenFile: reply.file_contents().length() = %ld\n", reply.file_contents().length());
                    
                        if (write(file, reply.file_contents().c_str(), reply.file_contents().length()) == -1)
                        {
                            dbgprintf("OpenFile: write() failed\n");
                            return -1;
                        }
                            
                        auto timing = reply.time_modify();
                        
                        struct timespec t;
                        t.tv_sec = timing.sec();
                        t.tv_nsec = timing.nsec();
                        
                        if(set_timings_opened(file,t) == -1) {
                            dbgprintf("OpenFile: error (%d) setting file timings\n",errno);
                        } else {
                            dbgprintf("OpenFile: updated file timings\n");
                        }
                        
                        if (fsync(file) == -1)
                        {
                            dbgprintf("OpenFile: fsync() failed\n");
                            return -1;
                        }
                        
                        if (close(file) == -1)
                        {
                            dbgprintf("OpenFile: close() failed\n");
                            return -1;
                        }
                        
                        dbgprintf("OpenFile: successfully wrote %d bytes to cache\n", (int)reply.file_contents().length()); 
                    } 
                    else 
                    {
                        dbgprintf("OpenFile: RPC Failure\n");
                        errno = transform_rpc_err(status.error_code());
                        return -1;
                    }
                } else {
                    dbgprintf("OpenFile: TestAuth reports no change\n");
                }

                file = open(get_cache_path(path).c_str(), O_RDWR | O_CREAT, 0666); // QUESTION: Why do we need O_CREAT?
                if (file == -1)
                {
                    dbgprintf("OpenFile: open() failed\n");
                    return -1;
                }
                
                dbgprintf("OpenFile: Exiting function\n");
                return file;
            }

            int CloseFile(int fd, string path) 
            {
                dbgprintf("CloseFile: Entered function\n");
                StoreRequest request;
                StoreResponse reply;
                Status status;
                uint32_t retryCount = 0;
                
	            // Check for init closes (fd = -1) and skip to RPC call
                if (fd != -1)
                    if (close(fd))
                    {
                        dbgprintf("CloseFile: close() failed\n");
                        return -1;
                    }

                if (SINGLE_LOG)
                {
                    if (!checkModified_single_log(fd, path)) return 0;
                }
                else
                {
                    if(!isFileModifiedv2(path)) return 0;
                }
                   

                // Set request
                const string cache_path = get_cache_path(path);
                
                request.set_pathname(path);
                request.set_file_contents(readFileIntoString(cache_path));

                // Make RPC
                // Retry with backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("CloseFile: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->Store(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("CloseFile: RPC Success\n");

                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("CloseFile: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                    
		            if (SINGLE_LOG) closeEntry_single_log(path);
                    else removePendingFile(to_flat_file(path));

                    auto timing = reply.time_modify();
                    
                    struct timespec t;
                    t.tv_sec = timing.sec();
                    t.tv_nsec = timing.nsec();
                    
                    if(set_timings(cache_path,t) == -1) {
                        dbgprintf("CloseFile: error (%d) setting file timings\n",errno);
                    } else {
                        dbgprintf("CloseFile: updated file timings\n");
                    }
                    
                    dbgprintf("CloseFile: Exiting function\n");
                    return 0;
                } 
                else 
                {
                    dbgprintf("CloseFile: RPC Failure\n");
                    dbgprintf("CloseFile: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }
            }

            /*
            * When called on Fetch,
            * TestAuth first checks if local file exits
            * if it does not exist, TestAuth returns true
            * 
            * For Fetch and Store,
            * TestAuth gets the modify time of the local file
            * and invokes the TestAuth RPC
            * if the modify times of the local and server file
            * do not match, TestAuth returns true, else false
            *
            * Error handling for GetModifyTime:
            * For eg. on Store, if the file does not exist
            * TestAuth returns false
            */
            TestAuthReturnType TestAuth(std::string path)
            {
                dbgprintf("TestAuth: Entering function\n");
                TestAuthRequest request;
                TestAuthResponse reply;
                Timestamp t;
                timespec modifyTime;
                Status status;
                uint32_t retryCount = 0;

                // Check if local file exists
                if (!FileExists(get_cache_path(path)))
                {
                    dbgprintf("TestAuth: Local file does not exist\n");
                    reply.set_has_changed(true);
                    return TestAuthReturnType(status, reply);
                }
                
                // Get local modified time
                if (GetModifyTime(get_cache_path(path), &modifyTime) != 0)
                {
                    dbgprintf("TestAuth: Exiting function\n");
                    reply.set_has_changed(false); // TO CHECK: Should we return true or false in this case?
                    return TestAuthReturnType(status, reply);
                }

                // Set Request
                request.set_pathname(path);
                t.set_sec(modifyTime.tv_sec);
                t.set_nsec(modifyTime.tv_nsec);
                request.mutable_time_modify()->CopyFrom(t);

                // Make RPC
                // Retry w backoff
                do 
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("TestAuth: Invoking RPC\n");
                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                    status = stub_->TestAuth(&context, request, &reply);
                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);

                if (status.ok()) 
                {
                    dbgprintf("TestAuth: RPC Success\n");
                }
                else
                {
                    dbgprintf("TestAuth: RPC Failure\n");
                }

                dbgprintf("TestAuth: Exiting function\n");
                return TestAuthReturnType(status, reply);
            }

            string get_cache_path(string relative_path)
            {
                return LOCAL_CACHE_PREFIX + relative_path;
            }

            // For Performance
            // TODO - add log
            int CloseFileWithStream(int fd, string path) 
            {
                dbgprintf("CloseFileWithStream: Entered function\n");
                if (close(fd) == -1)
                {
                    dbgprintf("CloseFileWithStream: close() failed\n");
                    return -1;
                }
            
                StoreRequest request;
                StoreResponse reply;
                Status status;
                uint32_t retryCount = 0;

                string cache_path = get_cache_path(path);
                
                do
                {
                    ClientContext context;
                    reply.Clear();
                    dbgprintf("CloseFileWithStream: Invoking RPC\n");

                    // No RPC necessary if file wasn't modified
                    // TODO: IsFileModified
                

                    sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);

                    std::unique_ptr<ClientWriter<StoreRequest>> writer(
                        stub_->StoreWithStream(&context, &reply));
                
                    // Set Request
                    request.set_pathname(path);
                    std::ifstream fin(cache_path.c_str(), std::ios::binary);
                    fin.clear();
                    fin.seekg(0, ios::beg);


                    // Get total chunks
                    struct stat st;
                    stat(get_cache_path(path).c_str(), &st);
                    int fileSize = st.st_size;
                    int totalChunks = 0;
                    totalChunks = fileSize / CHUNK_SIZE;
                    bool aligned = true;
                    int lastChunkSize = CHUNK_SIZE;
                    if (fileSize % CHUNK_SIZE)
                    {
                        totalChunks++; // for left overs
                        aligned = false;
                        lastChunkSize = fileSize % CHUNK_SIZE;
                    }
                
                    dbgprintf("CloseFileWithStream: fileSize = %d\n", fileSize);
                    dbgprintf("CloseFileWithStream: totalChunks = %d\n", totalChunks);
                    dbgprintf("CloseFileWithStream: lastChunkSize = %d\n", lastChunkSize);

                    // Read and send as stream
                    for (size_t chunk = 0; chunk < totalChunks; chunk++)
                    {
                        size_t currentChunkSize = (chunk == totalChunks - 1 && !aligned) ? 
                                                lastChunkSize : CHUNK_SIZE;
                        
                        //char * buffer = new char [currentChunkSize + 1];
                        char * buffer = new char [currentChunkSize];
                        //buffer[currentChunkSize] = '\0';

                        dbgprintf("CloseFileWithStream: Reading chunks\n");
                        if (fin.read(buffer, currentChunkSize)) 
                        {
                            request.set_file_contents(buffer);
                            //dbgprintf("buffer = %s\n", buffer); -- do not use if \0 not set at end

                            if (!writer->Write(request)) 
                            {
                                // Broken stream.
                                dbgprintf("CloseFileWithStream: Stream broke\n");
                                break; // TODO: Should we ret errno here?
                            }
                        }
                    }
                    fin.close();
                    // Done w Stream
                    writer->WritesDone();
                    status = writer->Finish();

                    retryCount++;
                } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);
                
                // Checking RPC Status
                if (status.ok()) 
                {
                    dbgprintf("CloseFileWithStream: RPC Success\n");
                    uint server_errno = reply.fs_errno();
                    if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                        dbgprintf("CloseFileWithStream: Exiting function\n"); 
                        errno = server_errno;
                            return -1;
                    }
                    
                    auto timing = reply.time_modify();
                    
                    struct timespec t;
                    t.tv_sec = timing.sec();
                    t.tv_nsec = timing.nsec();
                    
                    if(set_timings(cache_path,t) == -1) {
                        dbgprintf("CloseFileWithStream: error (%d) setting file timings\n",errno);
                    } else {
                        dbgprintf("CloseFileWithStream: updated file timings\n");
                    }
                    
                    dbgprintf("CloseFileWithStream: Exiting function\n");
                    return 0;
                } 
                else 
                {
                    dbgprintf("CloseFileWithStream: RPC Failure\n");
                    dbgprintf("CloseFileWithStream: Exiting function\n");
                    errno = transform_rpc_err(status.error_code());
                        return -1;
                }
            } 

            // For Performance
            int OpenFileWithStream(std::string path) 
            {
                dbgprintf("OpenFileWithStream: Inside function\n");
                int file;
                FetchRequest request;
                FetchResponse reply;
                Status status;
                ClientContext context;
                //struct utimbuf ubuf;
                uint32_t retryCount = 0;
                string cache_path = get_cache_path(path);
            
                // Note: TestAuth will internally call get_cache_path
                if (TestAuth(path).response.has_changed())
                {  
                    do
                    {
                        dbgprintf("OpenFileWithStream: Invoking RPC\n");
                        request.set_pathname(path);
                        
                        sleep(RETRY_TIME_START * retryCount * RETRY_TIME_MULTIPLIER);
                        
                        std::unique_ptr<ClientReader<FetchResponse>> reader(
                            stub_->FetchWithStream(&context, request));
                        
                        std::ofstream file;
                        file.open(cache_path, std::ios::binary); // TODO Check flags
                        file.clear();
                        file.seekp(0, ios::beg);

                        while (reader->Read(&reply))
                        {
                            file << reply.file_contents();
                        }

                        Status status = reader->Finish();
                        file.close();
                        retryCount++;
                    } while (retryCount < MAX_RETRY && status.error_code() == StatusCode::UNAVAILABLE);
                    

                    // Checking RPC Status
                    if (status.ok()) 
                    {
                        dbgprintf("OpenFileWithStream: RPC Success\n");
                        uint server_errno = reply.fs_errno();
                        if(server_errno) {
                        dbgprintf("...but error %d on server\n", server_errno);
                            errno = server_errno;
                            return -1;
                        }
                        
                        auto timing = reply.time_modify();
                        
                        struct timespec t;
                        t.tv_sec = timing.sec();
                        t.tv_nsec = timing.nsec();
                        
                        if(set_timings(cache_path,t) == -1) {
                            dbgprintf("CloseFileWithStream: error (%d) setting file timings\n",errno);
                        } else {
                            dbgprintf("CloseFileWithStream: updated file timings\n");
                        }
                        
                    } 
                    else 
                    {
                        dbgprintf("OpenFileWithStream: RPC Failure\n");
                        errno = transform_rpc_err(status.error_code());
                        return -1;
                    }
                } 

                file = open(get_cache_path(path).c_str(), O_RDWR | O_CREAT, 0666); // QUESTION: Why do we need O_CREAT?
                if (file == -1)
                {
                    dbgprintf("OpenFile: open() failed\n");
                    return -1;
                }
                
                dbgprintf("OpenFile: Exiting function\n");
                return file;
            }

            // For Crash Consistency
            // Log v2
            int removePendingFile(string filename)
            {
                dbgprintf("removePendingFile: Entering function\n");
                string command = "rm -f " + filename;
                dbgprintf("removePendingFile: command %s\n", command.c_str());
                dbgprintf("removePendingFile: Exiting function\n");
                return system(command.c_str());
            }

            // For Crash Consistency
            // Log v2
            string to_flat_file(string relative_path)
            {
                dbgprintf("to_flat_file: Entering function\n");
                for (int i=0; i<relative_path.length(); i++)
                {
                    if (relative_path[i] == '/') {
                        relative_path[i] = '%';
                    }
                }
                string flat_file = LOCAL_CACHE_PREFIX + relative_path + ".tmp"; 
                dbgprintf("to_flat_file: Exiting function\n");
                return flat_file;
            }

            // For Crash Consistency
            // Log v2
            string from_flat_file(string absolute_path)
            {
                dbgprintf("from_flat_file: Entering function\n");

                int i = 0;
                while (i++<string(LOCAL_CACHE_PREFIX).length());

                for (; i<absolute_path.length(); i++)
                {
                    if (absolute_path[i] == '%') {
                        absolute_path[i] = '/';
                    }
                }
                dbgprintf("from_flat_file: Exiting function\n");
                return absolute_path;
            }
                
        private:
            unique_ptr<FileSystemService::Stub> stub_;

            int set_timings(string cache_path, timespec t) {
                struct timespec p[2] = {t,t};
                return utimensat(AT_FDCWD,cache_path.c_str(),p,0);
            }

            int set_timings_opened(int fd, timespec t) {
                struct timespec p[2] = {t,t};
                return futimens(fd,p);
            }
            
            int create_path(string relative_path, bool is_file)
            {
                dbgprintf("create_path: Entering function\n");
                vector<string> tokens = tokenize_path(relative_path, '/', is_file);
                string base_path = LOCAL_CACHE_PREFIX;
                for (auto token : tokens)
                {
                    base_path += token + "/";

                    struct stat s;
                    int r = stat(base_path.c_str(), &s);
                    if (r != 0 && errno == 2) 
                    {
                        dbgprintf("create_path: stat() ENOENT\n");
                        if (mkdir(base_path.c_str(), S_IRWXU) != 0)
                        {
                            dbgprintf("create_path: mkdir() failed : %d\n", errno);
                            return -1;
                        }
                    }
                    else if (r != 0)
                    {
                        dbgprintf("create_path: stat() failed : %d\n", errno);
                        return -1;
                    }
                        
                }
                dbgprintf("create_path: Exiting function\n");
                return 0;
            }

            vector<string> tokenize_path(string path, char delim, bool is_file)
            {
                vector<string> tokens;
                string temp = "";
                
                for(int i = 0; i < path.length(); i++)
                {
                    if(path[i] == delim){
                        tokens.push_back(temp);
                        temp = "";
                    }
                    else
                        temp = temp + (path.c_str())[i];           
                }

                // if path is dir not file, create all dirs in path
                if (!is_file)
                    tokens.push_back(temp);

                return tokens;
            }

            bool FileExists(std::string path)
            {
                struct stat s;   
                return (stat(path.c_str(), &s) == 0); 
            }

            uint32_t GetModifyTime(std::string filepath, timespec * t) 
            {
                dbgprintf("GetModifyTime: Entering function\n");
                struct stat sb;
                if (stat(filepath.c_str(), &sb) == -1) 
                {
                    dbgprintf("GetModifyTime: Failed\n");
                    return -1;
                }
                dbgprintf("GetModifyTime: Exiting function\n");
                *t = sb.st_mtim;
                return 0;
            }

            string readFileIntoString(string path) 
            {
                ifstream input_file(path);
                if (!input_file.is_open()) 
                {
                    dbgprintf("readFileIntoString(): failed\n");
                    return string();
                }
                return string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
            }

            // For Crash Consistency
            // Log v1
            int checkModified_single_log(int fd, string path) {
                ifstream log;
                bool changed = false;
                log.open("/tmp/afs/log", ios::in);
                if (log.is_open()) {
                    string line;
                    while (getline(log, line)) {
                        if (line == path)
                        changed = true;
                    }
                }
                if (!changed) return 0; 
                
                return 1;
            }

            // For Crash Consistency
            // Log v1
            void closeEntry_single_log(string path) {
                // Delete entry from log
                ifstream log;
                log.open("/tmp/afs/log", ios::in);
                ofstream newlog;
                newlog.open("/tmp/afs/newlog", ios::out);
                if (log.is_open() && newlog.is_open()) {
                    string line;
                
                    while (getline(log, line)) {	
                        if (line != path) {
                            newlog << line << endl;     
                    }
                }
                    remove("/tmp/afs/log");
                    // If we crash here, we lose the log
                    rename("/tmp/afs/newlog", "/tmp/afs/log");
                }
            }

            // For Crash Consistency
            // Log v2
            bool isFileModifiedv2(string rel_path)
            {
                return FileExists(to_flat_file(rel_path));
            }
    };
}

