#include <bits/stdc++.h>
#include <grpcpp/grpcpp.h>
#include <sys/stat.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "filesystemcomm.grpc.pb.h"

/******************************************************************************
 * Macros
 *****************************************************************************/
#define DEBUG 1
#define dbgprintf(...)       \
    if (DEBUG) {             \
        printf(__VA_ARGS__); \
    }
#define errprintf(...) \
    { printf(__VA_ARGS__); }

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
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;
using std::cout;
using std::endl;
using std::string;
using fs::path;

class ServiceException : public std::runtime_error {
    StatusCode code;

   public:
    ServiceException(const char* msg, StatusCode code) : std::runtime_error(msg), code(code) {}

    StatusCode get_code() const {
        return code;
    }
};

static const string TEMP_FILE_EXT = ".afs_tmp";

// Server Implementation
class ServiceImplementation final : public FileSystemService::Service {
    path root;

    path to_storage_path(string relative) {
        path normalized = (root / relative).lexically_normal();

        // Check that this path is under our storage root
        auto [a, b] = std::mismatch(root.begin(), root.end(), normalized.begin());
        if (a != root.end()) {
            throw ServiceException("Normalized path is outside storage root", StatusCode::INVALID_ARGUMENT);
        }

        if (normalized.extension() == TEMP_FILE_EXT) {
            throw ServiceException("Attempting to access file with a reserved extension", StatusCode::INVALID_ARGUMENT);
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
                switch (ec.value()) {
                    case ENOENT:
                        throw new ServiceException("File not found", StatusCode::NOT_FOUND);
                    case ENOTDIR:
                        throw new ServiceException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
                    default:
                        throw new ServiceException("Error checking file type", StatusCode::UNKNOWN);
                }
            } else {
                throw new ServiceException("Attempting to read non-file item ", StatusCode::FAILED_PRECONDITION);
            }
        }

        std::ifstream file(filepath, std::ios::in | std::ios::binary);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        
        if (file.fail()) {
            dbgprintf("read_file: Exiting function\n");
            throw new ServiceException("Error performing file read", StatusCode::UNKNOWN);
        }

        dbgprintf("read_file: Exiting function\n");
        return buffer.str();
    }
    
    
    // Check that file content can be written to a given path.
    // The path must be either:
    // (a) An existing regular file, or
    // (b) An empty location in an existing directory.
    void assert_valid_write_destination(path filepath) {
        std::error_code ec;
        auto status = fs::status(filepath, ec);
        switch(status.type()){
            case fs::file_type::regular:
                return; // result (a)
            case fs::file_type::directory:
                throw new ServiceException("Attempting to write file to location of directory", StatusCode::FAILED_PRECONDITION);
            case fs::file_type::not_found:
                break;
            default:
                throw new ServiceException("Attempting to write file to location of non-file item", StatusCode::FAILED_PRECONDITION);
        }
        
        // type was not_found, so check the error code
        switch(ec.value()) {
            case ENOTDIR:
                throw new ServiceException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
            case ENOENT:
                break;
            default:
                throw new ServiceException("Error checking file status", StatusCode::UNKNOWN);
        }
        
        // error code was ENOENT, so check that parent directory exists
        if(!fs::is_directory(filepath.parent_path(), ec)) {
            throw new ServiceException("Parent directory does not exist", StatusCode::NOT_FOUND);
        }
        
        // result (b)
    }

    void write_file(path filepath, string content) {
        dbgprintf("write_file: Entering function\n");
        
        std::ofstream file;
        
        // Check that this is a valid destination
        assert_valid_write_destination(filepath);
        
        path temppath = get_tempfile_path(filepath);
        
        // Write to temp path
        file.open(temppath, std::ios::binary);
        file << content;
        file.close();
        
        // Typical failure cases should be handled by the validity check above,
        // so failure to write the temp file means something unexpected has happened.
        if(file.fail()) {
            dbgprintf("write_file: Exiting function\n");
            throw new ServiceException("Error writing temp file", StatusCode::UNKNOWN);
        }
        
        // Overwrite temp file with new file
        if(rename(temppath.c_str(), filepath.c_str()) == -1) {
            dbgprintf("write_file: Exiting function\n");
            throw new ServiceException("Error committing temp file", StatusCode::UNKNOWN);
        }
        
        dbgprintf("write_file: Exiting function\n");
    }

    void move_file(path srcpath, path dstpath) {
        dbgprintf("move_file: Entering function\n");

        if (fs::exists(dstpath)) {
            throw new ServiceException("Attempting to rename item to existing item", StatusCode::FAILED_PRECONDITION);
        }

        if (rename(srcpath.c_str(), dstpath.c_str()) == -1) {
            dbgprintf("move_file: Exiting function\n");
            switch (errno) {
                /* These cases shouldn't happen due to our exists() check
                case EISDIR:
                    throw new ServiceException("Attempting to rename file to existing directory", StatusCode::FAILED_PRECONDITION);
                case ENOTDIR:
                    throw new ServiceException("Attempting to rename directory to existing file", StatusCode::FAILED_PRECONDITION);
                */
                case ENOENT:
                    throw new ServiceException("File not found", StatusCode::NOT_FOUND);
                case EINVAL:
                    throw new ServiceException("Attempting to rename directory to child of itself", StatusCode::INVALID_ARGUMENT);
                default:
                    throw new ServiceException("Error in call to rename", StatusCode::UNKNOWN);
            }
        }
        dbgprintf("move_file: Exiting function\n");
    }

    void delete_file(path filepath) {
        dbgprintf("delete_file: Entering function\n");
        if (unlink(filepath.c_str()) == -1) {
            dbgprintf("delete_file: Exiting function\n");
            switch (errno) {
                case EISDIR:
                case EPERM:
                    throw new ServiceException("Called Remove on directory", StatusCode::PERMISSION_DENIED);
                case ENOENT:
                    throw new ServiceException("File not found", StatusCode::NOT_FOUND);
                default:
                    throw new ServiceException("Error in call to unlink", StatusCode::UNKNOWN);
            }
        }
        dbgprintf("delete_file: Exiting function\n");
    }

    void make_dir(path filepath) {
        dbgprintf("make_dir: Entering function\n");
        if (mkdir(filepath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
            dbgprintf("make_dir: Exiting function\n");
            switch (errno) {
                case EEXIST:
                    throw new ServiceException("Path already exists", StatusCode::ALREADY_EXISTS);
                case ENOENT:
                    throw new ServiceException("Missing directory in path prefix", StatusCode::NOT_FOUND);
                case ENOTDIR:
                    throw new ServiceException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
                default:
                    throw new ServiceException("Error in call to mkdir", StatusCode::UNKNOWN);
            }
        }
        dbgprintf("make_dir: Exiting function\n");
    }

    void remove_dir(path filepath) {
        if (rmdir(filepath.c_str()) == -1) {
            switch (errno) {
                case EEXIST:
                case ENOTEMPTY:
                    throw new ServiceException("Attempting to remove non-empty directory", StatusCode::FAILED_PRECONDITION);
                case EINVAL:
                    throw new ServiceException("Invalid directory name", StatusCode::INVALID_ARGUMENT);
                case ENOENT:
                    throw new ServiceException("Missing directory in path", StatusCode::NOT_FOUND);
                case ENOTDIR:
                    throw new ServiceException("Non-directory in path", StatusCode::FAILED_PRECONDITION);
                default:
                    throw new ServiceException("Error in call to mkdir", StatusCode::UNKNOWN);
            }
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

        if (stat(filepath.c_str(), &sb) == -1) {
            switch (errno) {
                case ENOENT:
                    throw new ServiceException("Item not found", StatusCode::NOT_FOUND);
                case ENOTDIR:
                    throw new ServiceException("Non-directory in path prefix", StatusCode::FAILED_PRECONDITION);
                default:
                    throw new ServiceException("Error in call to stat", StatusCode::UNKNOWN);
            }
        }

        FileStat ret;

        ret.set_file_name(filepath.filename());
        ret.set_mode(convert_filemode(sb.st_mode));
        ret.set_size(sb.st_size);

        Timestamp t_access = convert_timestamp(sb.st_atim);
        Timestamp t_change = convert_timestamp(sb.st_mtim);
        Timestamp t_modify = convert_timestamp(sb.st_ctim);

        ret.mutable_time_access()->CopyFrom(t_access);
        ret.mutable_time_change()->CopyFrom(t_change);
        ret.mutable_time_modify()->CopyFrom(t_modify);

        return ret;
    }

    timespec read_modify_time(path filepath) {
        struct stat sb;
        if (stat(filepath.c_str(), &sb) == -1) {
            // TODO transform error codes
            throw ServiceException("Stat failed", StatusCode::UNKNOWN);
        }
        return sb.st_mtim;
    }

    mode_t read_file_mode(path filepath) {
        struct stat sb;
        if (stat(filepath.c_str(), &sb) == -1) {
            // TODO transform error codes
            throw ServiceException("Stat failed", StatusCode::UNKNOWN);
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

   public:
    ServiceImplementation(path root) : root(root) {}

    Status Fetch(ServerContext* context, const FetchRequest* request, FetchResponse* reply) override {
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("Fetch: filepath = %s\n", filepath.c_str());

            // TODO wait for read/write lock

            // In C++, protobuf `bytes` fields are implemented as strings
            auto content = read_file(filepath);
            reply->set_file_contents(content);
            reply->mutable_time_modify()->CopyFrom(
                convert_timestamp(read_modify_time(filepath)));

            return Status::OK;
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            return Status(e.get_code(), e.what());
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

            auto time = convert_timestamp(read_modify_time(filepath));
            reply->mutable_time_modify()->CopyFrom(time);
            dbgprintf("Store: Exiting function on Success path\n");
            return Status::OK;

        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Store: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
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
            dbgprintf("Remove: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Remove: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
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
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("Rename: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
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
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            return Status(e.get_code(), e.what());
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
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("TestAuth: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
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
            make_dir(filepath);
            dbgprintf("MakeDir: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("MakeDir: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("MakeDir: Exiting function on Exception path\n");
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
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("RemoveDir: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("RemoveDir: Exiting function on Exception path\n");
            return Status(StatusCode::UNKNOWN, e.what());
        }
    }

    Status ListDir(ServerContext* context, const ListDirRequest* request, ListDirResponse* reply) override {
        dbgprintf("ListDir: Entering function\n");
        try {
            path filepath = to_storage_path(request->pathname());
            dbgprintf("ListDir: filepath = %s\n", filepath.c_str());

            // todo wait for write to finish??
            list_dir(filepath, reply);
            dbgprintf("ListDir: Exiting function on Success path\n");
            return Status::OK;
        } catch (const ServiceException& e) {
            dbgprintf("[Service Exception: %d] %s\n", e.get_code(), e.what());
            dbgprintf("ListDir: Exiting function on ServiceException path\n");
            return Status(e.get_code(), e.what());
        } catch (const std::exception& e) {
            errprintf("[Unexpected Exception] %s\n", e.what());
            dbgprintf("ListDir: Exiting function on Exception path\n");
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