
/*! @brief The action that should be invoked
 * @ingroup OSyncFilterAPI
 **/
typedef enum OSyncFilterAction {
	/** This filter should be ignored */
	OSYNC_FILTER_IGNORE = 0,
	/** The change should be allowed to pass (Overwrites previous action) */
	OSYNC_FILTER_ALLOW = 1,
	/** The change should be denied to pass (Overwrites previous action) */
	OSYNC_FILTER_DENY = 2
} OSyncFilterAction;

typedef OSyncFilterAction (* OSyncFilterFunction)(OSyncChange *, char *config);

osync_bool osync_filter_change_allowed(OSyncMember *destmember, OSyncChange *change);
OSyncFilter *osync_filter_add(OSyncGroup *group, OSyncMember *sourcemember, OSyncMember *destmember, const char *sourceobjtype, const char *destobjtype, const char *detectobjtype, OSyncFilterAction action);
OSyncFilter *osync_filter_add_custom(OSyncGroup *group, OSyncMember *sourcemember, OSyncMember *destmember, const char *sourceobjtype, const char *destobjtype, const char *detectobjtype, const char *function_name);
void osync_filter_remove(OSyncGroup *group, OSyncFilter *filter);
void osync_filter_free(OSyncFilter *filter);
void osync_filter_set_config(OSyncFilter *filter, const char *config);
const char *osync_filter_get_config(OSyncFilter *filter);

const char *osync_filter_get_sourceobjtype(OSyncFilter *filter);
const char *osync_filter_get_destobjtype(OSyncFilter *filter);
const char *osync_filter_get_detectobjtype(OSyncFilter *filter);
OSyncFilterAction osync_filter_get_action(OSyncFilter *filter);
OSyncMember *osync_filter_get_sourcemember(OSyncFilter *filter);
OSyncMember *osync_filter_get_destmember(OSyncFilter *filter);
