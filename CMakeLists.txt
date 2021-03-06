PROJECT( libopensync C )

SET( OPENSYNC_VERSION_MAJOR "0" )
SET( OPENSYNC_VERSION_MINOR "40" )

# The most recent interface number that this library implements.
#UPDATE: If any interface have been added, removed or changed since
#UPDATE: the last update increment CURRENT.
SET( OPENSYNC_LIBVERSION_CURRENT 1 )

# The implementation number of the CURRENT interface.
# UPDATE: Increment only if the library code has changed at all, since
# UPDATE: last release.
# UPDATE: Set REVISION to 0 if any interface have been added, removed or
# UPDATE: changed since the last update.
SET( OPENSYNC_LIBVERSION_REVISION 0 )

# The difference between the newest and the oldest interfaces.
# UPDATE: If any interface have been added since the last public
# UPDATE: release, then increment.
# UPDATE: If any interface have been removed since the last release
# UPDATE: reset to 0.
SET( OPENSYNC_LIBVERSION_AGE 0 )


# The range of implemention CURRENT - AGE is the SOVERSION
MATH( EXPR OPENSYNC_LIBVERSION_SOVERSION "${OPENSYNC_LIBVERSION_CURRENT} - ${OPENSYNC_LIBVERSION_AGE}" )

SET( OPENSYNC_LIBVERSION_VERSION "${OPENSYNC_LIBVERSION_SOVERSION}.${OPENSYNC_LIBVERSION_REVISION}.${OPENSYNC_LIBVERSION_AGE}" )

IF ( $ENV{OPENSYNC_RELEASE} )
	SET( OPENSYNC_VERSION "${OPENSYNC_VERSION_MAJOR}.${OPENSYNC_VERSION_MINOR}" )	
ELSE ( $ENV{OPENSYNC_RELEASE} )

	IF (DEFINED OPENSYNC_SCM_REVISION )
		SET( OPENSYNC_VERSION "${OPENSYNC_VERSION_MAJOR}.${OPENSYNC_VERSION_MINOR}-${OPENSYNC_SCM_REVISION}" )	
	ELSE (DEFINED OPENSYNC_SCM_REVISION )
		SET( OPENSYNC_VERSION "${OPENSYNC_VERSION_MAJOR}.${OPENSYNC_VERSION_MINOR}-snapshot" )	
	ENDIF (DEFINED OPENSYNC_SCM_REVISION )
ENDIF ( $ENV{OPENSYNC_RELEASE} )

SET( OPENSYNC_PLUGINVERSION ${OPENSYNC_LIBVERSION_SOVERSION} )

################################################	

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake )

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

IF(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

INCLUDE( Documentation )
INCLUDE( OpenSyncDefaults )
INCLUDE( OpenSyncPackaging )
INCLUDE( OpenSyncPlugin )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_BINARY_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/opensync" ${CMAKE_CURRENT_SOURCE_DIR} )

###############################################

# Minimum Requirment for glib2 to be compatible with
# various distros: RHEL5, maemo, ...
# http://thread.gmane.org/gmane.comp.misc.opensync.devel/3901/focus=3902
SET ( GLIB2_MIN_VERSION "2.12" )

FIND_PACKAGE( GLIB2 REQUIRED )
FIND_PACKAGE( GTHREAD2 REQUIRED )
FIND_PACKAGE( GMODULE2 REQUIRED )
FIND_PACKAGE( Sqlite3 REQUIRED )
FIND_PACKAGE( LibXml2 REQUIRED )
FIND_PACKAGE( LibXslt REQUIRED )
FIND_PACKAGE( LibExslt REQUIRED )
FIND_PACKAGE( SWIG )	
FIND_PACKAGE( PythonLibs )
FIND_PACKAGE( PythonInterp )
FIND_PACKAGE( Check )

# test configuration

OPTION( OPENSYNC_UNITTESTS "enable testing" OFF )
IF ( CHECK_FOUND )
	SET( OPENSYNC_UNITTESTS ON )
ELSE ( CHECK_FOUND )
	SET( OPENSYNC_UNITTESTS OFF )
ENDIF ( CHECK_FOUND )

# sub directories

ADD_SUBDIRECTORY( opensync )
ADD_SUBDIRECTORY( tools )
ADD_SUBDIRECTORY( misc )
ADD_SUBDIRECTORY( cmake )

OPTION( OPENSYNC_API_EXAMPLE "enable build of OpenSync API examples" OFF )
IF ( OPENSYNC_API_EXAMPLE )
	SET( OPENSYNC_LIBRARIES opensync )
	ADD_SUBDIRECTORY( docs/examples/plugins/src/ ) 
	ADD_SUBDIRECTORY( docs/examples/applications/src/ ) 
ENDIF ( OPENSYNC_API_EXAMPLE )

OPTION( ENABLE_WRAPPER "enable wrapper" OFF )
IF ( SWIG_FOUND )
	ADD_SUBDIRECTORY( wrapper )
	SET( ENABLE_WRAPPER ON )
ELSE ( SWIG_FOUND )
	SET( ENABLE_WRAPPER OFF )
ENDIF ( SWIG_FOUND )

IF ( OPENSYNC_UNITTESTS )
	INCLUDE( Testing )
	ADD_SUBDIRECTORY( tests )
ENDIF ( OPENSYNC_UNITTESTS )

IF ( CMAKE_SYSTEM_NAME MATCHES SunOS )
	SET( HAVE_SOLARIS 1 )
ENDIF (CMAKE_SYSTEM_NAME MATCHES SunOS )

############################################## 
MARK_AS_ADVANCED ( CLEAR BUILD_DOCUMENTATION )

IF ( BUILD_DOCUMENTATION )
	IF ( DOXYGEN_DOT_EXECUTABLE )
		SET( HAVE_DOT "YES" )
	ENDIF ( DOXYGEN_DOT_EXECUTABLE )
	CONFIGURE_FILE( "Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" )
	CONFIGURE_FILE( "misc/doxygen.css" "${CMAKE_CURRENT_BINARY_DIR}/misc/doxygen.css" )
	ADD_CUSTOM_TARGET( DoxygenDoc ${DOXYGEN_EXECUTABLE} )
ENDIF ( BUILD_DOCUMENTATION )

# Check if system has flock(..) function
INCLUDE( CheckFunctionExists )
SET( CMAKE_REQUIRED_INCLUDES "" )
SET( CMAKE_REQUIRED_LIBRARIES "" )
SET( CMAKE_REQUIRED_FLAGS "" )
SET( CMAKE_REQUIRED_DEFINITIONS "" )
CHECK_FUNCTION_EXISTS( flock HAVE_FLOCK )

IF ( APPLE ) 
	SET( OPENSYNC_PREVENT_CLIENT_SHUTDOWN "1" )
ENDIF ( APPLE )

CONFIGURE_FILE( "config.h.cmake" "${CMAKE_CURRENT_BINARY_DIR}/config.h" )
CONFIGURE_FILE( "libopensync1.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/libopensync1.pc" )

########## INSTALL ##############################
	
INSTALL( FILES "${CMAKE_CURRENT_BINARY_DIR}/libopensync1.pc" DESTINATION ${LIBDATA_INSTALL_DIR}/pkgconfig/ )	

### CPack ########################################

OPENSYNC_PACKAGE( ${PROJECT_NAME} ${OPENSYNC_VERSION} )

######## configuration status output ###############
INCLUDE( ShowStatus )
MESSAGE( STATUS "==================================================" )
SHOW_STATUS( OPENSYNC_UNITTESTS "enable testing\t\t" )
SHOW_STATUS( ENABLE_WRAPPER "build wrapper\t\t" )
SHOW_STATUS( BUILD_DOCUMENTATION "build documentation\t\t" )
