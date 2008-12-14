/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 * 
 */

#ifndef _OPENSYNC_MEMBER_H_
#define _OPENSYNC_MEMBER_H_

#include <opensync/opensync_list.h>

/**
 * @defgroup OSyncMemberAPI OpenSync Member
 * @ingroup OSyncPublic
 * @brief Used to manipulate members, which represent one device or application in a group
 * 
 */
/*@{*/

/** @brief Creates a new member for a group
 * 
 * @param error Pointer to a error-struct
 * @returns A newly allocated member
 * 
 */
OSYNC_EXPORT OSyncMember *osync_member_new(OSyncError **error);

/** @brief Increase the reference count of the member
 * 
 * @param member The member
 * 
 */
OSYNC_EXPORT OSyncMember *osync_member_ref(OSyncMember *member);

/** @brief Decrease the reference count of the member
 * 
 * @param member The member
 * 
 */
OSYNC_EXPORT void osync_member_unref(OSyncMember *member);


/** @brief Returns the name of the default plugin of the member
 * 
 * @param member The member
 * @returns The name of the plugin
 * 
 */
OSYNC_EXPORT const char *osync_member_get_pluginname(OSyncMember *member);

/** @brief Sets the name of the default plugin of a member
 * 
 * @param member The member
 * @param pluginname The name of the default plugin
 * 
 */
OSYNC_EXPORT void osync_member_set_pluginname(OSyncMember *member, const char *pluginname);


/** @brief Returns the inidividual name of the member
 * 
 * @param member The member
 * @returns The name of the plugin
 * 
 */
OSYNC_EXPORT const char *osync_member_get_name(OSyncMember *member);

/** @brief Sets an individual name of the member
 * 
 * @param member The member
 * @param name The individual name of the member 
 * 
 */
OSYNC_EXPORT void osync_member_set_name(OSyncMember *member, const char *name);


/** @brief Returns the configuration directory where this member is stored
 * 
 * @param member The member
 * @returns The configuration directory
 * 
 */
OSYNC_EXPORT const char *osync_member_get_configdir(OSyncMember *member);

/** @brief Sets the directory where a member is supposed to be stored
 * 
 * @param member The member
 * @param configdir The name of the directory
 * 
 */
OSYNC_EXPORT void osync_member_set_configdir(OSyncMember *member, const char *configdir);


/** @brief Checks if Member has configuration
 *
 * @param member The member
 * @returns TURE if member has configuration, FALSE otherwise
 */
OSYNC_EXPORT osync_bool osync_member_has_config(OSyncMember *member);

/** @brief Gets the configuration data of this member
 * 
 * The config file is read in this order:
 * - If there is a configuration in memory that is not yet saved
 * this is returned
 * - If there is a config file in the member directory this is read
 * and returned
 * - Otherwise the default config file is loaded from one the opensync
 * directories
 * 
 * @param member The member
 * @param error Pointer to a error
 * @returns The member configuration of the plugin default configuration if the member isn't configuered already 
 * 
 */
OSYNC_EXPORT OSyncPluginConfig *osync_member_get_config_or_default(OSyncMember *member, OSyncError **error);

/** @brief Gets the configuration data of this member
 * 
 * The config file is read in this order:
 * - If there is a configuration in memory that is not yet saved
 * this is returned
 * - If there is a config file in the member directory this is read
 * and returned
 * - Otherwise the default config file is loaded from one the opensync
 * directories (but only if the plugin specified that it can use the default
 * configuration)
 * 
 * @param member The member
 * @param error Pointer to a error
 * @returns Member configuration 
 * 
 */
OSYNC_EXPORT OSyncPluginConfig *osync_member_get_config(OSyncMember *member, OSyncError **error);

/** @brief Sets the config data for a member
 * 
 * Note that this does not save the config data
 * 
 * @param member The member
 * @param config The new config data
 * 
 */
OSYNC_EXPORT void osync_member_set_config(OSyncMember *member, OSyncPluginConfig *config);


/** @brief Loads a member from a directory where it has been saved
 * 
 * @param member The Member pointer of the member which gets loaded
 * @param path The path of the member
 * @param error Pointer to a error
 * @returns TRUE on success, FALSE if error
 * 
 */
OSYNC_EXPORT osync_bool osync_member_load(OSyncMember *member, const char *path, OSyncError **error);

/** @brief Saves a member to it config directory
 * 
 * @param member The member to save
 * @param error Pointer to a error
 * @returns TRUE if the member was saved successfully, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_member_save(OSyncMember *member, OSyncError **error);

/** @brief Delete a member
 * 
 * @param member The member to delete 
 * @param error Pointer to a error
 * @returns TRUE if the member was deleted successfully, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_member_delete(OSyncMember *member, OSyncError **error);


/** @brief Gets the unique id of a member
 * 
 * @param member The member
 * @returns The id of the member thats unique in its group
 * 
 */
OSYNC_EXPORT long long int osync_member_get_id(OSyncMember *member);


/** @brief Get the number of supported object types of this member
 * 
 * @param member The member pointer
 * @returns Number of supported object type of this member
 * 
 */
OSYNC_EXPORT int osync_member_num_objtypes(OSyncMember *member);

/** @brief Get the name of the nth supported object type of this member
 * 
 * @param member The member pointer
 * @param nth The nth position of the list of supported object types of this member
 * @returns Name of the nth supported object type
 * 
 */
OSYNC_EXPORT const char *osync_member_nth_objtype(OSyncMember *member, int nth);


/** @brief Add an OSyncObjTypeSink object to the member list of supported object types of this member
 * 
 * @param member The member pointer
 * @param sink The OSyncObjTypeSink object to add 
 * 
 */
OSYNC_EXPORT void osync_member_add_objtype_sink(OSyncMember *member, OSyncObjTypeSink *sink);

/** @brief Find the object type sink (OSyncObjTypeSink) for the given object type of
 *         a certain member.
 * 
 * @param member The member pointer
 * @param objtype The searched object type 
 * @returns OSyncObjTypeSink pointer if object type sink is avaliable, otherwise NULL 
 * 
 */
OSYNC_EXPORT OSyncObjTypeSink *osync_member_find_objtype_sink(OSyncMember *member, const char *objtype);


/** @brief Returns if a certain object type is enabled on this member
 * 
 * @param member The member
 * @param objtype The name of the object type to check
 * @returns TRUE if the object type is enabled, FALSE otherwise
 * 
 */
OSYNC_EXPORT osync_bool osync_member_objtype_enabled(OSyncMember *member, const char *objtype);

/** @brief Enables or disables a object type on a member
 * 
 * @param member The member
 * @param objtype The name of the object type to change
 * @param enabled Set to TRUE if you want to sync the object type, FALSE otherwise
 * 
 * Note: this function should be called only after sink information for the member
 *       is available (osync_member_require_sink_info())
 *
 * @todo Change function interface to not require the plugin to be instanced manually.
 *       See comments on osync_group_set_objtype_enabled()
 */
OSYNC_EXPORT void osync_member_set_objtype_enabled(OSyncMember *member, const char *objtype, osync_bool enabled);


/** @brief List of all available object formats for a specifc object type of this member 
 * 
 * @param member The member pointer
 * @param objtype The searched object type 
 * @param error Pointer to a error
 * @return List of all object formats of a specific object type of the member
 * 
 */
OSYNC_EXPORT const OSyncList *osync_member_get_objformats(OSyncMember *member, const char *objtype, OSyncError **error);

/** @brief Add a specifc Object Format to member 
 * 
 * @param member The member pointer
 * @param objtype The searched object type 
 * @param format The name of the Object Format 
 * 
 */
OSYNC_EXPORT void osync_member_add_objformat(OSyncMember *member, const char *objtype, const char *format);

/** @brief Add a specifc Object Format with a conversion path config to member 
 * 
 * @param member The member pointer
 * @param objtype The searched object type 
 * @param format The name of the Object Format 
 * @param format_config The Object Format specific configuration
 * 
 */
OSYNC_EXPORT void osync_member_add_objformat_with_config(OSyncMember *member, const char *objtype, const char *format, const char *format_config);


/** @brief Get the capabilities of the member 
 * 
 * @param member The member
 * @returns The capabilities of this member, NULL if no capabilities are set
 */
OSYNC_EXPORT OSyncCapabilities *osync_member_get_capabilities(OSyncMember *member);

/** @brief Set the capabilities of the member 
 * 
 * @param member The member
 * @param capabilities The capabilities
 * @param error Pointer to a error
 * @returns TRUE if the capabilities got set successfully, otherwise FALSE 
 */
OSYNC_EXPORT osync_bool osync_member_set_capabilities(OSyncMember *member, OSyncCapabilities *capabilities, OSyncError **error);


/** @brief Get pointer of the Merger 
 * 
 * @param member The member
 * @returns The pointer of the Merger, NULL if merger is disabled
 */
OSYNC_EXPORT OSyncMerger *osync_member_get_merger(OSyncMember *member);


/** @brief Remove all object types from member. 
 * 
 * @param member The member
 *
 * Note: this function should be called to flush the member before discovering.
 *       To detect if something isn't supported anymore.
 *
 */
OSYNC_EXPORT void osync_member_flush_objtypes(OSyncMember *member);


/** @brief Get the main sink of member. 
 * 
 * @param member The member
 * @returns OSyncObjTypeSink pointer of the main sink.
 *
 */
OSYNC_EXPORT OSyncObjTypeSink *osync_member_get_main_sink(OSyncMember *member);


/** @brief Checks if the member configuration is up to date. 
 * 
 * @param member The member
 * @returns TRUE if member configuration is up to date. 
 *
 */
OSYNC_EXPORT osync_bool osync_member_config_is_uptodate(OSyncMember *member);

/** @brief Checks if the plugin configuration is up to date. 
 * 
 * @param member The member
 * @returns TRUE if plugin configuration is up to date. 
 *
 */
OSYNC_EXPORT osync_bool osync_member_plugin_is_uptodate(OSyncMember *member);

/*@}*/

#endif /* _OPENSYNC_MEMBER_H_ */
