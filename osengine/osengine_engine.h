OSyncEngine *osync_engine_new(OSyncGroup *group, OSyncError **);
void osync_engine_free(OSyncEngine *engine);

void osync_engine_calculate_flags(OSyncEngine *engine);
void osengine_client_all_deciders(OSyncEngine *engine);
void osync_engine_decider(OSyncEngine *engine);
void osengine_client_decider(OSyncEngine *engine, OSyncClient *client);
void osync_engine_changes_done(OSyncEngine *engine, OSyncMember *member);
osync_bool osync_engine_init(OSyncEngine *engine, OSyncError **);
void osync_engine_finalize(OSyncEngine *engine);
void osync_engine_flag_only_info(OSyncEngine *engine);
osync_bool osync_engine_synchronize(OSyncEngine *engine, OSyncError **);
void osync_engine_pause(OSyncEngine *engine);
void osync_engine_abort(OSyncEngine *engine);
osync_bool osync_engine_wait_sync_end(OSyncEngine *engine, OSyncError **error);
void osync_engine_wait_info_end(OSyncEngine *engine);
void osync_engine_one_iteration(OSyncEngine *engine);
void osync_engine_flag_manual(OSyncEngine *engine);
void osync_engine_set_message_callback(OSyncEngine *engine, void *(* function) (OSyncEngine *, OSyncClient *, const char *, void *, void *), void *user_data);
void osync_engine_set_conflict_callback(OSyncEngine *engine, void (* function) (OSyncEngine *, OSyncMapping *, void *), void *user_data);
void osync_engine_set_changestatus_callback(OSyncEngine *engine, void (* function) (OSyncEngine*, MSyncChangeUpdate *, void *), void *user_data);
void osync_engine_set_mappingstatus_callback(OSyncEngine *engine, void (* function) (MSyncMappingUpdate *, void *), void *user_data);
void osync_engine_set_enginestatus_callback(OSyncEngine *engine, void (* function) (OSyncEngine *, OSyncEngineUpdate *, void *), void *user_data);
void osync_engine_set_memberstatus_callback(OSyncEngine *engine, void (* function) (MSyncMemberUpdate *, void *), void *user_data);
osync_bool osync_engine_sync_and_block(OSyncEngine *engine, OSyncError **error);
OSyncMapping *osync_engine_mapping_from_id(OSyncEngine *engine, long long id);
