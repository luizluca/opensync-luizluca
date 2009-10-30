/*
 *
 * !!! This is only a temporary solution and will be soon obsolete. !!!
 *
 * This plugin is just a stub to register an external plugin to the plugin-env
 * In the future this will be done via configuration files.
 *
 */

#include <opensync/opensync.h>
#include <opensync/opensync-plugin.h>

osync_bool get_sync_info(OSyncPluginEnv *env, OSyncError **error)
{
	OSyncPlugin *plugin = osync_plugin_new(error);
	if (!plugin)
		goto error;
	
	//Tell opensync something about your plugin
	osync_plugin_set_name(plugin, "external-demo");
	osync_plugin_set_longname(plugin, "long name. maybe < 50 chars");
	osync_plugin_set_description(plugin, "A longer description. < 200 chars");
	osync_plugin_set_start_type(plugin, OSYNC_START_TYPE_EXTERNAL);

	if (!osync_plugin_env_register_plugin(env, plugin, error))
		goto error;

	osync_plugin_unref(plugin);

	return TRUE;
error:
	osync_trace(TRACE_ERROR, "Unable to register: %s", osync_error_print(error));
	return FALSE;
}

int get_version(void)
{
	return 1;
}

