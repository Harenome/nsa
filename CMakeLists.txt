cmake_minimum_required(VERSION 2.8)
project(NSA)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

set(NSA_VERSION_MAJOR 0)
set(NSA_VERSION_MINOR 0)
set(NSA_VERSION_PATCH 1)

if(NOT CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    # cmake adds -g automatically
    set(DEBUG ON)
    message(STATUS "Debug mode active")
    #set(NSA_DEBUG_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/debug)
    #file(MAKE_DIRECTORY ${NSA_DEBUG_DIRECTORY})
else()
    set(DEBUG OFF)
endif()

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
file(MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}")

set(AUTOGEN_OUTPUT_PATH "${CMAKE_BINARY_DIR}/autogen")
file(MAKE_DIRECTORY "${AUTOGEN_OUTPUT_PATH}")
# Search for MPI

find_package(MPI REQUIRED)
include_directories("${MPI_C_INCLUDE_PATH}")

find_package(LibM REQUIRED)
include_directories("${LibM_INCLUDE_DIRS}")

find_package(LibGMP REQUIRED)
include_directories("${LibGMP_INCLUDE_DIRS}")

# config file

find_program(WHOAMI_PROG whoami)
find_program(HOSTNAME_PROG hostname)
find_program(DATE_PROG date)

if(NOT DEFINED USERNAME AND EXISTS "${WHOAMI_PROG}")
    execute_process(COMMAND "${WHOAMI_PROG}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE USERNAME)
else()
    set(USERNAME "H&M")
endif()
if(NOT DEFINED HOSTNAME AND EXISTS "${HOSTNAME_PROG}")
    execute_process(COMMAND "${HOSTNAME_PROG}"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE HOSTNAME)
else()
    set(HOSTNAME "NSA")
endif()

if(NOT DEFINED DATE AND EXISTS "${DATE_PROG}")
    execute_process(COMMAND "${DATE_PROG}" "+%F"
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE DATE)
else()
    set(DATE "1-1-1970")
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/config/version.h.in"
    "${AUTOGEN_OUTPUT_PATH}/include/nsa/version.h")


include_directories("${AUTOGEN_OUTPUT_PATH}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_definitions(-std=gnu11)

if(DEBUG)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -pedantic -Wfloat-equal -Wswitch-default -Winit-self -Wshadow -Wbad-function-cast -Wcast-align -Wconversion -Wlogical-op -Wstrict-prototypes -Wnested-externs")
endif()

# Executable

add_executable(nsa
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nsa/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nsa/eratostene.c"
    )
target_link_libraries(nsa
    "${MPI_C_LIBRARIES}"
    "${LibM_LIBRARIES}"
    "${LibGMP_LIBRARIES}"
    )

#add_executable(prof
#    "${CMAKE_CURRENT_SOURCE_DIR}/src/nsa/factoint28inter.c"
#    )
#target_link_libraries(prof
#    "${MPI_C_LIBRARIES}"
#    "${LibM_LIBRARIES}"
#    "${LibGMP_LIBRARIES}"
#    )


# Install
#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/man/man1/nsa.1"
#    DESTINATION "share/man/man1"
#    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
#    )

install(TARGETS nsa
    DESTINATION "bin"
    )

# Uninstall

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
    )

add_custom_target(uninstall
    COMMAND "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
