cmake_minimum_required(VERSION 2.8)

project (libmongoc)

option(ENABLE_SSL
       "Use OpenSSL for TLS connections and SCRAM-SHA-1 authentication.\
NOTE: OpenSSL is required for authenticating to MongoDB 3.0 and later."
       ON)

option(ENABLE_SASL "Use Cyrus SASL library for Kerberos." ON)
option(ENABLE_TESTS "Build MongoDB C Driver tests." ON)
option(ENABLE_EXAMPLES "Build MongoDB C Driver examples." ON)
option(ENABLE_AUTOMATIC_INIT_AND_CLEANUP "Enable automatic init and cleanup (GCC only)" ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/build/cmake)

include(InstallRequiredSystemLibraries)
include(FindBSON REQUIRED)

if (ENABLE_SSL)
    include(FindOpenSSL)
endif ()
if (ENABLE_SSL AND OPENSSL_FOUND)
   set (MONGOC_ENABLE_SSL 1)
else ()
   set (MONGOC_ENABLE_SSL 0)
endif ()

if (ENABLE_SASL)
   include(FindSASL2)
endif ()
if (ENABLE_SASL AND SASL2_FOUND)
   set (MONGOC_ENABLE_SASL 1)
else ()
   set (MONGOC_HAVE_SASL_CLIENT_DONE 0)
   set (MONGOC_ENABLE_SASL 0)
endif ()

if (ENABLE_AUTOMATIC_INIT_AND_CLEANUP)
   set (MONGOC_NO_AUTOMATIC_GLOBALS 0)
else ()
   set (MONGOC_NO_AUTOMATIC_GLOBALS 1)
endif ()

set (SOURCE_DIR "${PROJECT_SOURCE_DIR}/")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/build/cmake)

# Set MONGOC_MAJOR_VERSION, MONGOC_MINOR_VERSION, etc.
include(LoadVersion)
LoadVersion(${PROJECT_SOURCE_DIR}/VERSION_CURRENT MONGOC)
LoadVersion(${PROJECT_SOURCE_DIR}/VERSION_RELEASED MONGOC_RELEASED)

message("Current version (from VERSION_CURRENT file): ${MONGOC_VERSION}")
if (NOT ${MONGOC_VERSION} STREQUAL ${MONGOC_RELEASED_VERSION})
   message("Previous release (from VERSION_RELEASED file): ${MONGOC_RELEASED_VERSION}")
endif()

set (SOURCE_DIR "${PROJECT_SOURCE_DIR}/")

set (MONGOC_API_VERSION 1.0)

set (CPACK_RESOURCE_FILE_LICENSE "${SOURCE_DIR}/COPYING")
set (CPACK_PACKAGE_VERSION_MAJOR ${MONGOC_MAJOR_VERSION})
set (CPACK_PACKAGE_VERSION_MINOR ${MONGOC_MINOR_VERSION})

include (CPack)

set (MONGOC_HAVE_WEAK_SYMBOLS 0)

configure_file (
   "${SOURCE_DIR}/src/mongoc/mongoc-config.h.in"
   "${PROJECT_BINARY_DIR}/src/mongoc/mongoc-config.h"
)

configure_file (
   "${SOURCE_DIR}/src/mongoc/mongoc-version.h.in"
   "${PROJECT_BINARY_DIR}/src/mongoc/mongoc-version.h"
)

include_directories("${PROJECT_BINARY_DIR}/src/mongoc")
include_directories("${SOURCE_DIR}/src/mongoc")
include_directories(${BSON_INCLUDE_DIR})

set(LIBS ${LIBS} ${BSON_LIBRARIES})

if (UNIX AND NOT APPLE)
   set(LIBS ${LIBS} rt)
endif()

if (APPLE)
   cmake_policy(SET CMP0042 OLD)
endif()

add_definitions(-D_GNU_SOURCE)
add_definitions(-D_BSD_SOURCE)
add_definitions("-DBINARY_DIR=\"${SOURCE_DIR}/tests/binary\"")
if (APPLE)
   # Until CDRIVER-520.
   add_definitions(-Wno-deprecated-declarations)
endif()

set (SOURCES
   ${SOURCE_DIR}/src/mongoc/mongoc-array.c
   ${SOURCE_DIR}/src/mongoc/mongoc-async.c
   ${SOURCE_DIR}/src/mongoc/mongoc-async-cmd.c
   ${SOURCE_DIR}/src/mongoc/mongoc-b64.c
   ${SOURCE_DIR}/src/mongoc/mongoc-buffer.c
   ${SOURCE_DIR}/src/mongoc/mongoc-bulk-operation.c
   ${SOURCE_DIR}/src/mongoc/mongoc-client.c
   ${SOURCE_DIR}/src/mongoc/mongoc-client-pool.c
   ${SOURCE_DIR}/src/mongoc/mongoc-cluster.c
   ${SOURCE_DIR}/src/mongoc/mongoc-collection.c
   ${SOURCE_DIR}/src/mongoc/mongoc-counters.c
   ${SOURCE_DIR}/src/mongoc/mongoc-cursor-array.c
   ${SOURCE_DIR}/src/mongoc/mongoc-cursor.c
   ${SOURCE_DIR}/src/mongoc/mongoc-cursor-array.c
   ${SOURCE_DIR}/src/mongoc/mongoc-cursor-cursorid.c
   ${SOURCE_DIR}/src/mongoc/mongoc-cursor-transform.c
   ${SOURCE_DIR}/src/mongoc/mongoc-database.c
   ${SOURCE_DIR}/src/mongoc/mongoc-find-and-modify.c
   ${SOURCE_DIR}/src/mongoc/mongoc-init.c
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs.c
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file.c
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file-list.c
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file-page.c
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file-list.c
   ${SOURCE_DIR}/src/mongoc/mongoc-host-list.c
   ${SOURCE_DIR}/src/mongoc/mongoc-index.c
   ${SOURCE_DIR}/src/mongoc/mongoc-init.c
   ${SOURCE_DIR}/src/mongoc/mongoc-list.c
   ${SOURCE_DIR}/src/mongoc/mongoc-log.c
   ${SOURCE_DIR}/src/mongoc/mongoc-matcher.c
   ${SOURCE_DIR}/src/mongoc/mongoc-matcher-op.c
   ${SOURCE_DIR}/src/mongoc/mongoc-memcmp.c
   ${SOURCE_DIR}/src/mongoc/mongoc-opcode.c
   ${SOURCE_DIR}/src/mongoc/mongoc-queue.c
   ${SOURCE_DIR}/src/mongoc/mongoc-read-concern.c
   ${SOURCE_DIR}/src/mongoc/mongoc-read-prefs.c
   ${SOURCE_DIR}/src/mongoc/mongoc-rpc.c
   ${SOURCE_DIR}/src/mongoc/mongoc-server-description.c
   ${SOURCE_DIR}/src/mongoc/mongoc-server-stream.c
   ${SOURCE_DIR}/src/mongoc/mongoc-set.c
   ${SOURCE_DIR}/src/mongoc/mongoc-socket.c
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-buffered.c
   ${SOURCE_DIR}/src/mongoc/mongoc-stream.c
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-buffered.c
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-file.c
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-gridfs.c
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-socket.c
   ${SOURCE_DIR}/src/mongoc/mongoc-topology.c
   ${SOURCE_DIR}/src/mongoc/mongoc-topology-description.c
   ${SOURCE_DIR}/src/mongoc/mongoc-topology-scanner.c
   ${SOURCE_DIR}/src/mongoc/mongoc-uri.c
   ${SOURCE_DIR}/src/mongoc/mongoc-util.c
   ${SOURCE_DIR}/src/mongoc/mongoc-version-functions.c
   ${SOURCE_DIR}/src/mongoc/mongoc-write-command.c
   ${SOURCE_DIR}/src/mongoc/mongoc-write-concern.c
)

set (HEADERS
   ${PROJECT_BINARY_DIR}/src/mongoc/mongoc-config.h
   ${PROJECT_BINARY_DIR}/src/mongoc/mongoc-version.h
   ${SOURCE_DIR}/src/mongoc/mongoc.h
   ${SOURCE_DIR}/src/mongoc/mongoc-bulk-operation.h
   ${SOURCE_DIR}/src/mongoc/mongoc-client.h
   ${SOURCE_DIR}/src/mongoc/mongoc-client-pool.h
   ${SOURCE_DIR}/src/mongoc/mongoc-collection.h
   ${SOURCE_DIR}/src/mongoc/mongoc-cursor.h
   ${SOURCE_DIR}/src/mongoc/mongoc-database.h
   ${SOURCE_DIR}/src/mongoc/mongoc-error.h
   ${SOURCE_DIR}/src/mongoc/mongoc-flags.h
   ${SOURCE_DIR}/src/mongoc/mongoc-find-and-modify.h
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs.h
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file.h
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file-page.h
   ${SOURCE_DIR}/src/mongoc/mongoc-gridfs-file-list.h
   ${SOURCE_DIR}/src/mongoc/mongoc-host-list.h
   ${SOURCE_DIR}/src/mongoc/mongoc-init.h
   ${SOURCE_DIR}/src/mongoc/mongoc-index.h
   ${SOURCE_DIR}/src/mongoc/mongoc-iovec.h
   ${SOURCE_DIR}/src/mongoc/mongoc-log.h
   ${SOURCE_DIR}/src/mongoc/mongoc-matcher.h
   ${SOURCE_DIR}/src/mongoc/mongoc-opcode.h
   ${SOURCE_DIR}/src/mongoc/mongoc-opcode-private.h
   ${SOURCE_DIR}/src/mongoc/mongoc-read-concern.h
   ${SOURCE_DIR}/src/mongoc/mongoc-read-prefs.h
   ${SOURCE_DIR}/src/mongoc/mongoc-server-description.h
   ${SOURCE_DIR}/src/mongoc/mongoc-socket.h
   ${SOURCE_DIR}/src/mongoc/mongoc-socket-private.h
   ${SOURCE_DIR}/src/mongoc/mongoc-stream.h
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-buffered.h
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-file.h
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-gridfs.h
   ${SOURCE_DIR}/src/mongoc/mongoc-stream-socket.h
   ${SOURCE_DIR}/src/mongoc/mongoc-uri.h
   ${SOURCE_DIR}/src/mongoc/mongoc-version-functions.h
   ${SOURCE_DIR}/src/mongoc/mongoc-write-concern.h
)

if (OPENSSL_FOUND)
   set (HEADERS ${HEADERS}
      ${SOURCE_DIR}/src/mongoc/mongoc-rand.h
      ${SOURCE_DIR}/src/mongoc/mongoc-stream-tls.h
      ${SOURCE_DIR}/src/mongoc/mongoc-ssl.h)
   set (SOURCES ${SOURCES}
      ${SOURCE_DIR}/src/mongoc/mongoc-rand.c
      ${SOURCE_DIR}/src/mongoc/mongoc-scram.c
      ${SOURCE_DIR}/src/mongoc/mongoc-stream-tls.c
      ${SOURCE_DIR}/src/mongoc/mongoc-ssl.c
   )
   set(LIBS ${LIBS} ${OPENSSL_LIBRARIES})
   include_directories(${OPENSSL_INCLUDE_DIR})
endif()

if (MONGOC_ENABLE_SASL)
   set (HEADERS ${HEADERS} ${SOURCE_DIR}/src/mongoc/mongoc-sasl-private.h)
   set (SOURCES ${SOURCES} ${SOURCE_DIR}/src/mongoc/mongoc-sasl.c)
   set(LIBS ${LIBS} ${SASL2_LIBRARY})
   include_directories(${SASL2_INCLUDE_DIR})
endif()

if (MSVC)
   if (MONGOC_ENABLE_SSL)
      set(MONGOC_SHARED_SOURCES ${SOURCES} ${PROJECT_SOURCE_DIR}/build/cmake/libmongoc-ssl.def)
   else ()
      set(MONGOC_SHARED_SOURCES ${SOURCES} ${PROJECT_SOURCE_DIR}/build/cmake/libmongoc.def)
   endif()
else()
   set(MONGOC_SHARED_SOURCES ${SOURCES})
endif()

add_library(mongoc_shared SHARED ${MONGOC_SHARED_SOURCES} ${HEADERS})
add_library(mongoc_static STATIC ${SOURCES} ${HEADERS})

target_link_libraries(mongoc_shared ${LIBS})
target_link_libraries(mongoc_static ${LIBS})

set_target_properties(mongoc_shared PROPERTIES COMPILE_DEFINITIONS "MONGOC_COMPILATION")
set_target_properties(mongoc_static PROPERTIES COMPILE_DEFINITIONS "MONGOC_COMPILATION")

set_target_properties(mongoc_shared PROPERTIES VERSION ${MONGOC_VERSION} SOVERSION ${MONGOC_MAJOR_VERSION})
set_target_properties(mongoc_static PROPERTIES VERSION ${MONGOC_VERSION} SOVERSION ${MONGOC_MAJOR_VERSION})

set_target_properties(mongoc_shared PROPERTIES OUTPUT_NAME "mongoc-${MONGOC_API_VERSION}" PREFIX "lib")
set_target_properties(mongoc_static PROPERTIES OUTPUT_NAME "mongoc-static-${MONGOC_API_VERSION}")

function(mongoc_add_test test use_shared)
   if (ENABLE_TESTS)
      add_executable(${test} ${ARGN})
      if (${use_shared})
         target_link_libraries(${test} mongoc_shared)
      else()
         set_target_properties(${test} PROPERTIES COMPILE_DEFINITIONS "MONGOC_COMPILATION")
         target_link_libraries(${test} mongoc_static)
      endif()
      if(WIN32)
         target_link_libraries(${test} shlwapi)
      endif()
   endif()
endfunction()

function(mongoc_add_example example use_shared)
   if (ENABLE_EXAMPLES)
      add_executable(${example} ${ARGN})
      if (${use_shared})
         target_link_libraries(${example} mongoc_shared)
      else()
         set_target_properties(${example} PROPERTIES COMPILE_DEFINITIONS "MONGOC_COMPILATION")
         target_link_libraries(${example} mongoc_static)
      endif()
      if(WIN32)
         target_link_libraries(${example} shlwapi)
      endif()
      set (EXAMPLES ${EXAMPLES} ${example})
   endif ()
endfunction()

set(test-libmongoc-sources
   ${SOURCE_DIR}/tests/debug-stream.c
   ${SOURCE_DIR}/tests/ha-test.c
   ${SOURCE_DIR}/tests/json-test.c
   ${SOURCE_DIR}/tests/mock_server/future.c
   ${SOURCE_DIR}/tests/mock_server/future-functions.c
   ${SOURCE_DIR}/tests/mock_server/future-value.c
   ${SOURCE_DIR}/tests/mock_server/sync-queue.c
   ${SOURCE_DIR}/tests/mock_server/mock-rs.c
   ${SOURCE_DIR}/tests/mock_server/mock-server.c
   ${SOURCE_DIR}/tests/mock_server/request.c
   ${SOURCE_DIR}/tests/test-conveniences.c
   ${SOURCE_DIR}/tests/test-bulk.c
   ${SOURCE_DIR}/tests/test-libmongoc.c
   ${SOURCE_DIR}/tests/test-mongoc-array.c
   ${SOURCE_DIR}/tests/test-mongoc-async.c
   ${SOURCE_DIR}/tests/test-mongoc-buffer.c
   ${SOURCE_DIR}/tests/test-mongoc-client.c
   ${SOURCE_DIR}/tests/test-bulk.c
   ${SOURCE_DIR}/tests/test-mongoc-client-pool.c
   ${SOURCE_DIR}/tests/test-mongoc-cluster.c
   ${SOURCE_DIR}/tests/test-mongoc-collection.c
   ${SOURCE_DIR}/tests/test-mongoc-collection-find.c
   ${SOURCE_DIR}/tests/test-mongoc-cursor.c
   ${SOURCE_DIR}/tests/test-mongoc-database.c
   ${SOURCE_DIR}/tests/test-mongoc-exhaust.c
   ${SOURCE_DIR}/tests/test-mongoc-find-and-modify.c
   ${SOURCE_DIR}/tests/test-mongoc-gridfs.c
   ${SOURCE_DIR}/tests/test-mongoc-gridfs-file-page.c
   ${SOURCE_DIR}/tests/test-mongoc-list.c
   ${SOURCE_DIR}/tests/test-mongoc-log.c
   ${SOURCE_DIR}/tests/test-mongoc-matcher.c
   ${SOURCE_DIR}/tests/test-mongoc-queue.c
   ${SOURCE_DIR}/tests/test-mongoc-read-prefs.c
   ${SOURCE_DIR}/tests/test-mongoc-rpc.c
   ${SOURCE_DIR}/tests/test-mongoc-sdam.c
   ${SOURCE_DIR}/tests/test-mongoc-server-selection.c
   ${SOURCE_DIR}/tests/test-mongoc-server-selection-errors.c
   ${SOURCE_DIR}/tests/test-mongoc-set.c
   ${SOURCE_DIR}/tests/test-mongoc-socket.c
   ${SOURCE_DIR}/tests/test-mongoc-stream.c
   ${SOURCE_DIR}/tests/test-mongoc-thread.c
   ${SOURCE_DIR}/tests/test-mongoc-topology.c
   ${SOURCE_DIR}/tests/test-mongoc-topology-reconcile.c
   ${SOURCE_DIR}/tests/test-mongoc-topology-scanner.c
   ${SOURCE_DIR}/tests/test-mongoc-uri.c
   ${SOURCE_DIR}/tests/test-mongoc-version.c
   ${SOURCE_DIR}/tests/test-mongoc-usleep.c
   ${SOURCE_DIR}/tests/test-mongoc-write-concern.c
   ${SOURCE_DIR}/tests/test-sasl.c
   ${SOURCE_DIR}/tests/test-write-commands.c
   ${SOURCE_DIR}/tests/TestSuite.c
)

if (MONGOC_ENABLE_SSL)
   set(test-libmongoc-sources ${test-libmongoc-sources}
      ${SOURCE_DIR}/tests/test-x509.c
      ${SOURCE_DIR}/tests/ssl-test.c
      ${SOURCE_DIR}/tests/test-mongoc-stream-tls.c
      ${SOURCE_DIR}/tests/test-mongoc-stream-tls-error.c)
   mongoc_add_test(test-replica-set-ssl FALSE
      ${SOURCE_DIR}/tests/test-replica-set-ssl.c
      ${SOURCE_DIR}/tests/ha-test.c
      ${SOURCE_DIR}/tests/mongoc-tests.c
   )
endif()

mongoc_add_test(test-load FALSE
   ${SOURCE_DIR}/tests/test-load.c
   ${SOURCE_DIR}/tests/mongoc-tests.c)
mongoc_add_test(test-secondary FALSE
   ${SOURCE_DIR}/tests/test-secondary.c
   ${SOURCE_DIR}/tests/mongoc-tests.c)
mongoc_add_test(test-replica-set FALSE
   ${SOURCE_DIR}/tests/test-replica-set.c
   ${SOURCE_DIR}/tests/mongoc-tests.c
   ${SOURCE_DIR}/tests/ha-test.c)
mongoc_add_test(test-sharded-cluster FALSE
   ${SOURCE_DIR}/tests/test-sharded-cluster.c
   ${SOURCE_DIR}/tests/mongoc-tests.c
   ${SOURCE_DIR}/tests/ha-test.c)

mongoc_add_test(test-libmongoc FALSE ${test-libmongoc-sources})

if (ENABLE_TESTS)
   enable_testing()
   add_test(NAME test-libmongoc COMMAND test-libmongoc -f -p)
endif ()

mongoc_add_example(example-gridfs TRUE ${SOURCE_DIR}/examples/example-gridfs.c)
mongoc_add_example(example-client TRUE ${SOURCE_DIR}/examples/example-client.c)
mongoc_add_example(example-scram TRUE ${SOURCE_DIR}/examples/example-scram.c)
mongoc_add_example(mongoc-dump TRUE ${SOURCE_DIR}/examples/mongoc-dump.c)
mongoc_add_example(mongoc-ping TRUE ${SOURCE_DIR}/examples/mongoc-ping.c)
mongoc_add_example(mongoc-rpc-validate FALSE ${SOURCE_DIR}/examples/mongoc-rpc-validate.c)
mongoc_add_example(mongoc-tail TRUE ${SOURCE_DIR}/examples/mongoc-tail.c)

file(COPY ${SOURCE_DIR}/tests/binary DESTINATION ${PROJECT_BINARY_DIR}/tests)
file(COPY ${SOURCE_DIR}/tests/certificates DESTINATION ${PROJECT_BINARY_DIR}/tests)

install(
  TARGETS mongoc_shared mongoc_static ${EXAMPLES}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)
install(
  FILES ${HEADERS}
  DESTINATION "include/libmongoc-${MONGOC_API_VERSION}"
)

# Define pkg-config files
set(VERSION "${MONGOC_VERSION}")
set(prefix "${CMAKE_INSTALL_PREFIX}")
set(libdir "\${prefix}/lib")
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/libmongoc-1.0.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/libmongoc-1.0.pc
@ONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/libmongoc-priv.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/libmongoc-priv.pc
@ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libmongoc-1.0.pc ${CMAKE_CURRENT_BINARY_DIR}/libmongoc-priv.pc
  DESTINATION lib/pkgconfig
)
if (MONGOC_ENABLE_SSL)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libmongoc-ssl-1.0.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/libmongoc-ssl-1.0.pc
  @ONLY)
  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/libmongoc-ssl-1.0.pc
    DESTINATION lib/pkgconfig
  )
endif()
