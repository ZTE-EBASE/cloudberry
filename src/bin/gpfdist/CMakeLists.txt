cmake_minimum_required(VERSION 3.12)
project(gpfdist)

set(GPDB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../)

set (CPPFLAGS "/MP /wd4996 /wd4018 /wd4090 /wd4102 /wd4244 /wd4267 /wd4273 /wd4715")
add_definitions("${CPPFLAGS}")

set(HEADER_DIRS
    ${GPDB_SRC_DIR}/src/include
    ${GPDB_SRC_DIR}/src/include/port
    ${GPDB_SRC_DIR}/src/include/port/win32
    ${GPDB_SRC_DIR}/src/include/port/win32_msvc
    ${GPDB_SRC_DIR}/src/backend
    ${GPDB_SRC_DIR}/src/port
)

set(APR_LIB "${CMAKE_PREFIX_PATH}/lib/libapr-1.lib")
set(APR_DLL "${CMAKE_PREFIX_PATH}/bin/libapr-1.dll")
set(EVENT_LIB "${CMAKE_PREFIX_PATH}/lib/event.lib")

#set include dirs
include_directories("${CMAKE_PREFIX_PATH}/include")
include_directories(${HEADER_DIRS})
link_directories(${CMAKE_INSTALL_PREFIX}/lib)

#set source files
add_executable(gpfdist gpfdist.c gpfdist_helper.c
    ${GPDB_SRC_DIR}/src/backend/utils/misc/fstream/gfile.c
    ${GPDB_SRC_DIR}/src/backend/utils/misc/fstream/fstream.c
    ${GPDB_SRC_DIR}/src/port/glob.c)

SET_SOURCE_FILES_PROPERTIES(gpfdist.c ${GPDB_SRC_DIR}/src/backend/utils/misc/fstream/gfile.c PROPERTIES COMPILE_DEFINITIONS "FRONTEND")

include(FindOpenSSL)
if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "openssl not found")
endif(OPENSSL_FOUND)

find_package(ZLIB REQUIRED)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "zlib not found")
endif(ZLIB_FOUND)

# Optional LZO2 support
option(WITH_LZO2 "Build with LZO2 compression support" OFF)
if (WITH_LZO2)
    find_library(LZO2_LIBRARY NAMES lzo2)
    find_path(LZO2_INCLUDE_DIR lzo/lzo1x.h)
    if (LZO2_LIBRARY AND LZO2_INCLUDE_DIR)
        include_directories(${LZO2_INCLUDE_DIR})
        add_definitions(-DHAVE_LIBLZO2)
    else ()
        message(FATAL_ERROR "lzo2 library not found")
    endif ()
endif ()

# Optional libhdfs3 support
option(WITH_LIBHDFS3 "Build with libhdfs3 HDFS access support" OFF)
if (WITH_LIBHDFS3)
    find_library(HDFS3_LIBRARY NAMES hdfs3)
    find_path(HDFS3_INCLUDE_DIR hdfs/hdfs.h)
    if (HDFS3_LIBRARY AND HDFS3_INCLUDE_DIR)
        include_directories(${HDFS3_INCLUDE_DIR})
        add_definitions(-DUSE_LIBHDFS3)
    else ()
        message(FATAL_ERROR "libhdfs3 library not found")
    endif ()
endif ()

#set lib dirs
set(GPFDIST_EXTRA_LIBS "")
if (WITH_LZO2)
    list(APPEND GPFDIST_EXTRA_LIBS ${LZO2_LIBRARY})
endif ()
if (WITH_LIBHDFS3)
    list(APPEND GPFDIST_EXTRA_LIBS ${HDFS3_LIBRARY})
endif ()

target_link_libraries(gpfdist ${ZLIB_LIBRARIES} ${APR_LIB} ${EVENT_LIB} ${OPENSSL_LIBRARIES} libpgport ws2_32 Crypt32 ${GPFDIST_EXTRA_LIBS})

set_target_properties(gpfdist PROPERTIES FOLDER bin)
install(TARGETS gpfdist DESTINATION bin)
install(FILES ${APR_DLL} DESTINATION bin)
