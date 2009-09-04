LINK_DIRECTORIES(  ${OPENSYNC_LIBRARY_DIRS} )
INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} )

# create executables
SET( appl_SRCS list_all_groups.c )
SET( appl_NAME list_all_groups )
ADD_EXECUTABLE( ${appl_NAME} ${appl_SRCS} )
TARGET_LINK_LIBRARIES( ${appl_NAME} ${OPENSYNC_LIBRARIES} )

SET( appl_SRCS list_all_plugins.c )
SET( appl_NAME list_all_plugins )
ADD_EXECUTABLE( ${appl_NAME} ${appl_SRCS} )
TARGET_LINK_LIBRARIES( ${appl_NAME} ${OPENSYNC_LIBRARIES} )

SET( appl_SRCS list_all_formats.c )
SET( appl_NAME list_all_formats )
ADD_EXECUTABLE( ${appl_NAME} ${appl_SRCS} )
TARGET_LINK_LIBRARIES( ${appl_NAME} ${OPENSYNC_LIBRARIES} )
