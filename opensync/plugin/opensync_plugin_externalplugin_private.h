/*
 * libopensync - A synchronization framework
 * Copyright (C) 2009  Henrik Kaare Poulsen
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

#ifndef _OPENSYNC_PLUGIN_EXTERNALPLUGIN_PRIVATE_H_
#define _OPENSYNC_PLUGIN_EXTERNALPLUGIN_PRIVATE_H_

/**
 * @defgroup OSyncPluginExternalPluginPrivateAPI OpenSync Plugin ExternalPlugin Private
 * @ingroup OSyncPluginPrivate
 */

/*@{*/

/**
 * @brief Gives information for plugin of type OSYNC_START_TYPE_EXTERNAL
 **/
struct OSyncPluginExternalPlugin {
	/** Command to be executed to start the external process.
	    In printf format; should have one %s
	    which will be replaced with
	    the path of the plugin pipe to the client process
	*/
	char *external_command;

	/** Object reference counting */
	int ref_count;
};

/*@}*/

#endif /* _OPENSYNC_PLUGIN_EXTERNALPLUGIN_PRIVATE_H_ */

