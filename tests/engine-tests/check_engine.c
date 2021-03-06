#include "support.h"
#include "engine_support.h"

#include <opensync/opensync-group.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-client.h>
#include <opensync/opensync-engine.h>
#include <opensync/opensync-plugin.h>

#include "opensync/engine/opensync_engine_internals.h"
#include "opensync/engine/opensync_engine_private.h"

#include "opensync/group/opensync_member_internals.h"
#include "opensync/client/opensync_client_internals.h"

#include "../mock-plugin/mock_sync.h"
#include "../mock-plugin/mock_format.h"

static void _member_add_format(OSyncMember *member, const char *objtype, const char *objformat)
{
	OSyncObjTypeSink *sink = NULL;
	OSyncError *error = NULL;
	osync_assert(member);
	osync_assert(objtype);

	if (!osync_member_find_objtype_sink(member, objtype)) {
		sink = osync_objtype_sink_new(objtype, &error);
		osync_member_add_objtype_sink(member, sink);
		osync_objtype_sink_unref(sink);
	}

	osync_member_add_objformat(member, objtype, objformat, &error);
}

START_TEST (engine_new)
{
	char *testbed = setup_testbed(NULL);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(group, testbed);
	
	OSyncMember *member1 = osync_member_new(&error);
	fail_unless(member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(group, member1);
	
	OSyncMember *member2 = osync_member_new(&error);
	fail_unless(member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(group, member2);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_engine_ref(engine);
	osync_engine_unref(engine);
	osync_engine_unref(engine);

	osync_member_unref(member1);
	osync_member_unref(member2);
	osync_group_unref(group);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_init)
{
	char *path = NULL;
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	char *plugindir = g_strdup_printf("%s/plugins",  testbed);
	
	OSyncError *error = NULL;
	OSyncGroup *group = osync_group_new(&error);
	fail_unless(group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(group, testbed);
	
	OSyncMember *member1 = osync_member_new(&error);
	fail_unless(member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(group, member1);
	_member_add_format(member1, "mockobjtype1", "mockformat1");
	osync_member_set_pluginname(member1, "mock-sync");
	path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(member1, path);
	osync_member_set_schemadir(member1, testbed);
	g_free(path);
	
	OSyncMember *member2 = osync_member_new(&error);
	fail_unless(member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(group, member2);
	_member_add_format(member2, "mockobjtype1", "mockformat1");
	osync_member_set_pluginname(member2, "mock-sync");
	path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(member2, path);
	osync_member_set_schemadir(member2, testbed);
	g_free(path);
	
	OSyncEngine *engine = osync_engine_new(group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_engine_set_schemadir(engine, testbed);
	osync_engine_set_plugindir(engine, plugindir);
	osync_engine_set_formatdir(engine, formatdir);

	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	osync_engine_unref(engine);

	osync_member_unref(member1);
	osync_member_unref(member2);

	osync_group_unref(group);
	
	g_free(formatdir);
	g_free(plugindir);
	
	destroy_testbed(testbed);
}
END_TEST

static void connect1(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 0);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 0);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->num_connect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 1);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 1);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->num_disconnect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 1);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 0);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->num_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
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
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockobjtype1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect1);
	osync_objtype_sink_set_disconnect_func(sink, disconnect);
	osync_objtype_sink_set_get_changes_func(sink, get_changes);

	/* ObjTypeSink port - functions and userdata never got set! */
	osync_objtype_sink_set_userdata(sink, env);
	if (osync_objtype_sink_get_userdata(sink) != env)
		goto error;

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);

	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize(void *data)
{
	mock_env *env = data;
	
	osync_assert(env);
	osync_assert(env->num_connect == 1);
	osync_assert(env->num_disconnect == 1);
	osync_assert(env->num_get_changes == 1);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_free(env);
}

static OSyncDebugGroup *_create_group(char *testbed)
{
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(debug->group, testbed);
	
	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);

	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize_func(debug->plugin, initialize);
	osync_plugin_set_finalize_func(debug->plugin, finalize);
	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	return debug;
}

static void _free_group(OSyncDebugGroup *debug)
{
	osync_client_unref(debug->client1);
	osync_client_unref(debug->client2);
	
	osync_plugin_unref(debug->plugin);
	
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
		osync_plugin_env_register_plugin(engine->pluginenv, debug->plugin, NULL);

	if (debug->plugin2)
		osync_plugin_env_register_plugin(engine->pluginenv, debug->plugin2, NULL);
}

START_TEST (engine_sync)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group(testbed);
	
	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	osync_engine_set_schemadir(engine, testbed);

	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);

	_engine_instrument_pluginenv(engine, debug);
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

static void connect2(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect < 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 0);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->num_connect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void disconnect2(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect < 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	
	g_atomic_int_inc(&(env->num_disconnect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void get_changes2(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes < 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->num_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_connect2(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 0);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 0);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->main_connect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_disconnect2(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	
	g_atomic_int_inc(&(env->main_disconnect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_get_changes2(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->main_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void *initialize_multi(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockformat1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect2);
	osync_objtype_sink_set_disconnect_func(sink, disconnect2);
	osync_objtype_sink_set_get_changes_func(sink, get_changes2);

	osync_objtype_sink_set_userdata(sink, env);
	
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	sink = osync_objtype_sink_new("mockobjtype2", error);
	if (!sink)
		goto error;
	
	format_sink = osync_objformat_sink_new("vcard", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);

	osync_objtype_sink_set_connect_func(sink, connect2);
	osync_objtype_sink_set_disconnect_func(sink, disconnect2);
	osync_objtype_sink_set_get_changes_func(sink, get_changes2);

	osync_objtype_sink_set_userdata(sink, env);
	
	
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	sink = osync_objtype_sink_new("mockobjtype3", error);
	if (!sink)
		goto error;
	
	format_sink = osync_objformat_sink_new("plain", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);

	osync_objtype_sink_set_connect_func(sink, connect2);
	osync_objtype_sink_set_disconnect_func(sink, disconnect2);
	osync_objtype_sink_set_get_changes_func(sink, get_changes2);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	/* The main sink */
	sink = osync_objtype_main_sink_new(error);
	if (!sink)
		goto error;
	
	osync_objtype_sink_set_connect_func(sink, main_connect2);
	osync_objtype_sink_set_disconnect_func(sink, main_disconnect2);
	osync_objtype_sink_set_get_changes_func(sink, main_get_changes2);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_set_main_sink(info, sink);
	osync_objtype_sink_unref(sink);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize_multi(void *data)
{
	mock_env *env = data;
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 1);
	osync_assert(env->main_get_changes == 1);
	
	g_free(env);
}

static OSyncDebugGroup *_create_group2(char *testbed)
{
	osync_trace(TRACE_ENTRY, "%s(%s)", __func__, testbed);
	
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_group_set_configdir(debug->group, testbed);

	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);
	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member1, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member1, "mockobjtype3", "mockformat3");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member2, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member2, "mockobjtype3", "mockformat3");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize_func(debug->plugin, initialize_multi);
	osync_plugin_set_finalize_func(debug->plugin, finalize_multi);
	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, debug);
	return debug;
}

START_TEST (engine_sync_multi_obj)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group2(testbed);

	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);

	_engine_instrument_pluginenv(engine, debug);
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);

}
END_TEST

static void connect3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect < 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 0);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	env->ctx[env->num_connect] = ctx;
	osync_context_report_success(ctx);
	
	g_atomic_int_inc(&(env->num_connect));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void disconnect3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect < 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->num_sync_done == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	osync_assert(env->main_sync_done == 1);
	
	env->ctx[env->num_disconnect] = ctx;
	osync_context_ref(ctx);
	
	g_atomic_int_inc(&(env->num_disconnect));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void get_changes3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes < 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	env->ctx[env->num_get_changes] = ctx;
	osync_context_ref(ctx);
	
	g_atomic_int_inc(&(env->num_get_changes));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void sync_done3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->num_sync_done < 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	osync_assert(env->main_sync_done == 0);
	
	env->ctx[env->num_sync_done] = ctx;
	osync_context_ref(ctx);
	
	g_atomic_int_inc(&(env->num_sync_done));
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_connect3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 0);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 0);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->main_connect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_disconnect3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->num_sync_done == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	osync_assert(env->main_sync_done == 1);
	
	g_atomic_int_inc(&(env->main_disconnect));
	
	osync_context_report_success(env->ctx[0]);
	osync_context_report_success(env->ctx[2]);
	osync_context_report_success(ctx);
	osync_context_report_success(env->ctx[1]);
	
	osync_context_unref(env->ctx[0]);
	osync_context_unref(env->ctx[1]);
	osync_context_unref(env->ctx[2]);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_get_changes3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_atomic_int_inc(&(env->main_get_changes));
	
	
	osync_context_report_success(ctx);
	osync_context_report_success(env->ctx[2]);
	osync_context_report_success(env->ctx[1]);
	osync_context_report_success(env->ctx[0]);
	
	osync_context_unref(env->ctx[0]);
	osync_context_unref(env->ctx[1]);
	osync_context_unref(env->ctx[2]);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void main_sync_done3(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 0);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->num_sync_done == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	osync_assert(env->main_sync_done == 0);
	
	g_atomic_int_inc(&(env->main_sync_done));
	
	
	osync_context_report_success(ctx);
	osync_context_report_success(env->ctx[2]);
	osync_context_report_success(env->ctx[1]);
	osync_context_report_success(env->ctx[0]);
	
	osync_context_unref(env->ctx[0]);
	osync_context_unref(env->ctx[1]);
	osync_context_unref(env->ctx[2]);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void *initialize_order(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockobjtype1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect3);
	osync_objtype_sink_set_disconnect_func(sink, disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, sync_done3);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	sink = osync_objtype_sink_new("mockobjtype2", error);
	if (!sink)
		goto error;
	
	format_sink = osync_objformat_sink_new("vcard", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);

	osync_objtype_sink_set_connect_func(sink, connect3);
	osync_objtype_sink_set_disconnect_func(sink, disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, sync_done3);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	sink = osync_objtype_sink_new("mockobjtype3", error);
	if (!sink)
		goto error;
	
	format_sink = osync_objformat_sink_new("plain", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect3);
	osync_objtype_sink_set_disconnect_func(sink, disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, sync_done3);

	osync_objtype_sink_set_userdata(sink, env);
	
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	/* The main sink */
	sink = osync_objtype_main_sink_new(error);
	if (!sink)
		goto error;
	
	osync_objtype_sink_set_connect_func(sink, main_connect3);
	osync_objtype_sink_set_disconnect_func(sink, main_disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, main_get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, main_sync_done3);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_set_main_sink(info, sink);
	osync_objtype_sink_unref(sink);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize_order(void *data)
{
	mock_env *env = data;
	
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->num_sync_done == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 1);
	osync_assert(env->main_get_changes == 1);
	osync_assert(env->main_sync_done == 1);
	
	g_free(env);
}

static OSyncDebugGroup *_create_group3(char *testbed)
{
	osync_trace(TRACE_ENTRY, "%s(%s)", __func__, testbed);
	
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(debug->group, testbed);

	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);
	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member1, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member1, "mockobjtype3", "mockformat3");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member2, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member2, "mockobjtype3", "mockformat3");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize_func(debug->plugin, initialize_order);
	osync_plugin_set_finalize_func(debug->plugin, finalize_order);

	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, debug);
	return debug;
}

START_TEST (engine_sync_out_of_order)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group3(testbed);

	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);
	
	_engine_instrument_pluginenv(engine, debug);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

static void main_disconnect4(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	osync_assert(env);
	osync_assert(env->num_connect == 3);
	osync_assert(env->num_disconnect == 3);
	osync_assert(env->num_get_changes == 3);
	osync_assert(env->main_connect == 1);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 1);
	
	g_atomic_int_inc(&(env->main_disconnect));
	
	osync_context_report_success(env->ctx[0]);
	osync_context_report_success(env->ctx[2]);
	osync_context_report_success(ctx);
	osync_context_report_success(env->ctx[1]);
	
	osync_context_unref(env->ctx[0]);
	osync_context_unref(env->ctx[1]);
	osync_context_unref(env->ctx[2]);
	
	env->num_connect = 0;
	env->num_disconnect = 0;
	env->num_get_changes = 0;
	env->num_sync_done = 0;
	env->main_connect = 0;
	env->main_disconnect = 0;
	env->main_get_changes = 0;
	env->main_sync_done = 0;
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void *initialize_reuse(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockobjtype1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect3);
	osync_objtype_sink_set_disconnect_func(sink, disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, sync_done3);

	osync_objtype_sink_set_userdata(sink, env);
	
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	sink = osync_objtype_sink_new("mockobjtype2", error);
	if (!sink)
		goto error;
	
	format_sink = osync_objformat_sink_new("vcard", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect3);
	osync_objtype_sink_set_disconnect_func(sink, disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, sync_done3);

	osync_objtype_sink_set_userdata(sink, env);
	
	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	sink = osync_objtype_sink_new("mockobjtype3", error);
	if (!sink)
		goto error;
	
	format_sink = osync_objformat_sink_new("plain", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect3);
	osync_objtype_sink_set_disconnect_func(sink, disconnect3);
	osync_objtype_sink_set_get_changes_func(sink, get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, sync_done3);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	/* The main sink */
	sink = osync_objtype_main_sink_new(error);
	if (!sink)
		goto error;
	
	osync_objtype_sink_set_connect_func(sink, main_connect3);
	osync_objtype_sink_set_disconnect_func(sink, main_disconnect4);
	osync_objtype_sink_set_get_changes_func(sink, main_get_changes3);
	osync_objtype_sink_set_sync_done_func(sink, main_sync_done3);

	osync_objtype_sink_set_userdata(sink, env);
	
	osync_plugin_info_set_main_sink(info, sink);
	osync_objtype_sink_unref(sink);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize_reuse(void *data)
{
	mock_env *env = data;
	
	g_free(env);
}

static OSyncDebugGroup *_create_group4(char *testbed)
{
	osync_trace(TRACE_ENTRY, "%s(%s)", __func__, testbed);
	
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(debug->group, testbed);
	
	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);

	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member1, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member1, "mockobjtype3", "mockformat3");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member2, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member2, "mockobjtype3", "mockformat3");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);

	osync_plugin_set_initialize_func(debug->plugin, initialize_reuse);
	osync_plugin_set_finalize_func(debug->plugin, finalize_reuse);
	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, debug);
	return debug;
}

START_TEST (engine_sync_reuse)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group4(testbed);

	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);

	_engine_instrument_pluginenv(engine, debug);
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

START_TEST (engine_sync_stress)
{
	int n = 250; /* FIXME: Used to be 1000 - #1040 */
	int i = 0;
	
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group4(testbed);

	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);
	
	_engine_instrument_pluginenv(engine, debug);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	for (i = 0; i < n; i++) {
		fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
		fail_unless(error == NULL, NULL);
	}
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

static void connect5(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	g_atomic_int_inc(&(env->num_connect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void disconnect5(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	g_atomic_int_inc(&(env->num_disconnect));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void get_changes5(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);
	osync_assert(formatenv != NULL);
	
	OSyncObjFormat *format = osync_format_env_find_objformat(formatenv, "mockformat1");
	osync_assert(format != NULL);
	
	OSyncError *error = NULL;
	OSyncChange *change = osync_change_new(&error);
	osync_assert(change != NULL);
	osync_assert(error == NULL);
	
	osync_change_set_changetype(change, OSYNC_CHANGE_TYPE_ADDED);

	char *uid = osync_rand_str(16, &error); 
	osync_assert(error == NULL);
	osync_change_set_uid(change, uid);
	g_free(uid);
	
	OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), &error);
	osync_assert(file != NULL);
	file->path = g_strdup(osync_change_get_uid(change));
		
	file->data = g_strdup_printf("%p", change);
	file->size = strlen(file->data);
		
	OSyncData *changedata = osync_data_new((char *)file, sizeof(OSyncFileFormat), format, &error);
	osync_data_set_objtype(changedata, "mockobjtype1");

	osync_assert(changedata != NULL);
	osync_assert(error == NULL);
	osync_change_set_data(change, changedata);
	osync_data_unref(changedata);
	
	osync_context_report_change(ctx, change);
	osync_change_unref(change);
	
	g_atomic_int_inc(&(env->num_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void commit_change5(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *data)
{
	mock_env *env = data;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, data, info, ctx, change);
	
	g_atomic_int_inc(&(env->num_commit_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void *initialize5(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockformat1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect5);
	osync_objtype_sink_set_disconnect_func(sink, disconnect5);
	osync_objtype_sink_set_get_changes_func(sink, get_changes5);
	osync_objtype_sink_set_commit_func(sink, commit_change5);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize5(void *data)
{
	mock_env *env = data;
	
	osync_assert(env->num_connect == 1);
	osync_assert(env->num_disconnect == 1);
	osync_assert(env->num_get_changes == 1);
	osync_assert(env->num_commit_changes == 1);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_free(env);
}

static OSyncDebugGroup *_create_group5(char *testbed)
{
	osync_trace(TRACE_ENTRY, "%s(%s)", __func__, testbed);
	
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(debug->group, testbed);
	
	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);
	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize_func(debug->plugin, initialize5);
	osync_plugin_set_finalize_func(debug->plugin, finalize5);
	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, debug);
	return debug;
}

START_TEST (engine_sync_read_write)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group5(testbed);

	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);

	_engine_instrument_pluginenv(engine, debug);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

#define COMMIT_STRESS_GROUP6 250

static void get_changes6(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	int i;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);
	osync_assert(formatenv != NULL);
	
	OSyncObjFormat *format = osync_format_env_find_objformat(formatenv, "mockformat1");
	osync_assert(format != NULL);
	
	OSyncError *error = NULL;
	
	for (i = 0; i < COMMIT_STRESS_GROUP6; i++) {
		OSyncChange *change = osync_change_new(&error);
		osync_assert(change != NULL);
		osync_assert(error == NULL);
		
		osync_change_set_changetype(change, OSYNC_CHANGE_TYPE_ADDED);
		char *rand = osync_rand_str(16, &error);
		osync_assert(error == NULL);
		char *uid = osync_strdup_printf("uid_%s_%u", rand, i);
		osync_change_set_uid(change, uid);
		osync_free(uid);
		osync_free(rand);

		OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), &error);
		osync_assert(file != NULL);
		file->path = g_strdup(osync_change_get_uid(change));
		
		file->data = g_strdup_printf("%p", change);
		file->size = strlen(file->data);
		
		OSyncData *changedata = osync_data_new((char *)file, sizeof(OSyncFileFormat), format, &error);
		osync_assert(changedata != NULL);

		osync_data_set_objtype(changedata, "mockobjtype1");
		
		osync_assert(changedata != NULL);
		osync_assert(error == NULL);
		osync_change_set_data(change, changedata);
		osync_data_unref(changedata);
		
		osync_context_report_change(ctx, change);
		osync_change_unref(change);
	}
	
	g_atomic_int_inc(&(env->num_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void *initialize6(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockformat1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect5);
	osync_objtype_sink_set_disconnect_func(sink, disconnect5);
	osync_objtype_sink_set_get_changes_func(sink, get_changes6);
	osync_objtype_sink_set_commit_func(sink, commit_change5);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize6(void *data)
{
	mock_env *env = data;
	
	osync_assert(env->num_connect == 1);
	osync_assert(env->num_disconnect == 1);
	osync_assert(env->num_get_changes == 1);
	osync_assert(env->num_commit_changes == COMMIT_STRESS_GROUP6);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_free(env);
}

static OSyncDebugGroup *_create_group6(char *testbed)
{
	osync_trace(TRACE_ENTRY, "%s(%s)", __func__, testbed);
	
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(debug->group, testbed);
	
	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);
	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	_member_add_format(debug->member2, "mockobjtype2", "mockformat2");
	_member_add_format(debug->member2, "mockobjtype3", "mockformat3");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize_func(debug->plugin, initialize6);
	osync_plugin_set_finalize_func(debug->plugin, finalize6);
	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, debug);
	return debug;
}

START_TEST (engine_sync_read_write_stress)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group6(testbed);
	
	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);
	
	_engine_instrument_pluginenv(engine, debug);

	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

static void get_changes7(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *data)
{
	mock_env *env = data;
	int i;
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, data, info, ctx);
	
	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);
	osync_assert(formatenv != NULL);
	
	OSyncObjFormat *format = osync_format_env_find_objformat(formatenv, "mockformat1");
	osync_assert(format != NULL);
	
	OSyncError *error = NULL;
	
	for (i = 0; i < 2; i++) {
		OSyncChange *change = osync_change_new(&error);
		osync_assert(change != NULL);
		osync_assert(error == NULL);
		
		osync_change_set_changetype(change, OSYNC_CHANGE_TYPE_ADDED);
		char *uid = osync_rand_str(16, &error);
		osync_assert(error == NULL);
		osync_change_set_uid(change, uid);
		g_free(uid);
		
		OSyncFileFormat *file = osync_try_malloc0(sizeof(OSyncFileFormat), &error);
		osync_assert(file != NULL);
		file->path = g_strdup(osync_change_get_uid(change));
		
		file->data = g_strdup_printf("%p", change);
		file->size = strlen(file->data);
		
		OSyncData *changedata = osync_data_new((char *)file, sizeof(OSyncFileFormat), format, &error);
		osync_assert(changedata != NULL);

		osync_data_set_objtype(changedata, "mockobjtype1");
		
		osync_assert(changedata != NULL);
		osync_assert(error == NULL);
		osync_change_set_data(change, changedata);
		osync_data_unref(changedata);
		
		osync_context_report_change(ctx, change);
		osync_change_unref(change);
	}
	
	g_atomic_int_inc(&(env->num_get_changes));
	
	osync_context_report_success(ctx);
	
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void *initialize7(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p)", __func__, info, error);

	mock_env *env = osync_try_malloc0(sizeof(mock_env), error);
	if (!env)
		goto error;

	OSyncObjTypeSink *sink = osync_objtype_sink_new("mockobjtype1", error);
	if (!sink)
		goto error;
	
	OSyncObjFormatSink *format_sink = osync_objformat_sink_new("mockformat1", error);
	osync_objtype_sink_add_objformat_sink(sink, format_sink);
	osync_objformat_sink_unref(format_sink);
	
	osync_objtype_sink_set_connect_func(sink, connect5);
	osync_objtype_sink_set_disconnect_func(sink, disconnect5);
	osync_objtype_sink_set_get_changes_func(sink, get_changes7);
	osync_objtype_sink_set_commit_func(sink, commit_change5);

	osync_objtype_sink_set_userdata(sink, env);

	osync_plugin_info_add_objtype(info, sink);
	osync_objtype_sink_unref(sink);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, env);
	return (void *)env;

error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));
	return NULL;
}

static void finalize7(void *data)
{
	mock_env *env = data;
	
	osync_assert(env->num_connect == 1);
	osync_assert(env->num_disconnect == 1);
	osync_assert(env->num_get_changes == 1);
	osync_assert(env->num_commit_changes == 2);
	osync_assert(env->main_connect == 0);
	osync_assert(env->main_disconnect == 0);
	osync_assert(env->main_get_changes == 0);
	
	g_free(env);
}

static OSyncDebugGroup *_create_group7(char *testbed)
{
	osync_trace(TRACE_ENTRY, "%s(%s)", __func__, testbed);
	
	OSyncDebugGroup *debug = g_malloc0(sizeof(OSyncDebugGroup));
	
	OSyncError *error = NULL;
	debug->group = osync_group_new(&error);
	fail_unless(debug->group != NULL, NULL);
	fail_unless(error == NULL, NULL);

	osync_group_set_configdir(debug->group, testbed);
	
	debug->member1 = osync_member_new(&error);
	fail_unless(debug->member1 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member1);
	osync_member_set_pluginname(debug->member1, "mock-sync-foo");
	char *path = g_strdup_printf("%s/configs/group/1", testbed);
	osync_member_set_configdir(debug->member1, path);
	g_free(path);

	_member_add_format(debug->member1, "mockobjtype1", "mockformat1");
	
	debug->member2 = osync_member_new(&error);
	fail_unless(debug->member2 != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_group_add_member(debug->group, debug->member2);
	osync_member_set_pluginname(debug->member2, "mock-sync-foo");
	path = g_strdup_printf("%s/configs/group/2", testbed);
	osync_member_set_configdir(debug->member2, path);
	g_free(path);

	_member_add_format(debug->member2, "mockobjtype1", "mockformat1");
	
	debug->plugin = osync_plugin_new(&error);
	fail_unless(debug->plugin != NULL, NULL);
	fail_unless(error == NULL, NULL);
	
	osync_plugin_set_name(debug->plugin, "mock-sync-foo");
	osync_plugin_set_longname(debug->plugin, "Mock Sync Plugin");
	osync_plugin_set_description(debug->plugin, "This is a pseudo plugin");
	osync_plugin_set_start_type(debug->plugin, OSYNC_START_TYPE_EXTERNAL);
	osync_plugin_set_config_type(debug->plugin, OSYNC_PLUGIN_NO_CONFIGURATION);
	
	osync_plugin_set_initialize_func(debug->plugin, initialize7);
	osync_plugin_set_finalize_func(debug->plugin, finalize7);
	
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
	osync_client_run_external(debug->client2, pipe_path, debug->plugin, &error);
	g_free(pipe_path);
	
	osync_trace(TRACE_EXIT, "%s: %p", __func__, debug);
	return debug;
}

START_TEST (engine_sync_read_write_stress2)
{
	char *testbed = setup_testbed("sync_setup");
	char *formatdir = g_strdup_printf("%s/formats",  testbed);
	
	OSyncError *error = NULL;
	OSyncDebugGroup *debug = _create_group7(testbed);

	OSyncEngine *engine = osync_engine_new(debug->group, &error);
	fail_unless(engine != NULL, NULL);
	fail_unless(error == NULL, NULL);
	osync_engine_set_formatdir(engine, formatdir);
	osync_engine_set_schemadir(engine, testbed);

	_engine_instrument_pluginenv(engine, debug);
	
	fail_unless(osync_engine_initialize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_synchronize_and_block(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	fail_unless(osync_engine_finalize(engine, &error), NULL);
	fail_unless(error == NULL, NULL);
	
	_free_group(debug);
	
	osync_engine_unref(engine);
	
	g_free(formatdir);
	
	destroy_testbed(testbed);
}
END_TEST

OSYNC_TESTCASE_START(engine)

OSYNC_TESTCASE_ADD(engine_new)
OSYNC_TESTCASE_ADD(engine_init)
OSYNC_TESTCASE_ADD(engine_sync)
OSYNC_TESTCASE_ADD(engine_sync_multi_obj)
OSYNC_TESTCASE_ADD(engine_sync_out_of_order)
OSYNC_TESTCASE_ADD(engine_sync_reuse)
OSYNC_TESTCASE_ADD(engine_sync_stress)

OSYNC_TESTCASE_ADD(engine_sync_read_write)
OSYNC_TESTCASE_ADD(engine_sync_read_write_stress)
OSYNC_TESTCASE_ADD(engine_sync_read_write_stress2)

//connect problem
//get_changes problem

OSYNC_TESTCASE_END

