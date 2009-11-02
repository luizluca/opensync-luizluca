## Build OpenSync plugins as module
MACRO( OPENSYNC_PLUGIN_ADD _pluginName ) 

  SET( CMAKE_SHARED_MODULE_PREFIX "" )
  ADD_LIBRARY( ${_pluginName} MODULE ${ARGN} )

ENDMACRO( OPENSYNC_PLUGIN_ADD )

## Build OpenSync format plugins as module
MACRO( OPENSYNC_FORMAT_ADD _formatName ) 

  SET( CMAKE_SHARED_MODULE_PREFIX "" )
  ADD_LIBRARY( ${_formatName} MODULE ${ARGN} )

ENDMACRO( OPENSYNC_FORMAT_ADD )

## Install plugin
MACRO( OPENSYNC_PLUGIN_INSTALL _pluginName ) 
  INSTALL( TARGETS ${_pluginName} DESTINATION ${OPENSYNC_PLUGINDIR} )
ENDMACRO( OPENSYNC_PLUGIN_INSTALL )

## Install external plugin
### Got introduced to be able in future to move those plugins into
### a seperated directory, without fixing all plugin build environments
MACRO( OPENSYNC_EXTERNAL_PLUGIN_INSTALL _pluginName ) 
  INSTALL( TARGETS ${_pluginName} DESTINATION ${OPENSYNC_PLUGINDIR} )
ENDMACRO( OPENSYNC_EXTERNAL_PLUGIN_INSTALL )

## Install format plugin
MACRO( OPENSYNC_FORMAT_INSTALL _pluginName  ) 
  INSTALL( TARGETS ${_pluginName} DESTINATION ${OPENSYNC_FORMATSDIR} )
ENDMACRO( OPENSYNC_FORMAT_INSTALL )

## Install plugin description files
MACRO( OPENSYNC_PLUGIN_DESCRIPTIONS _descFiles ) 
  INSTALL( FILES ${_descFiles} DESTINATION ${OPENSYNC_DESCRIPTIONSDIR} )
ENDMACRO( OPENSYNC_PLUGIN_DESCRIPTIONS )

## Install plugin capabilities files
MACRO( OPENSYNC_PLUGIN_CAPABILITIES _capFiles ) 
  INSTALL( FILES ${_capFiles} DESTINATION ${OPENSYNC_CAPABILITIESDIR} )
ENDMACRO( OPENSYNC_PLUGIN_CAPABILITIES )

## Install plugin default configuration
MACRO( OPENSYNC_PLUGIN_CONFIG _pluginConfig )

  INSTALL( FILES ${_pluginConfig} DESTINATION ${OPENSYNC_CONFIGDIR} )

ENDMACRO( OPENSYNC_PLUGIN_CONFIG )

