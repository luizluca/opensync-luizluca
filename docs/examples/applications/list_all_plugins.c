#include <opensync/opensync.h>
#include <opensync/opensync-plugin.h>

int main(int argc, char *argv[]) {
	
	OSyncPluginEnv *env;
	OSyncPlugin* plugin;
	
	osync_bool couldloadplugins;
	
	int numplugins;
	int i = 0;
	
	env = osync_plugin_env_new(NULL);
	/* load plugins from default dir */
	couldloadplugins = osync_plugin_env_load(env, NULL, NULL);
	if (!couldloadplugins) {
		/* print error */
		printf("Could not load plugins.");
		return -1;
	}
	
	numplugins = osync_plugin_env_num_plugins(env);
	printf("found %i plugins\n", i);
	
	for( i=0; i < numplugins; i++ ) {
		plugin = osync_plugin_env_nth_plugin(env, i);
		printf("plugin nr. %i is %s\n", i+1, osync_plugin_get_name(plugin));
	}
	
	/* free env */
	osync_plugin_env_unref(env);
	
	return 0;
}
