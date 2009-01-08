#include <opensync/opensync.h>
#include <opensync/opensync-group.h>

int main(int argc, char *argv[]) {
	
	int numgroups = 0;
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
	
	numgroups = osync_group_env_num_groups(groupenv);
	printf("found %i groups\n", numgroups);
	
	for( i = 0; i < numgroups; i++ ) {
		group = osync_group_env_nth_group(groupenv, i);
		printf("group nr. %i is %s\n", i+1, osync_group_get_name(group));
	}
	
	/* free env */
	osync_group_env_unref(groupenv);
	
	return 0;
}
