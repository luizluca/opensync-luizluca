#include <opensync/opensync.h>
#include <opensync/opensync-group.h>

int main(int argc, char *argv[]) {
	
	int numgroups = 0;
	int i;
	
	osync_bool couldloadgroups;
	
	OSyncGroup *group = NULL;
	OSyncGroupEnv *groupenv = NULL;
	
	groupenv = osync_group_env_new(NULL);
	/* load groups from default dir */
	couldloadgroups = osync_group_env_load_groups(groupenv, NULL, NULL);
	
	if ( !couldloadgroups ) {
		/* print error to stderr */
		return -1;
	}
	
	numgroups = osync_group_env_num_groups(groupenv);
	for( i = 0; i < numgroups; i++ ) {
		group = osync_group_env_nth_group(groupenv, i);
	}
	
}
