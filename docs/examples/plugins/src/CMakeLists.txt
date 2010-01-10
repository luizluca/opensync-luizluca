LINK_DIRECTORIES(  ${OPENSYNC_LIBRARY_DIRS} )
INCLUDE_DIRECTORIES( ${OPENSYNC_INCLUDE_DIRS} )

### Simple Synchronization Plugin ########
OPENSYNC_PLUGIN_ADD( example_simple_plugin simple_plugin.c )
TARGET_LINK_LIBRARIES( example_simple_plugin ${OPENSYNC_LIBRARIES} )
OPENSYNC_PLUGIN_INSTALL( example_simple_plugin )

### Simple Synchronization Plugin with Static Capabilities ########
OPENSYNC_PLUGIN_ADD( example_simple_plugin_static_caps simple_plugin_static_caps.c )
TARGET_LINK_LIBRARIES( example_simple_plugin_static_caps ${OPENSYNC_LIBRARIES} )
OPENSYNC_PLUGIN_INSTALL( example_simple_plugin_static_caps )

OPENSYNC_PLUGIN_CAPABILITIES( simple_plugin_static_caps_capabilities.xml ) # install capabilites file
OPENSYNC_PLUGIN_DESCRIPTIONS( simple_plugin_static_caps_description.xml ) # install description file

### Advanced Synchronization Plugin ######
OPENSYNC_PLUGIN_ADD( example_plugin plugin.c )
TARGET_LINK_LIBRARIES( example_plugin ${OPENSYNC_LIBRARIES} )
OPENSYNC_PLUGIN_INSTALL( example_plugin )

### External Synchronization Plugin ########
OPENSYNC_EXTERNAL_PLUGIN_INSTALL( external_plugin.xml )

ADD_EXECUTABLE( example_external_demo external_demo.c )
TARGET_LINK_LIBRARIES( example_external_demo ${OPENSYNC_LIBRARIES} )

### Format Plugin ########################
OPENSYNC_FORMAT_ADD( format_plugin format.c )
TARGET_LINK_LIBRARIES( format_plugin ${OPENSYNC_LIBRARIES} )
OPENSYNC_FORMAT_INSTALL( format_plugin )

