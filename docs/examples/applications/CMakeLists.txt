PROJECT( opensync-example-applications C )

cmake_minimum_required(VERSION 2.4.4)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules" )

FIND_PACKAGE( OpenSync REQUIRED )
#INCLUDE( OpenSyncInternal )

LINK_DIRECTORIES(  ${OPENSYNC_LIBRARY_DIRS} )
INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} )

# create executables
SET( appl_SRCS list_all_groups.c )
SET( appl_NAME list_all_groups )
ADD_EXECUTABLE( ${appl_NAME} ${appl_SRCS} )
TARGET_LINK_LIBRARIES( ${appl_NAME} ${OPENSYNC_LIBRARIES} )