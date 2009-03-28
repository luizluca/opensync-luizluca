#include "support.h"
#include "engine_support.h"

#include "opensync/engine/opensync_engine_internals.h"
#include "opensync/engine/opensync_engine_private.h"

#include "opensync/group/opensync_group_internals.h"
#include "opensync/client/opensync_client_internals.h"

#include <opensync/opensync-group.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-client.h>
#include <opensync/opensync-engine.h>
#include <opensync/opensync-plugin.h>

#include "../mock-plugin/mock_sync.h"

static void _member_add_objtype(OSyncMember *member, const char *objtype)
{
	OSyncObjTypeSink *sink = NULL;
	osync_assert(member);
	osync_assert(objtype);

	if (!osync_member_find_objtype_sink(member, objtype)) {
		sink = osync_objtype_sink_new(objtype, NULL);
		osync_member_add_objtype_sink(member, sink);
		osync_objtype_sink_unref(sink);
	}
}

int num_connect = 0;
int num_disconnect = 0;
int num_get_changes = 0;

/*
static void connect1(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	g_atomic_int_inc(&(num_connect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}
*/

static void connect_error(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	g_atomic_int_inc(&(num_connect));
	
	osync_context_report_error(ctx, OSYNC_ERROR_GENERIC, "connect error");
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	g_atomic_int_inc(&(num_disconnect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	g_atomic_int_inc(&(num_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

/*
static void *initialize_error(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	osync_error_set(error, OSYNC_ERROR_GENERIC, "init error");
	
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void *initialize(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *formatsink = osync_objformat_sink_new("mockobjtype1", error);
	osync_objtype_sink_add_objformat_sink(sink, formatsink);
	
	OSyncObjTypeSinkFunctions functions;
	memset(&functions, 0, sizeof(functions));
	functions.connect = connect1;
	functions.disconnect = disconnect;
	functions.get_changes = get_changes;
	
	osync_objtype_sink_set_functions(sink, functions, NULL);
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);

	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}
*/

static void *initialize_connect_error(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	osync_assert(env);
	OSyncList *objtypesinks = osync_plugin_info_get_objtype_sinks(info);
	OSyncObjTypeSink *sink = (OSyncObjTypeSink*)objtypesinks->data;
	osync_assert(sink);
	
	OSyncObjFormatSink *formatsink = osync_objformat_sink_new("mockformat1", error);
	osync_objtype_sink_add_objformat_sink(sink, formatsink);
	
	OSyncObjTypeSinkFunctions functions;
	memset(&functions, 0, sizeof(functions));
	functions.connect = connect_error;
	functions.disconnect = disconnect;
	functions.get_changes = get_changes;
	
	osync_objtype_sink_set_functions(sink, functions, NULL);
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);

	osync_list_free(objtypesinks);
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;
}

static void finalize(void *data)
{
	mock_env *env = data;
	
	g_free(env);
}

static OSyncDebugGroup *_create_group5(char *testbed)
{
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	char *path = g_strdup_printf("%s/configs/group", testbed);
	osync_group_set_configdir(debug->group, path);
	g_free(path);
	
	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);

	_member_add_objtype(debug->member1, "mockobjtype1");
	OSyncPluginConfig *config1 = simple_plugin_config(NULL, "data1", "mockobjtype1", "mockformat1", NULL);
	osync_member_set_config(debug->member1, config1);
	osync_plugin_config_unref(config1);

	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);
	_member_add_objtype(debug->member2, "mockobjtype1");
	OSyncPluginConfig *config2 = simple_plugin_config(NULL, "data2", "mockobjtype1", "mockformat1", NULL);
	osync_member_set_config(debug->member2, config2);
	osync_plugin_config_unref(config2);

	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize(debug->plugin, initialize_connect_error);
	osync_plugin_set_finalize(debug->plugin, finalize);
	
	
	debug->plugin2 = osync_plugin_new(&error);
	fail_unless(debug->plugin2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin2, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin2, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin2, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin2, OSYNC_START_TYPE_EXTERNAL);
	
	osync_plugin_set_initialize(debug->plugin2, initialize_connect_error);
	osync_plugin_set_finalize(debug->plugin2, finalize);
	
	
	debug->client1 = osync_client_new(&error);
	fail_unless(debug->client1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	char *pipe_path = g_strdup_printf("%s/configs/group/1/pluginpipe", testbed);
	osync_client_run_external(debug->client1, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	debug->client2 = osync_client_new(&error);
	fail_unless(debug->client2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	pipe_path = g_strdup_printf("%s/configs/group/2/pluginpipe", testbed);
	osync_client_run_external(debug->client2, pipe_path, debug->plugin2, &error);
	g_free(pipe_path);
	
	return debug;
}

static void _free_group(OSyncDebugGroup *debug)
{
	if (debug->client1)
		osync_client_unref(debug->client1);

	if (debug->client2)
		osync_client_unref(debug->client2);
	
	if (debug->plugin)
		osync_plugin_unref(debug->plugin);

	if (debug->plugin2)
		osync_plugin_unref(debug->plugin2);
	
	osync_member_unref(debug->member1);
	osync_member_unref(debug->member2);
	osync_group_unref(debug->group);
	
	g_free(debug);
}

static void _engine_instrument_pluginenv(OSyncEngine *engine, OSyncDebugGroup *debug)
{
	fail_unless(engine->pluginenv == NULL, NULL);
	engine->pluginenv = osync_plugin_env_new(NULL);

	if (debug->plugin)
		osync_plugin_env_register_plugin(engine->pluginenv, debug->plugin);

	if (debug->plugin2)
		osync_plugin_env_register_plugin(engine->pluginenv, debug->plugin2);
}

START_TEST (engine_error_single_init_error)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error); 
	fail_unless(error == NULL, NULL);
		
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	/* This will trigger an error in the mock-sync plugin of member 2 */
	/* in the initialize function                                     */
	g_setenv("INIT_NULL", "2", TRUE);
	
	fail_unless(!osync_engine_initialize(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(strcmp("Triggering INIT_NULL error", osync_error_print(&error)) == 0, NULL);
	osync_error_unref(&error);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	osync_engine_unref(engine);
	osync_group_unref(group);	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_double_init_error)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error); 
	fail_unless(error == NULL, NULL);
		
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	/* This triggers an error in plugin 1 and 2 */
	g_setenv("INIT_NULL", "3", TRUE);
	
	fail_unless(!osync_engine_initialize(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	if (error)
		fail_unless(strcmp("Triggering INIT_NULL error", osync_error_print(&error)) == 0, NULL);
	osync_error_unref(&error);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	osync_engine_unref(engine);
	osync_group_unref(group);	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_no_config_error)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	fail_unless(g_unlink("configs/group/2/mock-sync.conf") == 0, NULL);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error); 
	fail_unless(error == NULL, NULL);
		
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	fail_unless(!osync_engine_initialize(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	if (error)
		fail_unless(strcmp("Plugin is not configured", osync_error_print(&error)) == 0, NULL);
	osync_error_unref(&error);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	osync_engine_unref(engine);
	osync_group_unref(group);	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST


START_TEST (engine_error_no_objtype_error)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	fail_unless(g_unlink("configs/group/1/mock-sync.conf") == 0, NULL);
	fail_unless(g_rename("configs/group/1/mock-sync.no-objtype-error.conf",
			"configs/group/1/mock-sync.conf") == 0, NULL);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error); 
	fail_unless(error == NULL, NULL);
		
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	osync_error_unref(&error);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);

	fail_unless(osync_engine_finalize(engine, &error), NULL);
	osync_engine_unref(engine);
	osync_group_unref(group);	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST


/* FIXME: Temprorarly disabled until reviewed/fixed - see #995 */
START_TEST (engine_error_dual_connect_error)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group5(testbed);
	
	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);
	
	_engine_instrument_pluginenv(engine, debug);

	osync_engine_set_conflict_callback(engine, conflict_handler_choose_first, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	osync_error_unref(&error);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(num_connect == 2, NULL);
	fail_unless(num_disconnect == 0, NULL);
	fail_unless(num_get_changes == 0, NULL);
	
	/* Client checks */
	fail_unless(num_client_connected == 0, NULL);
	fail_unless(num_client_read == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_sync_done == 0, NULL);
	fail_unless(num_client_discovered == 0, NULL);

	/* Main sink checks */

	fail_unless(num_client_main_disconnected == 2, NULL);
	fail_unless(num_client_main_connected == 2, NULL);

	fail_unless(num_client_main_read == 0, NULL);
	fail_unless(num_client_main_written == 0, NULL);
	fail_unless(num_client_main_sync_done == 0, NULL);
	
	/* Engine checks */
	fail_unless(num_engine_connected == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_read == 0, NULL);
	fail_unless(num_engine_written == 0, NULL);
	fail_unless(num_engine_sync_done == 0, NULL);

	/* Engine always disconnects - even on an error.
	 * See _osync_engine_generate_disconnected_event()
	 *
	 * This is just the call of the event callback,
	 * in theory no disconnect function of proxies get called.
	 */
	fail_unless(num_engine_disconnected == 1, NULL);

	fail_unless(num_engine_successful == 0, NULL);
	fail_unless(num_engine_end_conflicts == 0, NULL);
	fail_unless(num_engine_prev_unclean == 0, NULL);

	/* Change checks */
	fail_unless(num_change_read == 0, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 0, NULL);

	/* Mapping checks */
	fail_unless(num_mapping_solved == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_one_of_two_connect_error)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	if (system("cp testdata data1/testdata"))
		abort();
	
	g_setenv("CONNECT_ERROR", "1", TRUE);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error); 
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);

	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);

	osync_error_unref(&error);

	fail_unless(osync_engine_finalize(engine, &error), NULL);	
	osync_engine_unref(engine);
	osync_group_unref(group);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);

	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_two_of_three_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_ERROR", "5", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);

	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_two_of_three_connect_error2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);

	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(!osync_testing_diff("data1", "data2"));
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_three_of_three_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_ERROR", "7", TRUE);
	
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 0, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_one_of_three_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_client_disconnected == 2, NULL);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_no_connect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_ERROR", "0", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_connect_timeout)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	if (system("cp testdata data1/testdata"))
		abort();
	
	g_setenv("CONNECT_TIMEOUT", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_connect_timeout)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	if (system("cp testdata data1/testdata"))
		abort();
	
	g_setenv("CONNECT_TIMEOUT", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 0, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, "Number of Engine Errors: %u Expected: 1", num_engine_errors);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_one_of_three_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_TIMEOUT", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("CONNECT_TIMEOUT", "2", TRUE);
	g_setenv("CONNECT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 1, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_get_changes_error)
{
	char *testbed = setup_testbed("sync_easy_conflict");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("GET_CHANGES_ERROR", "2", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);

	fail_unless(num_engine_errors == 1, NULL);

	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_get_changes_error)
{
	char *testbed = setup_testbed("sync_easy_conflict");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	g_setenv("GET_CHANGES_ERROR", "3", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 2, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_change_read == 0, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);

	fail_unless(num_engine_errors == 1, NULL);

	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_two_of_three_get_changes_error)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("GET_CHANGES_ERROR", "5", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);

	fail_unless(num_engine_errors == 1, NULL);

	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_one_of_three_get_changes_error)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("GET_CHANGES_ERROR", "1", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);

	fail_unless(num_engine_errors == 1, NULL);

	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_one_of_three_get_changes_timeout)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("GET_CHANGES_TIMEOUT", "1", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_read == 2, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_change_read == 2, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_get_changes_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	g_setenv("GET_CHANGES_TIMEOUT", "3", TRUE);
	g_setenv("GET_CHANGES_ERROR", "4", TRUE);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_read == 0, NULL);
	fail_unless(num_client_written == 0, NULL);
	fail_unless(num_change_read == 0, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST


/* If get_changes delays and got timed out ... tainted proxy with an error.
 *
 * Make sure changes from the plugin got completely ignored by the engine when the timout handler got called.
 * Even better would be to abort the get_changes call from the plugin process...
 *
*/
START_TEST (engine_error_get_changes_timeout_sleep)
{
	char *testbed = setup_testbed("multisync_conflict_data_choose2");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("GET_CHANGES_TIMEOUT2", "7", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_read == 0, NULL);
	fail_unless(num_client_written == 0, NULL);

	/* If get_changes delays and get timed out .. proxy get tainted with an error.
	 * To make sure changes got completely ignored by the engine.
	 */
	fail_unless(num_change_read == 0, NULL);

	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_commit_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMIT_ERROR", "4", TRUE);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 1, NULL);

	// TODO: Review, whats the diffent of mapping status emits and change status emits?! (dgollub)
	fail_unless(num_change_error == 1, NULL);
	// TODO: Review, whats the diffent of mapping status emits and change status emits?! (dgollub)
	fail_unless(num_mapping_errors == 1, NULL);

	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_commit_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMIT_ERROR", "6", TRUE);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_commit_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMIT_TIMEOUT", "4", TRUE);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 1, NULL);
	fail_unless(num_change_error == 1, NULL);
	fail_unless(num_mapping_errors == 1, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);

	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_commit_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMIT_TIMEOUT", "6", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_commit_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMIT_TIMEOUT", "4", TRUE);
	g_setenv("COMMIT_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_commit_timeout_and_error2)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMIT_TIMEOUT", "2", TRUE);
	g_setenv("COMMIT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

/* FIXME: timeout */
START_TEST (engine_error_commit_error_modify)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	g_setenv("COMMIT_TIMEOUT", "2", TRUE);
	g_setenv("COMMIT_ERROR", "4", TRUE);

	g_usleep(2*G_USEC_PER_SEC);
	
	if (system("cp newdata2 data1/testdata"))
		abort();
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));
	fail_unless(osync_testing_diff("data2", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_commit_error_delete)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	g_setenv("COMMIT_TIMEOUT", "2", TRUE);
	g_setenv("COMMIT_ERROR", "4", TRUE);

	g_usleep(2*G_USEC_PER_SEC);
	
	if (system("rm -f data1/testdata"))
		abort();
	
	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 0, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 0, NULL);
	fail_unless(num_change_error == 2, NULL);
	fail_unless(num_mapping_errors == 2, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));
	fail_unless(osync_testing_diff("data2", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_committed_all_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("COMMITTED_ALL_ERROR", "3", TRUE);

	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 1, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_committed_all_batch_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("BATCH_COMMIT", "7", TRUE);
	g_setenv("COMMITTED_ALL_ERROR", "3", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 1, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_sync_done_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("SYNC_DONE_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_sync_done_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("SYNC_DONE_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_triple_sync_done_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("SYNC_DONE_ERROR", "7", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_sync_done_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("SYNC_DONE_TIMEOUT", "4", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_sync_done_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("SYNC_DONE_TIMEOUT", "6", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, "Number of Engine Errors: %u Expected: 1", num_engine_errors);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_sync_done_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("SYNC_DONE_TIMEOUT", "5", TRUE);
	g_setenv("SYNC_DONE_ERROR", "2", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 3, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 1, NULL);
	fail_unless(num_engine_successful == 0, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_ERROR", "4", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_ERROR", "6", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);

	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_triple_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_ERROR", "7", TRUE);

	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_single_disconnect_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_TIMEOUT", "4", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 1, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 2, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_dual_disconnect_timeout)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_TIMEOUT", "6", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), "Unexpected error: %s", osync_error_print(&error));
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 2, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 1, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_disconnect_timeout_and_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_TIMEOUT", "5", TRUE);
	g_setenv("DISCONNECT_ERROR", "2", TRUE);

	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(synchronize_once(engine, &error), NULL);
	fail_unless(!osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(num_client_errors == 3, NULL);
	fail_unless(num_client_connected == 3, NULL);
	fail_unless(num_client_disconnected == 0, NULL);
	fail_unless(num_client_written == 3, NULL);
	fail_unless(num_change_read == 1, NULL);
	fail_unless(num_change_written == 2, NULL);
	fail_unless(num_change_error == 0, NULL);
	fail_unless(num_mapping_errors == 0, NULL);
	fail_unless(num_mapping_conflicts == 0, NULL);
	fail_unless(num_engine_errors == 0, NULL);
	fail_unless(num_engine_successful == 1, NULL);
	
	fail_unless(osync_testing_diff("data1", "data2"));
	fail_unless(osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_get_changes_disconnect_error)
{
	char *testbed = setup_testbed("multisync_easy_new");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);
	
	g_setenv("DISCONNECT_TIMEOUT", "1", TRUE);
	g_setenv("DISCONNECT_ERROR", "2", TRUE);
	g_setenv("GET_CHANGES_TIMEOUT", "6", TRUE);
	g_setenv("NO_COMMITTED_ALL_CHECK", "1", TRUE);
	
	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	discover_all_once(engine, &error);

	osync_engine_set_memberstatus_callback(engine, member_status, GINT_TO_POINTER(1));
	osync_engine_set_enginestatus_callback(engine, engine_status, GINT_TO_POINTER(1));
	osync_engine_set_changestatus_callback(engine, entry_status, GINT_TO_POINTER(1));
	osync_engine_set_mappingstatus_callback(engine, mapping_status, GINT_TO_POINTER(1));
	osync_engine_set_conflict_callback(engine, conflict_handler_choose_modified, GINT_TO_POINTER(3));

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);

	fail_unless(!synchronize_once(engine, &error), NULL);
	fail_unless(osync_error_is_set(&error), NULL);
	
	osync_error_unref(&error);
	osync_engine_finalize(engine, &error);
	osync_engine_unref(engine);
	
	fail_unless(!osync_testing_diff("data1", "data2"));
	fail_unless(!osync_testing_diff("data1", "data3"));

	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_error_missing_format_plugin)
{
	char *testbed = setup_testbed("missing_format_plugin");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	char *plugindir = g_strdup_printf("%s/plugins",  testbed);

	OSyncError *error = NULL;

	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error);
	fail_unless(error == NULL, NULL);

	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	fail_unless(osync_engine_initialize(engine, &error) == FALSE, NULL); /* Intended fail */
	fail_unless(osync_error_is_set(&error), "No expected error about MISSINGFORMAT!");
	
	osync_error_unref(&error);
	osync_engine_unref(engine);
	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);

}
END_TEST

START_TEST (engine_error_discover_error)
{
	char *testbed = setup_testbed("sync");
	char *formatdir = g_strdup_printf("%s/formats", testbed);
	char *plugindir = g_strdup_printf("%s/plugins", testbed);

	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	osync_group_set_schemadir(group, testbed);
	osync_group_load(group, "configs/group", &error); 
	fail_unless(error == NULL, NULL);

	OSyncMember *member = osync_group_nth_member(group, 1);
		
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	/* This will trigger an error in the mock-sync plugin of member 1 */
	/* in the discover function                                     */
	g_setenv("MOCK_DISCOVER_ERROR", "1", TRUE);
	
	fail_unless(!osync_engine_discover_and_block(engine, member, &error), NULL);
	fail_unless(error != NULL, NULL);
	fail_unless(strcmp("MOCK_DISCOVER_ERROR on purpose!", osync_error_print(&error)) == 0, NULL);
	osync_error_unref(&error);
	
	osync_engine_unref(engine);
	osync_group_unref(group);	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST


OSYNC_TESTCASE_START("engine_error")
OSYNC_TESTCASE_ADD(engine_error_single_init_error)
OSYNC_TESTCASE_ADD(engine_error_double_init_error)

OSYNC_TESTCASE_ADD(engine_error_no_config_error)
OSYNC_TESTCASE_ADD(engine_error_no_objtype_error)

OSYNC_TESTCASE_ADD(engine_error_dual_connect_error)
OSYNC_TESTCASE_ADD(engine_error_one_of_two_connect_error)
OSYNC_TESTCASE_ADD(engine_error_two_of_three_connect_error)
OSYNC_TESTCASE_ADD(engine_error_two_of_three_connect_error2)
OSYNC_TESTCASE_ADD(engine_error_three_of_three_connect_error)

OSYNC_TESTCASE_ADD(engine_error_one_of_three_connect_error)
OSYNC_TESTCASE_ADD(engine_error_no_connect_error)

OSYNC_TESTCASE_ADD(engine_error_single_connect_timeout)
OSYNC_TESTCASE_ADD(engine_error_dual_connect_timeout)
OSYNC_TESTCASE_ADD(engine_error_one_of_three_timeout)
OSYNC_TESTCASE_ADD(engine_error_timeout_and_error)

OSYNC_TESTCASE_ADD(engine_error_single_get_changes_error)
OSYNC_TESTCASE_ADD(engine_error_dual_get_changes_error)
OSYNC_TESTCASE_ADD(engine_error_two_of_three_get_changes_error)
OSYNC_TESTCASE_ADD(engine_error_one_of_three_get_changes_error)

OSYNC_TESTCASE_ADD(engine_error_one_of_three_get_changes_timeout)
OSYNC_TESTCASE_ADD(engine_error_get_changes_timeout_and_error)
OSYNC_TESTCASE_ADD(engine_error_get_changes_timeout_sleep)

OSYNC_TESTCASE_ADD(engine_error_single_commit_error)
OSYNC_TESTCASE_ADD(engine_error_dual_commit_error)

OSYNC_TESTCASE_ADD(engine_error_single_commit_timeout)
OSYNC_TESTCASE_ADD(engine_error_dual_commit_timeout)
OSYNC_TESTCASE_ADD(engine_error_commit_timeout_and_error)
OSYNC_TESTCASE_ADD(engine_error_commit_timeout_and_error2)

OSYNC_TESTCASE_ADD(engine_error_commit_error_modify)
OSYNC_TESTCASE_ADD(engine_error_commit_error_delete)

OSYNC_TESTCASE_ADD(engine_error_committed_all_error)
OSYNC_TESTCASE_ADD(engine_error_committed_all_batch_error)

OSYNC_TESTCASE_ADD(engine_error_single_sync_done_error)
OSYNC_TESTCASE_ADD(engine_error_dual_sync_done_error)
OSYNC_TESTCASE_ADD(engine_error_triple_sync_done_error)

OSYNC_TESTCASE_ADD(engine_error_single_sync_done_timeout)
OSYNC_TESTCASE_ADD(engine_error_dual_sync_done_timeout)
OSYNC_TESTCASE_ADD(engine_error_sync_done_timeout_and_error)

OSYNC_TESTCASE_ADD(engine_error_single_disconnect_error)
OSYNC_TESTCASE_ADD(engine_error_dual_disconnect_error)
OSYNC_TESTCASE_ADD(engine_error_triple_disconnect_error)

OSYNC_TESTCASE_ADD(engine_error_single_disconnect_timeout)
OSYNC_TESTCASE_ADD(engine_error_dual_disconnect_timeout)
OSYNC_TESTCASE_ADD(engine_error_disconnect_timeout_and_error)

OSYNC_TESTCASE_ADD(engine_error_get_changes_disconnect_error)

OSYNC_TESTCASE_ADD(engine_error_missing_format_plugin)

OSYNC_TESTCASE_ADD(engine_error_discover_error)

OSYNC_TESTCASE_END

