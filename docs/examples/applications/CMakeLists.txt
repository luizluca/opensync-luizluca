PROJECT( opensync-example-applications C )

cmake_minimum_required(VERSION 2.4.4)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules" )

FIND_PACKAGE( OpenSync REQUIRED )
#INCLUDE( OpenSyncInternal )

ADD_SUBDIRECTORY( src )

