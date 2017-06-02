cmake_minimum_required(VERSION 2.8)

project (libbson)

option(ENABLE_TESTS "Build libbson tests." ON)

include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckStructHasMember)
include(CheckSymbolExists)
include(TestBigEndian)
include(InstallRequiredSystemLibraries)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/build/cmake)

# Set BSON_MAJOR_VERSION, BSON_MINOR_VERSION, etc.
include(LoadVersion)
LoadVersion(${PROJECT_SOURCE_DIR}/VERSION_CURRENT BSON)
LoadVersion(${PROJECT_SOURCE_DIR}/VERSION_RELEASED BSON_RELEASED)

message("Current version (from VERSION_CURRENT file): ${BSON_VERSION}")
if (NOT ${BSON_VERSION} STREQUAL ${BSON_RELEASED_VERSION})
   message("Previous release (from VERSION_RELEASED file): ${BSON_RELEASED_VERSION}")
endif()

set (SOURCE_DIR "${PROJECT_SOURCE_DIR}/")

set (BSON_API_VERSION 1.0)

set (CPACK_RESOURCE_FILE_LICENSE "${SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR ${BSON_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${BSON_MINOR_VERSION})

include (CPack)

TEST_BIG_ENDIAN(BSON_BIG_ENDIAN)

# These need proper checks like automake.
set (BSON_PTHREAD_ONCE_INIT_NEEDS_BRACES 0)

#librt needed on linux for clock_gettime
find_library(RT_LIBRARY rt)
if (RT_LIBRARY)
   #set required libraries for CHECK_FUNCTION_EXISTS
   set(CMAKE_REQUIRED_LIBRARIES ${RT_LIBRARY})
endif()

# See https://public.kitware.com/Bug/view.php?id=15659
CHECK_SYMBOL_EXISTS(snprintf stdio.h BSON_HAVE_SNPRINTF)
if (NOT BSON_HAVE_SNPRINTF)
  set(BSON_HAVE_SNPRINTF 0)
else ()
  set(BSON_HAVE_SNPRINTF 1)
endif ()

CHECK_FUNCTION_EXISTS(_set_output_format BSON_NEEDS_SET_OUTPUT_FORMAT)
if (NOT BSON_NEEDS_SET_OUTPUT_FORMAT)
  set(BSON_NEEDS_SET_OUTPUT_FORMAT 0)
endif ()

CHECK_STRUCT_HAS_MEMBER("struct timespec" tv_sec time.h BSON_HAVE_TIMESPEC)
if (NOT BSON_HAVE_TIMESPEC)
  message(STATUS "    no timespec struct")
  set(BSON_HAVE_TIMESPEC 0)
else ()
  message(STATUS "    struct timespec found")
  set(BSON_HAVE_TIMESPEC 1)
endif ()

if (MSVC)
   set (BSON_OS 2)
   set (BSON_HAVE_CLOCK_GETTIME 0)
   set (BSON_HAVE_STDBOOL_H 0)
   set (BSON_HAVE_STRNLEN 0)
   set (BSON_EXTRA_ALIGN 1)
else ()
   find_package (Threads)

   set (BSON_OS 1)
   CHECK_FUNCTION_EXISTS(clock_gettime BSON_HAVE_CLOCK_GETTIME)
   if (NOT BSON_HAVE_CLOCK_GETTIME)
      set(BSON_HAVE_CLOCK_GETTIME 0)
   endif ()
   CHECK_FUNCTION_EXISTS(strnlen BSON_HAVE_STRNLEN)
   if (NOT BSON_HAVE_STRNLEN)
      set(BSON_HAVE_STRNLEN 0)
   endif ()
   CHECK_INCLUDE_FILE(stdbool.h BSON_HAVE_STDBOOL_H)
   if (NOT BSON_HAVE_STDBOOL_H)
      set(BSON_HAVE_STDBOOL_H 0)
   endif ()
   set (BSON_EXTRA_ALIGN 1)
endif ()

set (BSON_HAVE_ATOMIC_32_ADD_AND_FETCH 1)
set (BSON_HAVE_ATOMIC_64_ADD_AND_FETCH 1)

if (BSON_BIG_ENDIAN)
   set (BSON_BYTE_ORDER 4321)
else ()
   set (BSON_BYTE_ORDER 1234)
endif ()

configure_file (
   "${SOURCE_DIR}/src/bson/bson-config.h.in"
   "${PROJECT_BINARY_DIR}/src/bson/bson-config.h"
)

configure_file (
   "${SOURCE_DIR}/src/bson/bson-version.h.in"
   "${PROJECT_BINARY_DIR}/src/bson/bson-version.h"
)

configure_file (
   "${PROJECT_SOURCE_DIR}/build/cmake/bson/bson-stdint.h"
   "${PROJECT_BINARY_DIR}/src/bson/bson-stdint.h"
)

include_directories("${PROJECT_BINARY_DIR}/src/bson")
include_directories("${SOURCE_DIR}/src/bson")
include_directories("${SOURCE_DIR}/src")

if (APPLE)
   cmake_policy(SET CMP0042 OLD)
endif()

set (SOURCES
   ${SOURCE_DIR}/src/bson/bcon.c
   ${SOURCE_DIR}/src/bson/bson.c
   ${SOURCE_DIR}/src/bson/bson-atomic.c
   ${SOURCE_DIR}/src/bson/bson-clock.c
   ${SOURCE_DIR}/src/bson/bson-context.c
   ${SOURCE_DIR}/src/bson/bson-error.c
   ${SOURCE_DIR}/src/bson/bson-iso8601.c
   ${SOURCE_DIR}/src/bson/bson-iter.c
   ${SOURCE_DIR}/src/bson/bson-json.c
   ${SOURCE_DIR}/src/bson/bson-keys.c
   ${SOURCE_DIR}/src/bson/bson-md5.c
   ${SOURCE_DIR}/src/bson/bson-memory.c
   ${SOURCE_DIR}/src/bson/bson-oid.c
   ${SOURCE_DIR}/src/bson/bson-reader.c
   ${SOURCE_DIR}/src/bson/bson-string.c
   ${SOURCE_DIR}/src/bson/bson-timegm.c
   ${SOURCE_DIR}/src/bson/bson-utf8.c
   ${SOURCE_DIR}/src/bson/bson-value.c
   ${SOURCE_DIR}/src/bson/bson-version-functions.c
   ${SOURCE_DIR}/src/bson/bson-writer.c
   ${SOURCE_DIR}/src/yajl/yajl_alloc.c
   ${SOURCE_DIR}/src/yajl/yajl_buf.c
   ${SOURCE_DIR}/src/yajl/yajl.c
   ${SOURCE_DIR}/src/yajl/yajl_encode.c
   ${SOURCE_DIR}/src/yajl/yajl_gen.c
   ${SOURCE_DIR}/src/yajl/yajl_lex.c
   ${SOURCE_DIR}/src/yajl/yajl_parser.c
   ${SOURCE_DIR}/src/yajl/yajl_tree.c
   ${SOURCE_DIR}/src/yajl/yajl_version.c
)

set (HEADERS
   ${PROJECT_BINARY_DIR}/src/bson/bson-config.h
   ${PROJECT_BINARY_DIR}/src/bson/bson-stdint.h
   ${PROJECT_BINARY_DIR}/src/bson/bson-version.h
   ${SOURCE_DIR}/src/bson/bcon.h
   ${SOURCE_DIR}/src/bson/bson-atomic.h
   ${SOURCE_DIR}/src/bson/bson-clock.h
   ${SOURCE_DIR}/src/bson/bson-compat.h
   ${SOURCE_DIR}/src/bson/bson-context.h
   ${SOURCE_DIR}/src/bson/bson-endian.h
   ${SOURCE_DIR}/src/bson/bson-error.h
   ${SOURCE_DIR}/src/bson/bson.h
   ${SOURCE_DIR}/src/bson/bson-iter.h
   ${SOURCE_DIR}/src/bson/bson-json.h
   ${SOURCE_DIR}/src/bson/bson-keys.h
   ${SOURCE_DIR}/src/bson/bson-macros.h
   ${SOURCE_DIR}/src/bson/bson-md5.h
   ${SOURCE_DIR}/src/bson/bson-memory.h
   ${SOURCE_DIR}/src/bson/bson-oid.h
   ${SOURCE_DIR}/src/bson/bson-reader.h
   ${SOURCE_DIR}/src/bson/bson-stdint-win32.h
   ${SOURCE_DIR}/src/bson/bson-string.h
   ${SOURCE_DIR}/src/bson/bson-types.h
   ${SOURCE_DIR}/src/bson/bson-utf8.h
   ${SOURCE_DIR}/src/bson/bson-value.h
   ${SOURCE_DIR}/src/bson/bson-version-functions.h
   ${SOURCE_DIR}/src/bson/bson-writer.h
)

if (MSVC)
   set(BSON_SHARED_SOURCES ${SOURCES} ${PROJECT_SOURCE_DIR}/build/cmake/libbson.def)
else()
   set(BSON_SHARED_SOURCES ${SOURCES})
endif()

add_library(bson_shared SHARED ${BSON_SHARED_SOURCES} ${HEADERS})
add_library(bson_static STATIC ${SOURCES} ${HEADERS})

set_target_properties(bson_shared PROPERTIES COMPILE_DEFINITIONS "BSON_COMPILATION")
set_target_properties(bson_static PROPERTIES COMPILE_DEFINITIONS "BSON_COMPILATION")

set_target_properties(bson_shared PROPERTIES VERSION ${BSON_VERSION} SOVERSION ${BSON_MAJOR_VERSION})
set_target_properties(bson_static PROPERTIES VERSION ${BSON_VERSION} SOVERSION ${BSON_MAJOR_VERSION})

set_target_properties(bson_shared PROPERTIES OUTPUT_NAME "bson-${BSON_API_VERSION}" PREFIX "lib")
set_target_properties(bson_static PROPERTIES OUTPUT_NAME "bson-static-${BSON_API_VERSION}")

if (RT_LIBRARY)
    target_link_libraries (bson_shared ${RT_LIBRARY})
    target_link_libraries (bson_static ${RT_LIBRARY})
endif()

if (UNIX)
    target_link_libraries (bson_shared ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries (bson_static ${CMAKE_THREAD_LIBS_INIT})
else()
    target_link_libraries (bson_shared ws2_32)
    target_link_libraries (bson_static ws2_32)
endif()

if (ENABLE_TESTS)
    enable_testing()

    add_executable(test-libbson
       ${SOURCE_DIR}/tests/TestSuite.c
       ${SOURCE_DIR}/tests/TestSuite.h
       ${SOURCE_DIR}/tests/test-libbson.c
       ${SOURCE_DIR}/tests/test-atomic.c
       ${SOURCE_DIR}/tests/test-bson.c
       ${SOURCE_DIR}/tests/test-endian.c
       ${SOURCE_DIR}/tests/test-clock.c
       ${SOURCE_DIR}/tests/test-error.c
       ${SOURCE_DIR}/tests/test-iso8601.c
       ${SOURCE_DIR}/tests/test-iter.c
       ${SOURCE_DIR}/tests/test-json.c
       ${SOURCE_DIR}/tests/test-oid.c
       ${SOURCE_DIR}/tests/test-reader.c
       ${SOURCE_DIR}/tests/test-string.c
       ${SOURCE_DIR}/tests/test-utf8.c
       ${SOURCE_DIR}/tests/test-value.c
       ${SOURCE_DIR}/tests/test-version.c
       ${SOURCE_DIR}/tests/test-writer.c
       ${SOURCE_DIR}/tests/test-bcon-basic.c
       ${SOURCE_DIR}/tests/test-bcon-extract.c
    )

    target_link_libraries(test-libbson bson_static)
    add_test(NAME test-libbson COMMAND test-libbson)

    file(COPY ${SOURCE_DIR}/tests/binary ${SOURCE_DIR}/tests/json
         DESTINATION ${PROJECT_BINARY_DIR}/tests)
endif ()  # ENABLE_TESTS

install(
  TARGETS bson_shared bson_static
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(
  FILES ${HEADERS}
  DESTINATION "include/libbson-${BSON_API_VERSION}"
)

set(VERSION "${BSON_VERSION}")
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(libdir "\${prefix}/lib")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libbson-1.0.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libbson-1.0.pc
@ONLY)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libbson-1.0.pc
    DESTINATION lib/pkgconfig
)
