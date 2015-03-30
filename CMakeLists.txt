cmake_minimum_required(VERSION 2.8)

################################## PROJECT ##################################

project(NSA)
set(NSA_VERSION_MAJOR 0)
set(NSA_VERSION_MINOR 1)
set(NSA_VERSION_PATCH 1)


if(NOT CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Debug mode active")
endif()

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
file(MAKE_DIRECTORY "${EXECUTABLE_OUTPUT_PATH}")

set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
file(MAKE_DIRECTORY "${LIBRARY_OUTPUT_PATH}")

set(AUTOGEN_OUTPUT_PATH "${CMAKE_BINARY_DIR}/autogen")
file(MAKE_DIRECTORY "${AUTOGEN_OUTPUT_PATH}")

################################## PACKAGES ##################################

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

find_package(MPI REQUIRED)
include_directories("${MPI_C_INCLUDE_PATH}")

find_package(LibM REQUIRED)
include_directories("${LibM_INCLUDE_DIRS}")

find_package(LibGMP REQUIRED)
include_directories("${LibGMP_INCLUDE_DIRS}")

################################ CONFIG FILES ################################

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

################################## COMPILER ##################################


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -O3")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Wextra -pedantic -Wfloat-equal -Wswitch-default -Winit-self -Wshadow -Wbad-function-cast -Wcast-align -Wconversion -Wlogical-op -Wstrict-prototypes -Wnested-externs")
endif()

################################# EXECUTABLE #################################

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
add_executable(nsa
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nsa/main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nsa/eratostene.c"
    )
target_link_libraries(nsa
    "${MPI_C_LIBRARIES}"
    "${LibM_LIBRARIES}"
    "${LibGMP_LIBRARIES}"
    )

add_executable(factoint28inter
    "${CMAKE_CURRENT_SOURCE_DIR}/src/nsa/factoint28inter.c"
    )
target_link_libraries(factoint28inter
    "${MPI_C_LIBRARIES}"
    "${LibM_LIBRARIES}"
    "${LibGMP_LIBRARIES}"
    )

################################### DOXYGEN ###################################

find_package(Doxygen QUIET)

if(DEFINED DOXYGEN_FOUND)
    message(STATUS "Doxygen found, doc target available")
    set(DOXYGEN_DOXYFILE "${AUTOGEN_OUTPUT_PATH}/Doxyfile")
    set(DOXYGEN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/doc")
    set(DOXYGEN_INPUT_DIRS
        "${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include")
    set(DOXYGEN_OUTPUT_FILES
        "${DOXYGEN_OUTPUT_DIR}/html/index.xhtml"
        "${DOXYGEN_OUTPUT_DIR}/latex/Makefile"
        )

    if(CMAKE_BUILD_TYPE MATCHES Release)
        set(DOXYGEN_WARNINGS "NO")
    else()
        set(DOXYGEN_WARNINGS "YES")
    endif()

    file(MAKE_DIRECTORY "${DOXYGEN_OUTPUT_DIR}")

    if(DEFINED DOXYGEN_DOT_FOUND)
        message(STATUS "DOT found, generating Doxygen DOT graphics")
        set(DOXYGEN_HAVE_DOT "YES")
    else()
        set(DOXYGEN_HAVE_DOT "NO")
    endif()

    configure_file(
        "${PROJECT_SOURCE_DIR}/config/Doxyfile.in"
        "${AUTOGEN_OUTPUT_PATH}/Doxyfile"
        IMMEDIATE @ONLY
        )

    add_custom_command(OUTPUT ${DOXYGEN_OUTPUT_FILES}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_DOXYFILE}
        MAIN_DEPENDENCY ${DOXYGEN_DOXYFILE}
        COMMENT "Generating Doxygen doc"
        )
    add_custom_target(doc DEPENDS ${DOXYGEN_OUTPUT_FILES})

else()
    message(STATUS "Doxygen not found, doc target not available")
endif()

################################### INSTALL ###################################

#install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/man/man1/nsa.1"
#    DESTINATION "share/man/man1"
#    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
#    )

install(TARGETS nsa
    DESTINATION "bin"
    )

################################## UNINSTALL ##################################

configure_file(
    "${PROJECT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
    )

add_custom_target(uninstall
    COMMAND "${CMAKE_COMMAND}" -P
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

