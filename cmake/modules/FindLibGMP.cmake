# Finding libgmp
# The module defines
# LibGMP_INCLUDE_DIRS - the libgmp include directories
# LibGMP_LIBRARIES - the libgmp library

include(LibFindMacros)

# get hints about paths
libfind_pkg_check_modules(LibGMP_PKGCONF libm)

# headers
find_path(LibGMP_INCLUDE_DIR
    NAMES "gmp.h"
    PATHS ${LibGMP_PKGCONF_INCLUDE_DIRS}
    )

# library
find_library(LibGMP_LIBRARY
    NAMES gmp
    PATHS ${LibGMP_PKGCONF_LIBRARY_DIRS}
    )

set(LibGMP_PROCESS_LIBS LibGMP_LIBRARY)
set(LibGMP_PROCESS_INCLUDES LibGMP_INCLUDE_DIR)
libfind_process(LibGMP)

