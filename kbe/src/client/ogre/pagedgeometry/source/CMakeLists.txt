
include_directories(${PagedGeometry_SOURCE_DIR}/include)
include_directories(${PagedGeometry_BINARY_DIR}/include)
include_directories(${Ogre_INCLUDE_DIRS})
link_directories   (${Ogre_LIBRARY_DIRS})

FILE(GLOB sources *.cpp ${PagedGeometry_SOURCE_DIR}/include/*.h)

set(LIBNAME "PagedGeometry")

ADD_LIBRARY(${LIBNAME} STATIC ${sources})

# install the library
INSTALL(TARGETS ${LIBNAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
)

# install the headers: both source and build headers
INSTALL(DIRECTORY ${PagedGeometry_SOURCE_DIR}/include/ DESTINATION include/PagedGeometry FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE)
INSTALL(DIRECTORY ${PagedGeometry_BINARY_DIR}/include/ DESTINATION include/PagedGeometry FILES_MATCHING PATTERN "*.h" PATTERN ".svn" EXCLUDE)
