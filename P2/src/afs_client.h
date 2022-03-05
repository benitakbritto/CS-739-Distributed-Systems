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
#define DEBUG                 1
#define dbgprintf(...)        if (DEBUG) { printf(__VA_ARGS__); }
#define MAX_RETRY             5
#define RETRY_TIME_START      1 // seconds
#define RETRY_TIME_MULTIPLIER 2
#define LOCAL_CACHE_PREFIX    "/tmp/afs/"
#define CHUNK_SIZE            1024

//Remove to disable all crashes
#define CRASH_TEST

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

/******************************************************************************
 * GLOBALS
 *****************************************************************************/
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

/******************************************************************************
 * gRPC SYNC CLIENT IMPLEMENTATION
 *****************************************************************************/
namespace FileSystemClient
{
    class ClientImplementation 
    {
        public:
            ClientImplementation(std::shared_ptr<Channel> channel)
                : stub_(FileSystemService::NewStub(channel)) {}
        
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
                    
                    // making the dir in cache directory, hierarchical if necessary
                    create_path(path, false);
                    return 0;
                }
                else
                {
                    dbgprintf("MakeDir: RPC failure\n");
                    dbgprintf("MakeDir: Exiting function\n");
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
                if (status.ok() && reply.error() == 0) 
                {
                    dbgprintf("CreateFile: RPC Success\n");
                    dbgprintf("CreateFile: Exiting function\n");

                    // adding file to local cache
                    mknod(get_cache_path(path).c_str(), mode, rdev);
                    return 0;
                } 
                else 
                {
                    dbgprintf("CreateFile: RPC Failure\n");
                    dbgprintf("CreateFile: Exiting function\n");
                    return reply.error();
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

                    // removing directory from cache
                    rmdir(get_cache_path(path).c_str());
                    return 0;

                } 
                else 
                {
                    dbgprintf("RemoveDir: RPC Failure\n");
                    dbgprintf("RemoveDir: Exiting function\n");
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
                } 
                else 
                {
                    dbgprintf("ListDir: RPC Failure\n");
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
                if (status.ok() && reply.status().error() == 0) 
                {
                    dbgprintf("GetFileStat: RPC Success\n");
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
                    return -reply.status().error();
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
                    return -1;
                }                
            }

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
                if (TestAuth(path).response.has_changed())
                {  
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

                        // create directory tree if not exists, as it exists on the server
                        if (create_path(path, true) != 0)
                        {
                            dbgprintf("create_path: failed\n");
                            return errno;
                        }

                        dbgprintf("OpenFile: create_path() Success\n");

                        file = open(get_cache_path(path).c_str(), O_RDWR | O_TRUNC | O_CREAT, 0666);
                        if (file == -1)
                        {
                            dbgprintf("OpenFile: open() failed\n");
                            return errno;
                        }
                    
                        //dbgprintf("OpenFile: reply.file_contents().length() = %ld\n", reply.file_contents().length());
                    
                        if (write(file, reply.file_contents().c_str(), reply.file_contents().length()) == -1)
                        {
                            dbgprintf("OpenFile: write() failed\n");
                            return errno;
                        }
                        
                        if (fsync(file) == -1)
                        {
                            dbgprintf("OpenFile: fsync() failed\n");
                            return errno;
                        }

                        if (close(file) == -1)
                        {
                            dbgprintf("OpenFile: close() failed\n");
                            return errno;
                        }
                    } 
                    else 
                    {
                        dbgprintf("OpenFile: RPC Failure\n");
                        return -1;
                    }
                } 

                file = open(get_cache_path(path).c_str(), O_RDWR | O_CREAT, 0666); // QUESTION: Why do we need O_CREAT?
                if (file == -1)
                {
                    dbgprintf("OpenFile: open() failed\n");
                    return errno;
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

                // No RPC necessary if file wasn't modified
                
                // TODO: Add isFileModified here
                
                // Set request
                request.set_pathname(path);
                request.set_file_contents(readFileIntoString(get_cache_path(path)));

                if (close(fd))
                {
                    dbgprintf("CloseFile: close() failed\n");
                    return -1;
                }

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
                    dbgprintf("CloseFile: Exiting function\n");
                    return 0;
                } 
                else 
                {
                    dbgprintf("CloseFile: RPC Failure\n");
                    dbgprintf("CloseFile: Exiting function\n");
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
            // TODO - retry
            int CloseFileWithStream(int fd, string path) 
            {
                dbgprintf("CloseFileWithStream: Entered function\n");
                if (close(fd) == -1)
                {
                    dbgprintf("CloseFileWithStream: close() failed\n");
                    return errno;
                }
            
                StoreRequest request;
                StoreResponse reply;
                Status status;
                uint32_t retryCount = 0;

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
                    std::ifstream fin(get_cache_path(path).c_str(), std::ios::binary);
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
                    dbgprintf("CloseFileWithStream: Exiting function\n");
                    return 0;
                } 
                else 
                {
                    dbgprintf("CloseFileWithStream: RPC Failure\n");
                    dbgprintf("CloseFileWithStream: Exiting function\n");
                    return -1;
                }
            } 

            // For Performance
            // TODO - retry
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
                        file.open(get_cache_path(path), std::ios::binary); // TODO Check flags
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
                    } 
                    else 
                    {
                        dbgprintf("OpenFileWithStream: RPC Failure\n");
                        return -1;
                    }
                } 

                file = open(get_cache_path(path).c_str(), O_RDWR | O_CREAT, 0666); // QUESTION: Why do we need O_CREAT?
                if (file == -1)
                {
                    dbgprintf("OpenFile: open() failed\n");
                    return errno;
                }
                
                dbgprintf("OpenFile: Exiting function\n");
                return file;
            }
                
        private:
            unique_ptr<FileSystemService::Stub> stub_;

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
                            return errno;
                        }
                    }
                    else if (r != 0)
                    {
                        dbgprintf("create_path: stat() failed : %d\n", errno);
                        return errno;
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
            // Log v2
            // TODO invoke
            int createPendingFile(string filename)
            {
                string command = "touch " + filename + ".tmp";
                dbgprintf("createPendingFile: command %s\n", command.c_str());
                return system(command.c_str());
            }

            // For Crash Consistency
            // Log v2
            // TODO invoke
            int removePendingFile(string filename)
            {
                string command = "rm -f " + filename + ".tmp";
                dbgprintf("removePendingFile: command %s\n", command.c_str());
                return system(command.c_str());
            }

            // For Crash Consistency
            // Log v2
            // TODO invoke
            bool isFileModifiedv2(string filename)
            {
                return FileExists(filename);
            }
    };
}

