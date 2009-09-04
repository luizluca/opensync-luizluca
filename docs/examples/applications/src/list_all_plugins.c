#include <opensync/opensync.h>
#include <opensync/opensync-plugin.h>

int main(int argc, char *argv[]) {
	
	OSyncPluginEnv *env;
	OSyncPlugin* plugin;
	OSyncList *list, *l;
	
	osync_bool couldloadplugins;
	
	
	env = osync_plugin_env_new(NULL);
	/* load plugins from default dir */
	couldloadplugins = osync_plugin_env_load(env, NULL, NULL);
	if (!couldloadplugins) {
		/* print error */
		printf("Could not load plugins.");
		return -1;
	}
	
	list = osync_plugin_env_get_plugins(env);
	printf("found %i plugins\n", osync_list_length(list));
	
	for(l=list; l; l = l->next) {
		plugin = (OSyncPlugin *) l->data;
		printf("plugin: %s\n", osync_plugin_get_name(plugin));
	}

	osync_list_free(list);
	
	/* free env */
	osync_plugin_env_unref(env);
	
	return 0;
}
