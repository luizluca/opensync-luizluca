LINK_DIRECTORIES( ${GLIB2_LIBRARY_DIRS} ${LIBXML2_LIBRARY_DIRS} ${LIBXSLT_LIBRARY_DIRS} ${LIBEXSLT_LIBRARY_DIRS} ${SQLITE3_LIBRARY_DIRS} )
INCLUDE_DIRECTORIES( ${GLIB2_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${LIBEXSLT_INCLUDE_DIRS} ${SQLITE3_INCLUDE_DIRS} )

INCLUDE( Compiler )

SET( libopensync_LIB_SRCS
   archive/opensync_archive.c
   capabilities/opensync_capabilities.c
   capabilities/opensync_capabilities_objtype.c
   capabilities/opensync_capability.c
   client/opensync_client.c
   client/opensync_client_proxy.c
   common/opensync.c
   common/opensync_error.c
   common/opensync_file.c
   common/opensync_list.c
   common/opensync_marshal.c
   common/opensync_memory.c
   common/opensync_string.c
   common/opensync_thread.c
   common/opensync_xml.c
   data/opensync_change.c
   data/opensync_data.c
   db/opensync_db.c
   debug/opensync_trace.c
   engine/opensync_engine.c
   engine/opensync_mapping_engine.c
   engine/opensync_mapping_entry_engine.c
   engine/opensync_obj_engine.c
   engine/opensync_sink_engine.c
   engine/opensync_status.c
   format/opensync_caps_converter.c
   format/opensync_converter.c
   format/opensync_filter.c
   format/opensync_format_env.c
   format/opensync_objformat.c
   format/opensync_objformat_sink.c
   format/opensync_time.c
   format/opensync_merger.c
   group/opensync_group.c
   group/opensync_group_env.c
   group/opensync_member.c
   group/opensync_updater.c
   helper/opensync_sink_state_db.c
   helper/opensync_hashtable.c
   ipc/opensync_message.c
   ipc/opensync_queue.c
   ipc/opensync_serializer.c
   mapping/opensync_mapping.c
   mapping/opensync_mapping_entry.c
   mapping/opensync_mapping_table.c
   module/opensync_module.c
   plugin/opensync_context.c
   plugin/opensync_plugin_advancedoptions.c
   plugin/opensync_plugin_authentication.c
   plugin/opensync_plugin.c
   plugin/opensync_plugin_config.c
   plugin/opensync_plugin_connection.c
   plugin/opensync_plugin_env.c
   plugin/opensync_plugin_info.c
   plugin/opensync_plugin_localization.c
   plugin/opensync_plugin_resource.c
   plugin/opensync_plugin_externalplugin.c
   plugin/opensync_objtype_sink.c
   version/opensync_version.c
   xmlformat/opensync_xmlfield.c
   xmlformat/opensync_xmlfieldlist.c
   xmlformat/opensync_xmlformat.c
   xmlformat/opensync_xmlformat_schema.c
)

ADD_LIBRARY( opensync SHARED ${libopensync_LIB_SRCS} )

TARGET_LINK_LIBRARIES( opensync ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} ${GMODULE2_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${LIBEXSLT_LIBRARIES} ${SQLITE3_LIBRARIES} )

SET_TARGET_PROPERTIES( opensync PROPERTIES VERSION ${OPENSYNC_LIBVERSION_VERSION} )
SET_TARGET_PROPERTIES( opensync PROPERTIES SOVERSION ${OPENSYNC_LIBVERSION_SOVERSION} )

IF ( COMPILER_SUPPORTS_VISIBILITY )
  SET_TARGET_PROPERTIES( opensync PROPERTIES COMPILE_FLAGS ${SYMBOLS_VISIBILITY} )
ENDIF ( COMPILER_SUPPORTS_VISIBILITY )

IF ( OPENSYNC_UNITTESTS )
  ADD_LIBRARY( opensync-testing SHARED ${libopensync_LIB_SRCS} )
  IF ( COMPILER_SUPPORTS_VISIBILITY )
    SET_TARGET_PROPERTIES( opensync-testing PROPERTIES COMPILE_FLAGS ${SYMBOLS_VISIBILITY} )
  ENDIF ( COMPILER_SUPPORTS_VISIBILITY )
  TARGET_LINK_LIBRARIES( opensync-testing ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} ${GMODULE2_LIBRARIES} ${LIBXML2_LIBRARIES}  ${LIBXSLT_LIBRARIES} ${LIBEXSLT_LIBRARIES} ${SQLITE3_LIBRARIES} )
ENDIF ( OPENSYNC_UNITTESTS )

INSTALL( TARGETS opensync DESTINATION ${LIB_INSTALL_DIR} )

INSTALL( DIRECTORY 
  archive
  client
  common
  data
  db
  debug
  engine
  format
  group
  helper
  ipc
  mapping
  capabilities
  module
  plugin
  version
  xmlformat
DESTINATION ${OPENSYNC_INCLUDE_DIR}/opensync
PATTERN "*/*_internals.h" EXCLUDE 
PATTERN "*/*_private.h" EXCLUDE 
PATTERN ".*" EXCLUDE 
PATTERN "*.c*" EXCLUDE
PATTERN "*.h.*" EXCLUDE )

INSTALL( FILES
   opensync.h
   opensync-capabilities.h
   opensync-client.h
   opensync-context.h
   opensync-common.h
   opensync-data.h
   opensync-debug.h
   opensync-engine.h
   opensync-error.h
   opensync-format.h
   opensync-group.h
   opensync-helper.h
   opensync-ipc.h
   opensync_list.h
   opensync-mapping.h
   opensync-merger.h
   opensync-module.h
   opensync-plugin.h
   opensync-serializer.h
   opensync-support.h
   opensync-time.h
   opensync-version.h
   opensync-xmlformat.h
DESTINATION ${OPENSYNC_INCLUDE_DIR}/opensync )


########### osplugin ##################

SET( osplugin_SRCS
   client/osplugin.c
)
ADD_EXECUTABLE( osplugin ${osplugin_SRCS} )

IF(COMMAND cmake_policy)
  cmake_policy(SET CMP0005 OLD)
ENDIF(COMMAND cmake_policy)

ADD_DEFINITIONS( -DOSPLUGIN="\\\"${OPENSYNC_LIBEXEC_DIR}/osplugin\\\"" )

TARGET_LINK_LIBRARIES( osplugin opensync ${GLIB2_LIBRARIES} ${GTHREAD2_LIBRARIES} ${GMODULE2_LIBRARIES} )

# TODO: osplugin in libexec dir?
INSTALL( TARGETS osplugin DESTINATION ${OPENSYNC_LIBEXEC_DIR} ) 

