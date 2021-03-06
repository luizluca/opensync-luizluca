#ifndef OSYNC_TEST_SUPPORT
#define OSYNC_TEST_SUPPORT

#include <check.h>

#include <opensync/opensync.h>
#include <opensync/opensync_internals.h>
#include <opensync/opensync-engine.h>
#include <opensync/opensync-mapping.h>
#include <opensync/opensync-helper.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-group.h>
#include <opensync/opensync-plugin.h>

#include "opensync/helper/opensync_hashtable_internals.h"
#include "opensync/engine/opensync_mapping_engine_internals.h"

#include "config.h"

#define OSYNC_TESTCASE_START(x)		\
	const char *_unittest = (#x);	\
	struct osync_testcase_s osync_testcase[] = {

#define OSYNC_TESTCASE_ADD(x) { (#x), x },

#define OSYNC_TESTCASE_END			\
	{ NULL, NULL }				\
	};					\
						\
int main(int argc, char **argv)			\
{						\
	return osync_testsuite(argc, argv, _unittest, osync_testcase);	\
}

struct osync_testcase_s {
	const char *name;
	void *func;
};


int osync_system(const char *command);
int osync_testsuite(int argc, char **argv, const char *unittest,
		struct osync_testcase_s *tc);


int num_client_connected;
int num_client_main_connected;
int num_client_connect_done;
int num_client_main_connect_done;
int num_client_disconnected;
int num_client_main_disconnected;
int num_client_read;
int num_client_main_read;
int num_client_written;
int num_client_main_written;
int num_client_errors;
int num_client_sync_done;
int num_client_main_sync_done;
int num_client_discovered;

int num_change_read;
int num_change_written;
int num_change_error;

int num_engine_connected;
int num_engine_connect_done;
int num_engine_read;
int num_engine_prepared_map;
int num_engine_mapped;
int num_engine_multiplied;
int num_engine_prepared_write;
int num_engine_written;
int num_engine_disconnected;
int num_engine_errors;
int num_engine_successful;
int num_engine_end_conflicts;
int num_engine_prev_unclean;
int num_engine_sync_done;


int num_mapping_solved;
int num_mapping_written;
int num_mapping_errors;
int num_mapping_conflicts;

void check_env(void);

char *setup_testbed(const char *fkt_name);
void destroy_testbed(char *path);
// create_case() with timeout of 30seconds (default)
void create_case(Suite *s, const char *name, TFun function);
// create_case_timeout() allow to specific a specific timeout - intended for breaking testcases which needs longer then 30seconds (default)
void create_case_timeout(Suite *s, const char *name, TFun function, int timeout);

void conflict_handler_random(OSyncEngine *engine, OSyncMapping *mapping, void *user_data);

OSyncMappingTable *mappingtable_load(const char *path, const char *objtype, unsigned int num_mappings);
void mappingtable_close(OSyncMappingTable *maptable);

OSyncHashTable *hashtable_load(const char *path, const char *objtype, unsigned int entries);
void check_hash(OSyncHashTable *table, const char *cmpuid);
void check_mapping(OSyncMappingTable *table, int memberid, int mappingid, unsigned int numentries, const char *uid);
/** @brief Confirms a mapping table is complete after a sync
     Does not work if conflicts have been ignored

    @param testbed path to the testbed
    @param num_members number of members the sync was performed between
    @param uids array of uids that should be in the mapping table
    @param num_uids number of uids in the uids array
*/
void validate_mapping_table(const char *testbed, unsigned int num_members, const char *uids[], unsigned int num_uids);

void create_random_file(const char *path);

void reset_counters();
osync_bool synchronize_once(OSyncEngine *engine, OSyncError **error);
void discover_all_once(OSyncEngine *engine, OSyncError **error);

/* Status callbacks */
void member_status(OSyncEngineMemberUpdate *status, void *user_data);
void entry_status(OSyncEngineChangeUpdate *status, void *user_data);
void engine_status(OSyncEngineUpdate *status, void *user_data);
void mapping_status(OSyncEngineMappingUpdate *status, void *user_data);

/* Conflict handlers */
void conflict_handler_choose_member(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void conflict_handler_choose_first(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void conflict_handler_choose_deleted(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void conflict_handler_choose_modified(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void conflict_handler_ignore(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void conflict_handler_duplicate(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void conflict_handler_abort(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);
void solve_conflict(OSyncMappingEngine *mapping);
void conflict_handler_delay(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data);

/* Plugin Env helper */
OSyncFormatEnv *osync_testing_load_formatenv(const char *formatdir);

/* File testing helper */
osync_bool osync_testing_file_exists(const char *file);
osync_bool osync_testing_file_remove(const char *file);
osync_bool osync_testing_file_chmod(const char *file, int mode);
osync_bool osync_testing_file_copy(const char *source, const char *dest);
osync_bool osync_testing_diff(const char *file1, const char *file2);
osync_bool osync_testing_directory_is_empty(const char *dirname);

/* Plugin config helper */
OSyncPluginConfig *simple_plugin_config(OSyncPluginConfig *config, const char *path, const char *objtype, const char *objformat, const char *format_config);

/* gdiff is GNU diff in Solaris */
#ifdef HAVE_SOLARIS
#define DIFF "gdiff"
#else
#define DIFF "diff"
#endif

/* Engine helper */
OSyncEngine *osync_testing_create_engine_dummy(unsigned int member_size);

/* System Env helper */
void osync_testing_system_abort(const char *command);

#endif /* OSYNC_TEST_SUPPORT */

