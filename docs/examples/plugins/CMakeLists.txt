PROJECT( libopensync-plugin-example )

SET( VERSION "0.40" )

cmake_minimum_required(VERSION 2.4.4)

FIND_PACKAGE( OpenSync REQUIRED )
FIND_PACKAGE( GLIB2 REQUIRED )

INCLUDE( OpenSyncInternal )

ADD_SUBDIRECTORY( src )

## Packaging

OPENSYNC_PACKAGE( ${CMAKE_PROJECT_NAME} ${VERSION} )

