#include "support.h"
#ifndef _WIN32
#include <sys/wait.h> /* For WEXITSTATUS */
#endif

#include "opensync/engine/opensync_engine_internals.h"
#include "opensync/archive/opensync_archive_internals.h"
#include "opensync/group/opensync_group_internals.h"
#include "opensync/mapping/opensync_mapping_table_internals.h"
#include "opensync/mapping/opensync_mapping_internals.h"

char *olddir = NULL;


int osync_system(const char *command)
{
	int status = system(command);
	if (WIFEXITED(status))
		return WEXITSTATUS(status);
	else {
		printf("system() failed while executing: '%s' result: %d, errno: %d, %s\n",
			command, status, (errno), strerror(errno));
		return -1;
	}
}

void osync_testsuite_all(Suite *s, struct osync_testcase_s *tc)
{
	unsigned int i;
	for (i=0; tc[i].name; i++)
		create_case(s, tc[i].name, tc[i].func);
}

unsigned int osync_testsuite_selected(Suite *s, int argc, char **argv,
		struct osync_testcase_s *tc)
{
	int i, j, n=0;
	/* Also argv[0]! for symlink-ed calls */
	for (i=0; argc > i; i++) {
		for (j=0; tc[j].name; j++) {
			if (strcmp(g_basename(argv[i]), tc[j].name))
					continue;

			create_case(s, tc[j].name, tc[j].func);
			n++;
		}
	}

	return n;
}

int osync_testsuite(int argc, char **argv, const char *unittest,
		struct osync_testcase_s *tc)
{
	int nf;
	Suite *s = suite_create(unittest);
	SRunner *sr;

	check_env();

	if (!osync_testsuite_selected(s, argc, argv, tc))
		osync_testsuite_all(s, tc);

	sr = srunner_create(s);
	srunner_run_all(sr, CK_VERBOSE);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void reset_env(void)
{
	g_unsetenv("CONNECT_ERROR");
	g_unsetenv("CONNECT_TIMEOUT");
	g_unsetenv("CONNECT_SLOWSYNC");
	g_unsetenv("INIT_NULL");
	g_unsetenv("INIT_NULL_NOERROR");
	g_unsetenv("GET_CHANGES_ERROR");
	g_unsetenv("GET_CHANGES_TIMEOUT");
	g_unsetenv("GET_CHANGES_TIMEOUT2");
	g_unsetenv("COMMIT_ERROR");
	g_unsetenv("COMMIT_TIMEOUT");
	g_unsetenv("SYNC_DONE_ERROR");
	g_unsetenv("SYNC_DONE_TIMEOUT");
	g_unsetenv("DISCONNECT_ERROR");
	g_unsetenv("DISCONNECT_TIMEOUT");
	g_unsetenv("COMMITTED_ALL_ERROR");
	g_unsetenv("NO_COMMITTED_ALL_CHECK");
	g_unsetenv("MAINSINK_CONNECT");

	g_unsetenv("OSYNC_NOMEMORY");
	g_unsetenv("MOCK_SYNC_EXPECT_COMMIT_ALWAYS_ADDED");
	g_unsetenv("MOCK_SYNC_ALWAYS_CHANGETYPE_MODIFIED");

	g_unsetenv("MOCK_FORMAT_PATH_COMPARE_NO");

	g_unsetenv("MOCK_DISCOVER_ERROR");
}


void check_env(void) {

	if (g_getenv("OSYNC_TRACE"))
		fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
				"WARNING! Environment variable OSYNC_TRACE is set.\n"
				"This unit contains stress & performance tests which\n"
				"will fail/timeout because of bad performance impact of\n"
				"the i/o which will be produced by OSYNC_TRACE!\n"
				"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n");
}

char *setup_testbed(const char *fkt_name)
{
#ifndef _WIN32	
	setuid(65534);
#endif
	char *testbed = g_strdup_printf("%s/testbed.XXXXXX", g_get_tmp_dir());
	char *command = NULL, *dirname = NULL;
#ifdef _WIN32
	if(g_file_test(testbed, G_FILE_TEST_IS_DIR))
		destroy_testbed(g_strdup(testbed));

	if(g_mkdir(testbed,0777) < 0){
			osync_trace(TRACE_INTERNAL, "%s: Cannot create testbed directory %s", __func__, testbed);
			abort();
	}
#else /* WIN32 */
	if (!mkdtemp(testbed))
		abort();
#endif /* WIN32 */
	
	if (fkt_name) {
		dirname = g_strdup_printf(OPENSYNC_TESTDATA"/%s", fkt_name);
		if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
			osync_trace(TRACE_INTERNAL, "%s: Path %s not exist.", __func__, dirname);
			abort();
		}
		command = g_strdup_printf("cp -R %s/* %s", dirname, testbed);
		if (osync_system(command))
			abort();
		g_free(command);

		/* special handling for the git repo, since git does not
		 * support empty directories, check for a file named
		 * "empty_dirs" and create every subdirectory listed
		 * there that does not begin with a slash (relative dirs only)
		 */
		{
			char *empty_name = NULL;
			FILE *fh = NULL;

			empty_name = g_strdup_printf("%s/empty_dirs", dirname);
			if ((fh = fopen(empty_name, "r"))) {
				char line[100], *s;
				while ((s = fgets(line, sizeof(line), fh))) {
					int len = strlen(s);
					/* trim newline */
					if (len && s[len-1] == '\n')
						s[len-1] = 0;
					/* only create relative paths */
					if (len && s[0] != '/' && s[0] != '\\' && s[0] != '#') {
						char *newdir = g_strdup_printf("%s/%s", testbed, s);
						if (newdir) {
							g_mkdir_with_parents(newdir, 0755);
							g_free(newdir);
						}
					}
				}
				fclose(fh);
			}
			g_free(empty_name);
		}

		g_free(dirname);
	}
	
	dirname = g_strdup_printf("%s/formats",  testbed);
	if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
		if (g_mkdir(dirname,0777) < 0) {
			osync_trace(TRACE_ERROR, "Could not create format direcotry %s", dirname);
			abort();
		}
	}
	g_free(dirname);

	dirname = g_strdup_printf("%s/plugins",  testbed);
	if (!g_file_test(dirname, G_FILE_TEST_IS_DIR)) {
		if (g_mkdir(dirname,0777) < 0) {
			osync_trace(TRACE_ERROR, "Could not create plugin direcotry %s", dirname);
			abort();
		}
	}
	g_free(dirname);
	
	command = g_strdup_printf("cp ./mock-plugin/mock-sync.%s %s/plugins", G_MODULE_SUFFIX, testbed);
	if (osync_system(command))
		abort();
	g_free(command);
	
	command = g_strdup_printf("cp ./mock-plugin/mock-format.%s %s/formats", G_MODULE_SUFFIX, testbed);
	if (osync_system(command))
		abort();
	g_free(command);

	command = g_strdup_printf("cp -R %s/../../misc/schemas/*.xsd %s", OPENSYNC_TESTDATA, testbed);
	if (osync_system(command))
		abort();
	g_free(command);

#ifndef _WIN32	/* chmod is useless on windows system */
	command = g_strdup_printf("chmod -R 700 %s", testbed);
	if (osync_system(command))
		abort();
	g_free(command);
#endif
		
	olddir = g_get_current_dir();
	if (g_chdir(testbed) < 0){
		osync_trace(TRACE_ERROR, "Could not chdir to testbed");
		abort();
	}
	
	reset_counters();

	osync_trace(TRACE_INTERNAL, "Seting up %s at %s", fkt_name, testbed);
/*	printf(".");
	fflush(NULL);*/
	reset_env();
	return testbed;
}

void destroy_testbed(char *path)
{
	char *command = g_strdup_printf("rm -rf %s", path);
	if (olddir) {
		if (g_chdir(olddir) < 0)
			abort();
		g_free(olddir);
	}
	if (osync_system(command))
		abort();

	g_free(command);
	osync_trace(TRACE_INTERNAL, "Tearing down %s", path);
	g_free(path);
}

void create_case(Suite *s, const char *name, TFun function)
{
	TCase *tc_new = tcase_create(name);
	tcase_set_timeout(tc_new, 30);
	suite_add_tcase (s, tc_new);
	tcase_add_test(tc_new, function);
}


// create_case_timeout() allow to specific a specific timeout - intended for breaking testcases which needs longer then 30seconds (default)
void create_case_timeout(Suite *s, const char *name, TFun function, int timeout)
{
	TCase *tc_new = tcase_create(name);
	tcase_set_timeout(tc_new, timeout);
	suite_add_tcase (s, tc_new);
	tcase_add_test(tc_new, function);
}

OSyncMappingTable *mappingtable_load(const char *path, const char *objtype, unsigned int num_mappings)
{
	OSyncError *error = NULL;
	OSyncMappingTable *table = NULL;
	OSyncArchive *archive = NULL;

	osync_trace(TRACE_ENTRY, "%s(%s, %s, %i)", __func__, path, objtype, num_mappings);
	
	table = osync_mapping_table_new(&error);
	fail_unless(table != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	archive = osync_archive_new(path, &error);
	fail_unless(archive != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_mapping_table_load(table, archive, objtype, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	osync_archive_unref(archive);
	
	fail_unless(osync_mapping_table_num_mappings(table) == num_mappings, NULL);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, table);
	return table;
}

void check_mapping(OSyncMappingTable *maptable, int memberid, int mappingid, unsigned int numentries, const char *uid)
{
	unsigned int i = 0;
	osync_trace(TRACE_ENTRY, "%s(%p, %i, %i, %i, %s)", __func__, maptable, memberid, mappingid, numentries, uid);
	
	for (i = 0; i < osync_mapping_table_num_mappings(maptable); i++) {
		OSyncMapping *mapping = osync_mapping_table_nth_mapping(maptable, i);
		OSyncMappingEntry *testentry = osync_mapping_find_entry_by_member_id(mapping, memberid);
		if (testentry) {
			if ((mappingid != -1 && osync_mapping_get_id(mapping) == mappingid) || (mappingid == -1 && !strcmp(osync_mapping_entry_get_uid(testentry), uid))) {
				unsigned int n = 0;
				fail_unless(osync_mapping_num_entries(mapping) == numentries);
				for (n = 0; n < osync_mapping_num_entries(mapping); n++) {
					OSyncMappingEntry *entry = osync_mapping_nth_entry(mapping, n);
					if (osync_mapping_entry_get_member_id(entry) == memberid) {
						fail_unless(!strcmp(osync_mapping_entry_get_uid(entry), uid), NULL);
						goto out;
					}
				}
				fail(NULL);
			}
		}
	}
	fail(NULL);

out:
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void validate_mapping_table(const char *testbed, unsigned int num_members, const char *uids[], unsigned int num_uids)
{
	unsigned int member;
	unsigned int uid;
	osync_trace(TRACE_ENTRY, "%s(%s, %i, %p, %i)", __func__, testbed, num_members, uids, num_uids);

	char *path = g_strdup_printf("%s/configs/group/archive.db", testbed);
	OSyncMappingTable *maptable = mappingtable_load(path, "mockobjtype1", num_uids);
	g_free(path);

	/* check we have num_members mapping entries for each uid */
	for (uid = 0; uid < num_uids; uid++) {
		for (member = 1; member <= num_members; member++) {
			check_mapping(maptable, member, -1, num_members, uids[uid]);
		}
	}
	osync_mapping_table_close(maptable);
	osync_mapping_table_unref(maptable);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

OSyncHashTable *hashtable_load(const char *path, const char *objtype, unsigned int entries)
{
	OSyncError *error = NULL;
	OSyncHashTable *table = osync_hashtable_new(path, objtype, NULL, &error);
	fail_unless(table != NULL, NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(osync_hashtable_load(table, &error), NULL);
	
	fail_unless(osync_hashtable_num_entries(table) == entries, NULL);

	return table;
}

void check_hash(OSyncHashTable *table, const char *cmpuid)
{
	fail_unless(!!osync_hashtable_get_hash(table, cmpuid), "Couldn't find hash!");
}


void create_random_file(const char *path)
{
	OSyncError *error = NULL;
	char *content = osync_rand_str(g_random_int_range(100, 200), &error);
	fail_unless(error == NULL, NULL);
	osync_assert(osync_file_write(path, content, strlen(content), 0700, NULL) == TRUE);
	g_free(content);
}

void member_status(OSyncEngineMemberUpdate *status, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p), %p)", __func__, status, user_data);
	fail_unless(GINT_TO_POINTER(1) == user_data, NULL);

        OSyncError *error = osync_engine_member_update_get_error(status);
        const char *objtype = osync_engine_member_update_get_objtype(status);
	
	switch (osync_engine_member_update_get_event(status)) {
		case OSYNC_ENGINE_MEMBER_EVENT_CONNECTED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			
			if (objtype == NULL) {
				num_client_main_connected++;
			} else {
				fail_unless(!strncmp(objtype, "mockobjtype", 11), NULL);
				num_client_connected++;
			}
			
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_CONNECT_DONE:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			
			if (objtype == NULL) {
				num_client_main_connect_done++;
			} else {
				fail_unless(!strncmp(objtype, "mockobjtype", 11), NULL);
				num_client_connect_done++;
			}
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_DISCONNECTED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			
			if (objtype == NULL) {
				num_client_main_disconnected++;
			} else {
				fail_unless(!strncmp(objtype, "mockobjtype", 11), NULL);
				num_client_disconnected++;
			}
			
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_READ:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			
			if (objtype == NULL) {
				num_client_main_read++;
			} else {
				fail_unless(!strncmp(objtype, "mockobjtype", 11), NULL);
				num_client_read++;
			}
			
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_WRITTEN:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			
			if (objtype == NULL) {
				num_client_main_written++;
			} else {
				fail_unless(!strncmp(objtype, "mockobjtype", 11), NULL);
				num_client_written++;
			}
			
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_ERROR:
			fail_unless(osync_error_is_set(&(error)), NULL);
			num_client_errors++;
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_SYNC_DONE:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			
			if (objtype == NULL) {
				num_client_main_sync_done++;
			} else {
				fail_unless(!strncmp(objtype, "mockobjtype", 11), NULL);
				num_client_sync_done++;
			}
			
			break;
		case OSYNC_ENGINE_MEMBER_EVENT_DISCOVERED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_client_discovered++;
			break;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void entry_status(OSyncEngineChangeUpdate *status, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, status, user_data);

        OSyncError *error = osync_engine_change_update_get_error(status);
	
	switch (osync_engine_change_update_get_event(status)) {
		case OSYNC_ENGINE_CHANGE_EVENT_READ:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_change_read++;
			break;
		case OSYNC_ENGINE_CHANGE_EVENT_WRITTEN:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_change_written++;
			break;
		case OSYNC_ENGINE_CHANGE_EVENT_ERROR:
			fail_unless(osync_error_is_set(&(error)), NULL);
			num_change_error++;
			break;
	}
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void engine_status(OSyncEngineUpdate *status, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, status, user_data);

        OSyncError *error = osync_engine_update_get_error(status);

	switch (osync_engine_update_get_event(status)) {
		case OSYNC_ENGINE_EVENT_CONNECTED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_connected++;
			break;
		case OSYNC_ENGINE_EVENT_CONNECT_DONE:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_connect_done++;
			break;
		case OSYNC_ENGINE_EVENT_READ:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_read++;
			break;
		case OSYNC_ENGINE_EVENT_PREPARED_MAP:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_prepared_map++;
			break;
		case OSYNC_ENGINE_EVENT_MAPPED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_mapped++;
			break;
		case OSYNC_ENGINE_EVENT_MULTIPLIED: 
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_multiplied++;
			break;
		case OSYNC_ENGINE_EVENT_PREPARED_WRITE:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_prepared_write++;
			break;
		case OSYNC_ENGINE_EVENT_WRITTEN:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_written++;
			break;
		case OSYNC_ENGINE_EVENT_DISCONNECTED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_disconnected++;
			break;
		case OSYNC_ENGINE_EVENT_ERROR:
			fail_unless(osync_error_is_set(&(error)), NULL);
			num_engine_errors++;
			break;
		case OSYNC_ENGINE_EVENT_SUCCESSFUL:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_successful++;
			break;
		case OSYNC_ENGINE_EVENT_PREV_UNCLEAN:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_prev_unclean++;
			break;
		case OSYNC_ENGINE_EVENT_END_CONFLICTS:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_end_conflicts++;
			break;
		case OSYNC_ENGINE_EVENT_SYNC_DONE:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_engine_sync_done++;
			break;
	}

	osync_trace(TRACE_EXIT, "%s", __func__);
}

void mapping_status(OSyncEngineMappingUpdate *status, void *user_data)
{
        OSyncError *error = osync_engine_mapping_update_get_error(status);

	switch (osync_engine_mapping_update_get_event(status)) {
		case OSYNC_ENGINE_MAPPING_EVENT_SOLVED:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_mapping_solved++;
			break;
		/*case OSYNC_ENGINE_MAPPING_EVENT_WRITTEN:
			fail_unless(!osync_error_is_set(&(error)), NULL);
			num_mapping_written++;
			break;*/
		case OSYNC_ENGINE_MAPPING_EVENT_ERROR:
			fail_unless(osync_error_is_set(&(error)), NULL);
			num_mapping_errors++;
			break;
	}
}


void conflict_handler_choose_member(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncChange *change = NULL;
	OSyncError *error = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);
	
	num_mapping_conflicts++;
	fail_unless(num_engine_end_conflicts == 0, NULL);

	change = osync_mapping_engine_member_change(mapping, GPOINTER_TO_UINT(user_data));
	osync_assert(osync_engine_mapping_solve(engine, mapping, change, &error));
	osync_assert(error == NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void conflict_handler_choose_first(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncChange *change = NULL;
	OSyncError *error = NULL;

	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);
	
	num_mapping_conflicts++;
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);

	change = osync_mapping_engine_member_change(mapping, 1);
	osync_assert(osync_engine_mapping_solve(engine, mapping, change, &error));
	osync_assert(error == NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void conflict_handler_choose_deleted(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncError *error = NULL;
	unsigned int i;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);
	
	num_mapping_conflicts++;
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);

	for (i = 0; i < osync_mapping_engine_num_changes(mapping); i++) {
		OSyncChange *change = osync_mapping_engine_nth_change(mapping, i);
		if (osync_change_get_changetype(change) == OSYNC_CHANGE_TYPE_DELETED) {
			osync_assert(osync_engine_mapping_solve(engine, mapping, change, &error));
			osync_assert(error == NULL);
			
			osync_trace(TRACE_EXIT, "%s", __func__);
			return;
		}
	}
	fail(NULL);
}

void conflict_handler_choose_modified(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncError *error = NULL;
	unsigned int i;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);

	num_mapping_conflicts++;
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);

	for (i = 0; i < osync_mapping_engine_num_changes(mapping); i++) {
		OSyncChange *change = osync_mapping_engine_nth_change(mapping, i);
		if (osync_change_get_changetype(change) == OSYNC_CHANGE_TYPE_MODIFIED) {
			osync_assert(osync_mapping_engine_solve(mapping, change, &error));
			osync_assert(error == NULL);
			osync_trace(TRACE_EXIT, "%s", __func__);
			return;
		}
	}
	fail("");
}

void conflict_handler_ignore(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncError *error = NULL;
	num_mapping_conflicts++;
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);
	
	osync_engine_mapping_ignore_conflict(engine, mapping, &error);
	fail_unless(error == NULL, NULL);
}

void conflict_handler_duplicate(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncError *error = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);
	
	num_mapping_conflicts++;
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);
	
	fail_unless(osync_engine_mapping_duplicate(engine, mapping, &error), NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void conflict_handler_abort(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	OSyncError *error = NULL;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);
	
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);
	
	fail_unless(osync_engine_abort(engine, &error), NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

OSyncEngine *gengine = NULL;

void solve_conflict(OSyncMappingEngine *mapping)
{
	OSyncChange *change = NULL;
	OSyncError *error = NULL;

	g_usleep(5*G_USEC_PER_SEC);
	
	change = osync_mapping_engine_nth_change(mapping, 0);
	osync_assert(osync_engine_mapping_solve(gengine, mapping, change, &error));
	osync_assert(error == NULL);
}

void conflict_handler_delay(OSyncEngine *engine, OSyncMappingEngine *mapping, void *user_data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, engine, mapping, user_data);
	
	num_mapping_conflicts++;
	if (user_data)
		fail_unless(osync_mapping_engine_num_changes(mapping) == GPOINTER_TO_UINT(user_data), NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);
	gengine = engine;
	g_thread_create ((GThreadFunc)solve_conflict, mapping, TRUE, NULL);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

void reset_counters()
{
	num_client_connected = 0;
	num_client_main_connected = 0;
	num_client_disconnected = 0;
	num_client_main_disconnected = 0;
	num_client_read = 0;
	num_client_main_read = 0;
	num_client_written = 0;
	num_client_main_written = 0;
	num_client_errors = 0;
	num_client_sync_done = 0;
	num_client_main_sync_done = 0;
	num_client_discovered = 0;
	
	num_change_read = 0;
	num_change_written = 0;
	num_change_error = 0;
	
	num_engine_connected = 0;
	num_engine_read = 0;
	num_engine_prepared_map = 0;
	num_engine_mapped = 0;
	num_engine_multiplied = 0;
	num_engine_written = 0;
	num_engine_disconnected = 0;
	num_engine_errors = 0;
	num_engine_successful = 0;
	num_engine_end_conflicts = 0;
	num_engine_prev_unclean = 0;
	num_engine_sync_done = 0;
	
	num_mapping_solved = 0;
	num_mapping_written = 0;
	num_mapping_errors = 0;
	
	num_mapping_conflicts = 0;
}

osync_bool synchronize_once(OSyncEngine *engine, OSyncError **error)
{
	reset_counters();
	fail_unless(osync_engine_repair(engine, error), "Repair of engine failed: %s", osync_error_print(error));
	return osync_engine_synchronize_and_block(engine, error);
}

void discover_all_once(OSyncEngine *engine, OSyncError **error)
{
	OSyncGroup *group = osync_engine_get_group(engine);
	int i, max = osync_group_num_members(group); 
	for (i=0; i < max; i++) {
		OSyncMember *member = osync_group_nth_member(group, i);
		osync_engine_discover_and_block(engine, member, error);
		osync_member_save(member, error);
	}
}

OSyncFormatEnv *osync_testing_load_formatenv(const char *formatdir)
{
	OSyncError *error = NULL;
	OSyncFormatEnv *formatenv = osync_format_env_new(&error);
	fail_unless(formatenv != NULL, NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(osync_format_env_load_plugins(formatenv, formatdir, &error));
	return formatenv;
}

/** @brief Check if file or directory exists. No check for regular file! 
 * 
 * @param file filename or fullpath of file/directory 
 * @returns TRUE if exists, FALSE otherwise
 * 
 */
osync_bool osync_testing_file_exists(const char *file)
{
	return g_file_test(file, G_FILE_TEST_EXISTS);
}

/** @brief Removes files and directories 
 * 
 * @param file filename or fullpath of file/directory 
 * @returns TRUE on success, FALSE otherwise
 * 
 */
osync_bool osync_testing_file_remove(const char *file)
{
	return g_remove(file);
}

/** @brief Modifies permission of file - like chmod() 
 * 
 * @param file filename or fullpath of file 
 * @param mode the permission mode like chmod()
 * @returns TRUE on success, FALSE otherwise
 * 
 */
osync_bool osync_testing_file_chmod(const char *file, int mode)
{
	/* GLib 2.16.3 Note: 
	   "[...] Software that needs to manage file 
	   permissions on Windows exactly should use the Win32 API."
	TODO: Do we have to care about this on Windows?! */
	return g_chmod(file, mode);
}

/** @brief Copy files
 *
 * @param source source filename
 * @param dest destination filename
 * @returns TRUE on success, FALSE otherwise
 *
 */
osync_bool osync_testing_file_copy(const char *source, const char *dest)
{
	gboolean ret;
	const char *argv[] = { "cp", source, dest, NULL };
	int exitstatus = -1;

	ret = g_spawn_sync(NULL,			/* working directory */
						(char **)argv,	/* arguments */
						NULL,			/* environment */
						G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL,
						NULL,			/* child setup function */
						NULL,			/* user data for child setup func */
						NULL,			/* stdin */
						NULL,			/* stdout */
						&exitstatus,	/* exit status */
						NULL			/* error function */
						);
#ifndef _WIN32
	return ret && WEXITSTATUS(exitstatus) == 0;
#else
	return ret;
#endif
}

/** @brief Find differences between two files
 *
 * @param source source filename
 * @param dest destination filename
 * @returns TRUE on when equal/success, FALSE otherwise
 *
 */
osync_bool osync_testing_diff(const char *file1, const char *file2)
{
	gchar *cmd;
	int ret;

	osync_assert(file1);
	osync_assert(file2);

	cmd = g_strdup_printf(DIFF " -x \".*\" %s %s", file1, file2);
	ret = osync_system(cmd);
	g_free(cmd);

	return !ret;
}

/**
 * @brief Test if the directory is empty 
 */
osync_bool osync_testing_directory_is_empty(const char *dirname)
{
	gchar *cmd;
	int ret;
	cmd = g_strdup_printf("ls %s | grep \".*\"", dirname);
	ret = osync_system(cmd);
	return (ret != 0);
}

/** @brief Creates a simple OSyncPluginConfig with a single resource.
 *         If config is not null the resource information gets added.
 * 
 * @param config OSyncPluginConfig pointer to add resource info
 * @param path relative path of resource
 * @param objformat the objformat of the resource
 * @param format_config the format converter config paramter
 * @returns OSyncPluginConfig pointer or asserts on error
 * 
 */
OSyncPluginConfig *simple_plugin_config(OSyncPluginConfig *config, const char *path, const char *objtype, const char *objformat, const char *format_config) {
	OSyncError *error = NULL;
	OSyncObjFormatSink *format_sink = NULL;
	OSyncPluginResource *res = NULL;
	osync_assert(objtype);
	osync_assert(objformat);

	if (!config)
		config = osync_plugin_config_new(&error);

	fail_unless(config != NULL, NULL);
	fail_unless(error == NULL, NULL);

	format_sink = osync_objformat_sink_new(objformat, &error);
	if (format_config)
		osync_objformat_sink_set_config(format_sink, format_config);

	res = osync_plugin_resource_new(&error);
	osync_plugin_resource_set_objtype(res, objtype);
	osync_plugin_resource_set_path(res, path); 
	osync_plugin_resource_enable(res, TRUE);
	osync_plugin_resource_add_objformat_sink(res, format_sink);


	osync_plugin_config_add_resource(config, res);

	return config;
}

/** @brief Creates a simple OSyncEngine with nth Members with mock-sync
 *         plugins.
 * 
 * @param member_size The number of member the Group for this Engine should have
 * @returns OSyncEngine pointer or asserts on error
 * 
 */
OSyncEngine *osync_testing_create_engine_dummy(unsigned int member_size)
{
	unsigned int u;
	OSyncError *error = NULL;
	OSyncEngine *engine = NULL;
	OSyncGroup *group = osync_group_new(&error);

	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	for (u=0; u < member_size; u++) {
		OSyncMember *member = osync_member_new(&error);
		fail_unless(member != NULL, NULL);
		fail_unless(error == NULL, NULL);

		osync_group_add_member(group, member);
	}

	engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	return engine;
}

void osync_testing_system_abort(const char *command)
{
	if (osync_system(command))
		abort();
}

