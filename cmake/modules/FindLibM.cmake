# Finding libmath
# The module defines
# LibM_INCLUDE_DIRS - the libm include directories
# LibM_LIBRARIES - the libm library

include(LibFindMacros)

# get hints about paths
libfind_pkg_check_modules(LibM_PKGCONF libm)

# headers
find_path(LibM_INCLUDE_DIR
    NAMES "math.h"
    PATHS ${LibM_PKGCONF_INCLUDE_DIRS}
    )

# library
find_library(LibM_LIBRARY
    NAMES m
    PATHS ${LibM_PKGCONF_LIBRARY_DIRS}
    )

set(LibM_PROCESS_LIBS LibM_LIBRARY)
set(LibM_PROCESS_INCLUDES LibM_INCLUDE_DIR)
libfind_process(LibM)

