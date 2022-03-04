FIND_PATH(FUSE_INCLUDE_DIR fuse.h
        /usr/local/include/osxfuse
        /usr/local/include
        /usr/include
        /home/benitakbritto/fuse-3.10.5/include
        )

FIND_LIBRARY(
        FUSE_LIBRARY
        NAMES libfuse libfuse3 fuse libfuse3.so
        PATHS /lib64 /lib /usr/lib64 /usr/lib /usr/local/lib64 /usr/local/lib /usr/lib/x86_64-linux-gnu  /home/benitakbritto/fuse-3.10.5/build/lib
        NO_CACHE
)

include("FindPackageHandleStandardArgs")
find_package_handle_standard_args("FUSE" DEFAULT_MSG
        FUSE_INCLUDE_DIR FUSE_LIBRARY)