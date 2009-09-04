#include <opensync/opensync.h>
#include <opensync/opensync-group.h>

int main(int argc, char *argv[]) {
	
	OSyncList *groups, *g;
	int i = 0;
	
	osync_bool couldloadgroups;
	
	OSyncGroup *group = NULL;
	OSyncGroupEnv *groupenv = NULL;
	
	groupenv = osync_group_env_new(NULL);
	/* load groups from default dir */
	couldloadgroups = osync_group_env_load_groups(groupenv, NULL, NULL);
	
	if ( !couldloadgroups ) {
		/* print error */
		printf("Could not load groups.");
		return -1;
	}
	
	groups = osync_group_env_get_groups(groupenv);
	printf("found %i groups\n", osync_list_length(groups));
	
	for (g = groups; g; g = g->next) {
		group = (OSyncGroup *) g->data;
		printf("group nr. %i is %s\n", i+1, osync_group_get_name(group));
	}

	/* Free the list */
	osync_list_free(groups);
	
	/* free env */
	osync_group_env_unref(groupenv);
	
	return 0;
}
