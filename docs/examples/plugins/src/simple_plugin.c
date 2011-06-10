#include <opensync/opensync.h>
#include <opensync/opensync-data.h>
#include <opensync/opensync-format.h>
#include <opensync/opensync-plugin.h>
#include <opensync/opensync-helper.h>
#include <opensync/opensync-version.h>

#include <string.h>

static void connect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *userdata)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p, %p)", __func__, sink, info, ctx, userdata);
	//Each time you get passed a context (which is used to track
	//calls to your plugin)

	//cast void* userdata to the sink specific data type

	OSyncError *error = NULL;

	/*
	 * Now connect to your devices and report
	 * 
	 * an error via:
	 * osync_context_report_error(ctx, ERROR_CODE, "Some message");
	 * 
	 * or success via:
	 * osync_context_report_success(ctx);
	 * 
	 * You have to use one of these 2 somewhere to answer the context.
	 * 
	 */

	//you can also use the anchor system to detect a device reset
	//or some parameter change here. Check the docs to see how it works
	//Now you get the last stored anchor from the device
	OSyncSinkStateDB *state_db = osync_objtype_sink_get_state_db(sink);
	osync_bool anchormatch;

	if (!osync_sink_state_equal(state_db, "anchor_key", "dynamic_anchor_value", &anchormatch, &error)) {
		/* anchor couldn't be compared */
		goto error;
	}
	
	if (!anchormatch) {
		/* request slow sync */
		osync_context_report_slowsync(ctx);
	}

	osync_context_report_success(ctx);
	osync_trace(TRACE_EXIT, "%s", __func__);
	return;

error:
	osync_context_report_osyncerror(ctx, error);
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(&error));
	osync_error_unref(&error);
}

static void get_changes(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, osync_bool slow_sync, void *userdata)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, userdata, info, ctx);

	OSyncFormatEnv *formatenv = osync_plugin_info_get_format_env(info);

	OSyncError *error = NULL;

	/*
	 * Now you can get the changes.
	 * Loop over all changes you get and do the following:
	 */

	do {
		char *uid = osync_strdup("<some uid>");

		//Now get the data of this change
		char *data = NULL;


		/* Determine the "Change Type" of this change (e.g. ADDED, MODIFIED, DELETED, ...) */
		OSyncChangeType changetype  = OSYNC_CHANGE_TYPE_ADDED;

		//Make the new change to report
		OSyncChange *change = osync_change_new(&error);
		if (!change) {
			osync_context_report_osyncwarning(ctx, error);
			osync_error_unref(&error);
			continue;
		}

		//Now set the uid of the object
		osync_change_set_uid(change, uid);
		
		osync_change_set_changetype(change, changetype);
		
		if (changetype == OSYNC_CHANGE_TYPE_UNMODIFIED) {
			osync_free(uid);
			osync_change_unref(change);
			continue;
		}

		osync_free(uid);

		OSyncObjFormat *format = osync_format_env_find_objformat(formatenv, "<objformat>");
		
		OSyncData *odata = osync_data_new(data, 0, format, &error);
		if (!odata) {
			osync_change_unref(change);
			osync_context_report_osyncwarning(ctx, error);
			osync_error_unref(&error);
			continue;
		}

		osync_data_set_objtype(odata, osync_objtype_sink_get_name(sink));

		//Now you can set the data for the object
		osync_change_set_data(change, odata);
		osync_data_unref(odata);

		// just report the change via
		osync_context_report_change(ctx, change);

		osync_change_unref(change);

		osync_free(uid);
	} while(0);

	//Now we need to answer the call
	osync_context_report_success(ctx);
	osync_trace(TRACE_EXIT, "%s", __func__);
}

static void commit_change(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, OSyncChange *change, void *userdata)
{
	
	/*
	 * Here you have to add, modify or delete a object
	 * 
	 */
	switch (osync_change_get_changetype(change)) {
		case OSYNC_CHANGE_TYPE_DELETED:
			//Delete the change
			//Dont forget to answer the call on error
			break;
		case OSYNC_CHANGE_TYPE_ADDED:
			//Add the change
			//Dont forget to answer the call on error
			break;
		case OSYNC_CHANGE_TYPE_MODIFIED:
			//Modify the change
			//Dont forget to answer the call on error
			break;
		default:
			;
	}

	//Answer the call
	osync_context_report_success(ctx);
}

static void sync_done(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *userdata)
{
	/*
	 * This function will only be called if the sync was successful
	 */
	OSyncError *error = NULL;
	
	//If we use anchors we have to update it now.
	//Now you get/calculate the current anchor of the device
	OSyncSinkStateDB *state_db = osync_objtype_sink_get_state_db(sink); 
	if (!osync_sink_state_set(state_db, "anchor_key", "dynamic_anchor_value", &error))
			goto error;

	//Answer the call
	osync_context_report_success(ctx);
	return;
error:
	osync_context_report_osyncerror(ctx, error);
	osync_error_unref(&error);
	return;
}

static void disconnect(OSyncObjTypeSink *sink, OSyncPluginInfo *info, OSyncContext *ctx, void *userdata)
{
	//Close all stuff you need to close
	
	//Answer the call
	osync_context_report_success(ctx);
}

static void finalize(void *userdata)
{
	//Free all stuff that you have allocated here.
}


static void initialize(OSyncPlugin *plugin, OSyncPluginInfo *info, OSyncError **error)
{
	/*
	 * get the config
	 */
	OSyncPluginConfig *config = osync_plugin_info_get_config(info);
	if (!config) {
		osync_error_set(error, OSYNC_ERROR_GENERIC, "Unable to get config.");
		goto error;
	}
	/*
	 * You need to specify the <some name>_environment somewhere with
	 * all the members you need
	*/
	void *env = NULL;
	if (!env)
		goto error;

	/* 
	 * Process the config here and set the options on your environment
	*/
	/*
	 * Process plugin specific advanced options 
	 */
	OSyncList *optslist = osync_plugin_config_get_advancedoptions(config);
	OSyncList *o;
	for (o = optslist; o; o = o->next) {
		OSyncPluginAdvancedOption *option = o->data;

		const char *val = osync_plugin_advancedoption_get_value(option);
		const char *name = osync_plugin_advancedoption_get_name(option);

		if (!strcmp(name,"<your-option>")) {
			if (!strcmp(val, "<your-value>")) {
				/*
				 * set a varaible to a specific value
				 */;
			}
		}
	}
	osync_list_free(optslist);
	
	/*
	 * Process Ressource options
	 */
	OSyncList *l, *list = NULL;
	list = osync_plugin_info_get_objtype_sinks(info);
	for (l=list; l; l = l->next) {
		OSyncObjTypeSink *sink = (OSyncObjTypeSink *) l->data ;

		const char *objtype = osync_objtype_sink_get_name(sink);
		OSyncPluginResource *res = osync_plugin_config_find_active_resource(config, objtype);
		
		/* get objformat sinks */
		OSyncList *s = NULL;
		OSyncList *objformats = osync_plugin_resource_get_objformat_sinks(res);
		for (s = objformats; s; s = s->next) {
			OSyncObjFormatSink *fsink = s->data; // there could be only one sink
			const char *objformat = osync_objformat_sink_get_objformat(fsink);
			osync_assert(objformat);
			osync_trace(TRACE_INTERNAL, "objtype %s has objformat %s", objtype, objformat);
		}	
		osync_list_free(objformats);
		/* Every sink can have different functions ... */
		osync_objtype_sink_set_connect_func(sink, connect);
		osync_objtype_sink_set_disconnect_func(sink, disconnect);
		osync_objtype_sink_set_get_changes_func(sink, get_changes);
		osync_objtype_sink_set_commit_func(sink, commit_change);
		osync_objtype_sink_set_sync_done_func(sink, sync_done);

		/*
		 * If you need plugin specific userdata passed to this
		 * plugin sink functions. You can set it with:
		 *
		 * osync_objtype_sink_set_userdata(sink, userdata_pointer);
		 */
	}

	osync_list_free(list);
	
	//Now save your struct.
	osync_plugin_set_data(plugin, env);
	return;
error:
	osync_trace(TRACE_EXIT_ERROR, "%s: %s", __func__, osync_error_print(error));	
}

/* Here we actually tell opensync which sinks are available. */
static osync_bool discover(OSyncPluginInfo *info, void *userdata, OSyncError **error)
{
	osync_trace(TRACE_ENTRY, "%s(%p, %p, %p)", __func__, userdata, info, error);

	// Report avaliable sinks...
	OSyncObjTypeSink *sink = osync_plugin_info_find_objtype(info, "<objtype e.g. note>");
	if (!sink) {
		return FALSE;
	}
	osync_objtype_sink_set_available(sink, TRUE);
	
	OSyncVersion *version = osync_version_new(error);
	osync_version_set_plugin(version, "<your plugin-name>");
	//osync_version_set_version(version, "version");
	//osync_version_set_modelversion(version, "version");
	//osync_version_set_firmwareversion(version, "firmwareversion");
	//osync_version_set_softwareversion(version, "softwareversion");
	//osync_version_set_hardwareversion(version, "hardwareversion");
	osync_plugin_info_set_version(info, version);
	osync_version_unref(version);

	osync_trace(TRACE_EXIT, "%s", __func__);
	return TRUE;
}


osync_bool get_sync_info(OSyncPluginEnv *env, OSyncError **error)
{
	//Now you can create a new plugin information and fill in the details
	//Note that you can create several plugins here
	OSyncPlugin *plugin = osync_plugin_new(error);
	if (!plugin)
		goto error;
	
	//Tell opensync something about your plugin
	osync_plugin_set_name(plugin, "short name, maybe < 15 chars");
	osync_plugin_set_longname(plugin, "long name. maybe < 50 chars");
	osync_plugin_set_description(plugin, "A longer description. < 200 chars");

	//Now set the function we made earlier
	osync_plugin_set_initialize(plugin, initialize);
	osync_plugin_set_finalize(plugin, finalize);
	osync_plugin_set_discover(plugin, discover);

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
