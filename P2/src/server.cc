#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <sys/stat.h>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <chrono>
#include <utility>
#include "filesystemcomm.grpc.pb.h"

/******************************************************************************
 * MACROS
 *****************************************************************************/
#define DEBUG 1
#define dbgprintf(...)       \
    if (DEBUG) {             \
        printf(__VA_ARGS__); \
    }
#define errprintf(...) \
    { printf(__VA_ARGS__); }

#define SMALL_FILE_SIZE_THRESHOLD 10000 // bytes
#define MEM_MAP_MAX_KEY_COUNT    100 // num keys in map
#define MEM_MAP_FREE_COUNT       10 // free 10 files
#define MEM_MAP_START_FREE       50 // start freeing map when keys have reached this count value
#define PERFORMANCE_MEM_MAP      0 // setting to read from mem map
#define CHUNK_SIZE               1024 // for streaming

/******************************************************************************
 * NAMESPACE
 *****************************************************************************/
namespace fs = std::filesystem;
using filesystemcomm::DirectoryEntry;
using filesystemcomm::FetchRequest;
using filesystemcomm::FetchResponse;
using filesystemcomm::FileMode;
using filesystemcomm::FileStat;
using filesystemcomm::FileSystemService;
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
using filesystemcomm::Timestamp;
using fs::path;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using std::cout;
using std::endl;
using std::string;
using grpc::ServerReader;
using grpc::ServerWriter;


/******************************************************************************
 * GLOBALS
 *****************************************************************************/
// For Performance
// stores filepath, timestamp, content of insertion
// TODO: Add reader/writer locks
std::map<fs::path, std::pair<time_t, std::string>> mem_map; 
static const string TEMP_FILE_EXT = ".afs_tmp";


/******************************************************************************
 * EXCEPTION HANDLER
 *****************************************************************************/
class ProtocolException : public std::runtime_error {
    StatusCode code;

   public:
    ProtocolException(const char* msg, StatusCode code) : std::runtime_error(msg), code(code) {}

    StatusCode get_code() const {
        return code;
    }
};

class FileSystemException : public std::runtime_error {
    uint fs_errno;
    
    public:
    FileSystemException(uint fs_errno) : std::runtime_error("Error in filesystem call"), fs_errno(fs_errno) {}
    
    uint get_fs_errno() const {
        return fs_errno;
    }
};

/******************************************************************************
 * gRPC SYNC SERVER IMPLEMENTATION
 *****************************************************************************/
class ServiceImplementation final : public FileSystemService::Service {
    path root;

    path to_storage_path(string relative) {
    	dbgprintf("to_storage_path: root = %s\n", root.c_str());
	    dbgprintf("to_storage_path: relative = %s\n", relative.c_str());
	    path normalized = (root / relative).lexically_normal();
        dbgprintf("to_storage_path: normalized = %s\n", normalized.c_str());
        // Check that this path is under our storage root
        auto [a, b] = std::mismatch(root.begin(), root.end(), normalized.begin());
        if (a != root.end()) {
            throw ProtocolException("Normalized path is outside storage root", StatusCode::INVALID_ARGUMENT);
        }

        if (normalized.extension() == TEMP_FILE_EXT) {
            throw ProtocolException("Attempting to access file with a reserved extension", StatusCode::INVALID_ARGUMENT);
        }

        return normalized;
    }

    path get_tempfile_path(path filepath) {
        return path(filepath.string() + TEMP_FILE_EXT);
    }

    string read_file(path filepath) {
        dbgprintf("read_file: Entering function\n");

        // Check that path exists and is a file before proceeding
        std::error_code ec;
        if (!fs::is_regular_file(filepath, ec)) {
            dbgprintf("read_file: Exiting function\n");

            if (ec) {
                throw FileSystemException(ec.value());
                // switch (ec.value()) {
                //     case ENOENT:
                //         throw ProtocolException("File not found", StatusCode::NOT_FOUND);
                //     case ENOTDIR:
                //         throw ProtocolException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
                //     default:
                //         throw ProtocolException("Error checking file type", StatusCode::UNKNOWN);
                // }
            } else {
                throw FileSystemException(EISDIR);
                // throw ProtocolException("Attempting to read non-file item ", StatusCode::FAILED_PRECONDITION);
            }
        }

        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();

        if (file.fail()) {
            dbgprintf("read_file: Exiting function\n");
            throw ProtocolException("Error performing file read", StatusCode::UNKNOWN);
        }

        dbgprintf("read_file: Exiting function\n");
        return buffer.str();
    }

    // Check that file content can be written to a given path.
    // The path must be either:
    // (a) An existing regular file, or
    // (b) An empty location in an existing directory.
    uint check_valid_write_destination(path filepath) {
        std::error_code ec;
        auto status = fs::status(filepath, ec);
        switch (status.type()) {
            case fs::file_type::regular:
                return 0;
            case fs::file_type::directory:
                return EISDIR;
                //throw ProtocolException("Attempting to write file to location of directory", StatusCode::FAILED_PRECONDITION);
            case fs::file_type::not_found:
                break;
            default:
                return EPERM;
                //throw ProtocolException("Attempting to write file to location of non-file item", StatusCode::FAILED_PRECONDITION);
        }

        // type was not_found, so check the error code
        switch (ec.value()) {
            case ENOENT:
                break;
            // case ENOTDIR:
            //     return ENOTDIR;
            //     throw ProtocolException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
            default:
                return ec.value();
                // throw ProtocolException("Error checking file status", StatusCode::UNKNOWN);
        }

        // error code was ENOENT, so check that parent directory exists
        if (!fs::is_directory(filepath.parent_path(), ec)) {
            return ENOENT;
            // throw ProtocolException("Parent directory does not exist", StatusCode::NOT_FOUND);
        }

        // result (b)
        return 0;
    }

    void write_file(path filepath, string content) {
        dbgprintf("write_file: Entering function\n");

        std::ofstream file;

        // Check that this is a valid destination
        uint pre_err = check_valid_write_destination(filepath);
        if(pre_err != 0) {
            throw FileSystemException(pre_err);
        }

        path temppath = get_tempfile_path(filepath);

        // Write to temp path
        file.open(temppath, std::ios::binary);
        file << content;
        file.close();

        // Typical failure cases should be handled by the validity check above,
        // so failure to write the temp file means something unexpected has happened.
        if (file.fail()) {
            dbgprintf("write_file: Exiting function\n");
            throw ProtocolException("Error writing temp file", StatusCode::UNKNOWN);
        }

        // Overwrite temp file with new file
        if (rename(temppath.c_str(), filepath.c_str()) == -1) {
            dbgprintf("write_file: Exiting function\n");
            throw ProtocolException("Error committing temp file", StatusCode::UNKNOWN);
        }

        dbgprintf("write_file: Exiting function\n");
    }

    void move_file(path srcpath, path dstpath) {
        dbgprintf("move_file: Entering function\n");

        if (fs::exists(dstpath)) {
            dbgprintf("move_file: Exiting function\n");
            throw FileSystemException(EEXIST);
            // throw ProtocolException("Attempting to rename item to existing item", StatusCode::FAILED_PRECONDITION);
        }

        if (rename(srcpath.c_str(), dstpath.c_str()) == -1) {
            dbgprintf("move_file: Exiting function\n");
            throw FileSystemException(errno);
            // switch (errno) {
            //     /* These cases shouldn't happen due to our exists() check
            //     case EISDIR:
            //         throw ProtocolException("Attempting to rename file to existing directory", StatusCode::FAILED_PRECONDITION);
            //     case ENOTDIR:
            //         throw ProtocolException("Attempting to rename directory to existing file", StatusCode::FAILED_PRECONDITION);
            //     */
            //     case ENOENT:
            //         throw ProtocolException("File not found", StatusCode::NOT_FOUND);
            //     case EINVAL:
            //         throw ProtocolException("Attempting to rename directory to child of itself", StatusCode::INVALID_ARGUMENT);
            //     default:
            //         throw ProtocolException("Error in call to rename", StatusCode::UNKNOWN);
            // }
        }
        
        dbgprintf("move_file: Exiting function\n");
    }

    void delete_file(path filepath) {
        dbgprintf("delete_file: Entering function\n");
        if (unlink(filepath.c_str()) == -1) {
            dbgprintf("delete_file: Exiting function\n");
            throw FileSystemException(errno);
            // switch (errno) {
            //     case EISDIR:
            //     case EPERM:
            //         throw ProtocolException("Called Remove on directory", StatusCode::PERMISSION_DENIED);
            //     case ENOENT:
            //         throw ProtocolException("File not found", StatusCode::NOT_FOUND);
            //     default:
            //         throw ProtocolException("Error in call to unlink", StatusCode::UNKNOWN);
            // }
        }
        dbgprintf("delete_file: Exiting function\n");
    }

    void make_dir(path filepath, int mode) {
        dbgprintf("make_dir: Entering function\n");
        if (mkdir(filepath.c_str(), mode) == -1) {
            dbgprintf("make_dir: Exiting function\n");
            throw FileSystemException(errno);
            // switch (errno) {
            //     case EEXIST:
            //         throw ProtocolException("Path already exists", StatusCode::ALREADY_EXISTS);
            //     case ENOENT:
            //         throw ProtocolException("Missing directory in path prefix", StatusCode::NOT_FOUND);
            //     case ENOTDIR:
            //         throw ProtocolException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
            //     default:
            //         throw ProtocolException("Error in call to mkdir", StatusCode::UNKNOWN);
            // }
        }
        dbgprintf("make_dir: Exiting function\n");
    }

    void remove_dir(path filepath) {
        if (rmdir(filepath.c_str()) == -1) {
            throw FileSystemException(errno);
            // switch (errno) {
            //     case EEXIST:
            //     case ENOTEMPTY:
            //         throw ProtocolException("Attempting to remove non-empty directory", StatusCode::FAILED_PRECONDITION);
            //     case EINVAL:
            //         throw ProtocolException("Invalid directory name", StatusCode::INVALID_ARGUMENT);
            //     case ENOENT:
            //         throw ProtocolException("Missing directory in path", StatusCode::NOT_FOUND);
            //     case ENOTDIR:
            //         throw ProtocolException("Non-directory in path", StatusCode::FAILED_PRECONDITION);
            //     default:
            //         throw ProtocolException("Error in call to mkdir", StatusCode::UNKNOWN);
            // }
        }
    }

    void list_dir(path filepath, ListDirResponse* reply) {
        dbgprintf("list_dir: Entered function\n");
        // TODO catch errors from directory_iterator
        for (const auto& entry : fs::directory_iterator(filepath)) {
            DirectoryEntry* msg = reply->add_entries();
            msg->set_file_name(entry.path().filename());
            msg->set_mode(entry.is_regular_file() ? FileMode::REG : entry.is_directory() ? FileMode::DIR
                                                                                         : FileMode::UNSUPPORTED);
            msg->set_size(entry.is_regular_file() ? entry.file_size() : 0);
        }
        dbgprintf("list_dir: Exiting function\n");
    }

    FileStat read_stat(path filepath) {
        struct stat sb;
        FileStat ret;

        if (stat(filepath.c_str(), &sb) == -1) {
            dbgprintf("read_stat: failed\n");
            dbgprintf("read_stat: errno = %d\n", errno);
            throw FileSystemException(errno);
            // ret.set_error(errno);
            // return ret;
            // not needed
            // switch (errno) {
            //     case ENOENT:
            //         throw ProtocolException("Item not found", StatusCode::NOT_FOUND);
            //     case ENOTDIR:
            //         throw ProtocolException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
            //     default:
            //         throw ProtocolException("Error in call to stat", StatusCode::UNKNOWN);
            // }
        }

        
        ret.set_ino(sb.st_ino);
        ret.set_mode(sb.st_mode);
        ret.set_nlink(sb.st_nlink);
        ret.set_uid(sb.st_uid);
        ret.set_gid(sb.st_gid);
        ret.set_size(sb.st_size);
        ret.set_blksize(sb.st_blksize);
        ret.set_blocks(sb.st_blocks);
        ret.set_atime(sb.st_atime);
        ret.set_mtime(sb.st_mtime);
        ret.set_ctime(sb.st_ctime);
        // ret.set_error(0);

        return ret;
    }

    timespec read_modify_time(path filepath) {
        struct stat sb;
        if (stat(filepath.c_str(), &sb) == -1) {
            throw FileSystemException(errno);
            // // TODO transform error codes
            // throw ProtocolException("Stat failed", StatusCode::UNKNOWN);
        }
        return sb.st_mtim;
    }

    mode_t read_file_mode(path filepath) {
        struct stat sb;
        if (stat(filepath.c_str(), &sb) == -1) {
            throw FileSystemException(errno);
            // // TODO transform error codes
            // throw ProtocolException("Stat failed", StatusCode::UNKNOWN);
        }
        return sb.st_mode;
    }

    FileMode convert_filemode(mode_t raw) {
        switch (raw) {
            case S_IFDIR:
                return FileMode::DIR;
            case S_IFREG:
                return FileMode::REG;
            default:
                return FileMode::UNSUPPORTED;
        }
    }

    Timestamp convert_timestamp(timespec raw) {
        Timestamp a;
        a.set_sec(raw.tv_sec);
        a.set_nsec(raw.tv_nsec);
        return a;
    }

    auto read_file_size(path filepath)
    {
        struct stat sb;

        if (stat(filepath.c_str(), &sb) == -1) {
            throw FileSystemException(errno);
            
            // switch (errno) {
            //     case ENOENT:
            //         throw ProtocolException("Item not found", StatusCode::NOT_FOUND);
            //     case ENOTDIR:
            //         throw ProtocolException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
            //     default:
            //         throw ProtocolException("Error in call to stat", StatusCode::UNKNOWN);
            // }
        }

        return sb.st_size;
    }

    auto get_current_time()
    {
        return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }

    // For Performance
    void put_file_in_mem_map(path filepath, string content)
    {
        auto current_time = get_current_time();
        if (mem_map.count(filepath) == 0)
        {
            mem_map.insert(std::make_pair(filepath, std::make_pair(current_time, content)));
        }
        else
        {
            mem_map[filepath] = (std::make_pair(current_time, content));
        }
    }

    // For Performance
    auto get_file_from_mem_map(path filepath)
    {
        if (mem_map.count(filepath) == 0)
        {
            dbgprintf("get_file_from_mem_map: filepath does not exist in map\n");
            throw ProtocolException("Item not found", StatusCode::NOT_FOUND);
        }
        else
        {
            return mem_map[filepath].second;
        }
    }

    // For Performance
    void delete_file_from_mem_map(path filepath)
    {
        if (mem_map.count(filepath))
        {
            return;
        }
        else
        {
            mem_map.erase(filepath);
        }
    }

    // For Performance
    // TODO: Invoke using background daemon that keeps polling this function
    /*
        Frees up to MEM_MAP_FREE_COUNT old entries (based on timestamps)
        When we hit a watermark of MEM_MAP_START_FREE keys in the map
    */
    void partial_free_mem_map()
    {
        if (mem_map.size() > MEM_MAP_START_FREE)
        {
            uint32_t count = 0;
            for (std::map<path,std::pair<time_t, string>>::iterator it = mem_map.begin(); 
                it != mem_map.end() &&  count < MEM_MAP_FREE_COUNT; 
                ++it, count++)
            {
                mem_map.erase(it->first);
            }
        }
    }

   public:
    ServiceImplementation(path root) : root(root) {}

    Status Fetch(ServerContext* context, const FetchRequest* request, FetchResponse* reply) override {
        path filepath = to_storage_path(request->pathname());
        dbgprintf("Fetch: filepath = %s\n", filepath.c_str());

#if PERFORMANCE_MEM_MAP == 1
        try {
            auto content = get_file_from_mem_map(filepath);
            reply->set_file_contents(content);
            reply->mutable_time_modify()->CopyFrom(
                convert_timestamp(read_modify_time(filepath)));

            dbgprintf("Fetch: Read from mem map\n");
            return Status::OK;

        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            // do nothing
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            // do nothing
        }
#endif
        // reading from mem map failed, try to get from disk

        try {
            // TODO wait for read/write lock
            // In C++, protobuf `bytes` fields are implemented as strings
            auto content = read_file(filepath);
            reply->set_file_contents(content);
            reply->mutable_time_modify()->CopyFrom(
                convert_timestamp(read_modify_time(filepath)));

            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status Store(ServerContext* context, const StoreRequest* request, StoreResponse* reply) override {
        dbgprintf("Store: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("Store: filepath = %s\n", filepath.c_str());

            // TODO wait for read/write lock
            write_file(filepath, request->file_contents());

// if optimization turned on, write to map
#if PERFORMANCE_MEM_MAP == 1
            // if the file is small, put in mem map
            auto file_size = read_file_size(filepath);
            if (file_size < SMALL_FILE_SIZE_THRESHOLD)
            {
                dbgprintf("Store: Storing file in mem map\n");
                put_file_in_mem_map(filepath, request->file_contents());
            }
#endif

            auto time = convert_timestamp(read_modify_time(filepath));
            reply->mutable_time_modify()->CopyFrom(time);
            dbgprintf("Store: Exiting function on Success path\n");
            return Status::OK;

        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Store: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("Store: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status Remove(ServerContext* context, const RemoveRequest* request, RemoveResponse* reply) override {
        dbgprintf("Remove: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            cout << "Remove: filepath = " << filepath << endl;

            // TODO wait for read/write lock

            delete_file(filepath);
// if optimization turned on, delete from map
#if PERFORMANCE_MEM_MAP == 1
            delete_file_from_mem_map(filepath);
#endif
            dbgprintf("Remove: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Remove: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("Remove: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status Rename(ServerContext* context, const RenameRequest* request, RenameResponse* reply) override {
        dbgprintf("Rename: Entering function\n");
        try {
            path srcpath = to_storage_path(request->pathname());
            path dstpath = to_storage_path(srcpath.parent_path() / request->componentname());

            // TODO: check that dst is in same dir as src
            dbgprintf("Rename: srcpath = %s, dstpath = %s\n", srcpath.c_str(), dstpath.c_str());

            // TODO wait for read/write lock
            move_file(srcpath, dstpath);

            dbgprintf("Rename: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Rename: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("Rename: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status GetFileStat(ServerContext* context, const GetFileStatRequest* request, GetFileStatResponse* reply) override {
        dbgprintf("GetFileStat: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("GetFileStat: filepath = %s\n", filepath.c_str());

            // TODO wait for read/write lock
            auto status = read_stat(filepath);
            reply->mutable_status()->CopyFrom(status);
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status TestAuth(ServerContext* context, const TestAuthRequest* request, TestAuthResponse* reply) override {
        dbgprintf("TestAuth: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("TestAuth: filepath = %s\n", filepath.c_str());

            // todo wait for write to finish??

            auto ts0 = read_modify_time(filepath);
            auto ts1 = request->time_modify();
            bool changed = (ts0.tv_sec != ts1.sec()) || (ts0.tv_nsec != ts1.nsec());

            reply->set_has_changed(changed);
            dbgprintf("TestAuth: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("TestAuth: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            // Instead of reporting filesystem error, just invalidate the cache entry
            reply -> set_has_changed(true); 
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("TestAuth: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status MakeDir(ServerContext* context, const MakeDirRequest* request, MakeDirResponse* reply) override {
        dbgprintf("MakeDir: Entering function\n");

        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("TestAuth: filepath = %s\n", filepath.c_str());

            // todo wait for write to finish??
            make_dir(filepath, request->mode());
            dbgprintf("MakeDir: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("MakeDir: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("MakeDir: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status Mknod(ServerContext* context, const MknodRequest* request, MknodResponse* reply) override {
        dbgprintf("Mknod: Entering function\n");

        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("Mknod: filepath = %s\n", filepath.c_str());

            int ret = mknod(filepath.c_str(), request->mode(), request->dev());
            
            dbgprintf("Mknod: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Mknod: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("Mknod: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status RemoveDir(ServerContext* context, const RemoveDirRequest* request, RemoveDirResponse* reply) override {
        dbgprintf("RemoveDir: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("RemoveDir: filepath = %s\n", filepath.c_str());

            // todo wait for write to finish??
            remove_dir(filepath);

            dbgprintf("RemoveDir: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("RemoveDir: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("RemoveDir: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status ReadDir(ServerContext* context, const ListDirRequest* request, ListDirResponse* reply) override {
        dbgprintf("ListDir: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("ListDir: filepath = %s\n", filepath.c_str());

            // todo wait for write to finish??
            list_dir(filepath, reply);
            dbgprintf("ListDir: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("ListDir: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("ListDir: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    // For Performance
    Status StoreWithStream(ServerContext* context, ServerReader<StoreRequest> * reader, StoreResponse* reply) override {
        dbgprintf("StoreWithStream: Entering function\n");
        StoreRequest request;
        std::ofstream file;
        path filepath;
        int i = 0;
        try {
            while (reader->Read(&request))
            {
                filepath = to_storage_path(request.pathname());
                dbgprintf("StoreWithStream: filepath = %s\n", filepath.c_str());
                if (i == 0) file.open(filepath, std::ios::binary); // set path once
                i++;

                // TODO wait for read/write lock

                file << request.file_contents();                
            }

            file.close();

            auto time = convert_timestamp(read_modify_time(filepath));
            reply->mutable_time_modify()->CopyFrom(time);
            dbgprintf("StoreWithStream: Exiting function\n");

            return Status::OK; 
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Store: Exiting function on ProtocolException path\n");
            return Status(e.get_code(), e.what());
        } catch(const FileSystemException& e) {
            dbgprintf("[System Exception: %d]\n", e.get_fs_errno());
            reply -> set_fs_errno(e.get_fs_errno());
            return Status::OK;
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("Store: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    // For Performance
    Status FetchWithStream(ServerContext* context, const FetchRequest* request, ServerWriter<FetchResponse>* writer) override {
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("FetchWithStream: filepath = %s\n", filepath.c_str());

            // TODO wait for read/write lock

            FetchResponse reply;
            // Set response
            reply.mutable_time_modify()->CopyFrom(
                convert_timestamp(read_modify_time(filepath)));
            
            // Get total chunks
            std::ifstream fin(filepath.c_str(), std::ios::binary);

            struct stat st;
            stat(filepath.c_str(), &st);
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
                
            dbgprintf("FetchWithStream: fileSize = %d\n", fileSize);
            dbgprintf("FetchWithStream: totalChunks = %d\n", totalChunks);
            dbgprintf("FetchWithStream: lastChunkSize = %d\n", lastChunkSize);

            // Read and send as stream
            for (size_t chunk = 0; chunk < totalChunks; chunk++)
            {
                size_t currentChunkSize = (chunk == totalChunks - 1 && !aligned) ? 
                                              lastChunkSize : CHUNK_SIZE;
                    
                char * buffer = new char [currentChunkSize + 1];
                //char * buffer = new char [currentChunkSize];
                buffer[currentChunkSize + 1] = '\0';
                dbgprintf("CloseFileWithStream: Reading chunks\n");
                if (fin.read(buffer, currentChunkSize)) 
                {
                    //dbgprintf("buffer = %s\n", buffer); -- do not use if \0 not set at end
                    dbgprintf("FetchWithStream = buffer %s\n", buffer);
                    reply.set_file_contents(buffer);
                    writer->Write(reply);
                }
            }

            fin.close();
            return Status::OK;
        } catch (const ProtocolException& e) {
            dbgprintf("[Protocol Exception: %d] %s\n", e.get_code(), e.what());
            return Status(e.get_code(), e.what());
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }
    

};

void RunServer(path root) {
    ServiceImplementation service(root);
    ServerBuilder builder;

    string server_address("0.0.0.0:50051");
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    cout << "Server listening on port: " << server_address << endl;

    server->Wait();
}

/******************************************************************************
 * DRIVER
 *****************************************************************************/
int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Usage: " << argv[0] << " root_folder" << endl;
        return 1;
    }

    auto root = fs::canonical(argv[1]);

    cout << "Serving files from " << root << endl;

    RunServer(root);
    return 0;
}
