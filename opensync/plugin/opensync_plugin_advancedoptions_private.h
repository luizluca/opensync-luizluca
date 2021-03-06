/*
 * libopensync - A synchronization framework
 * Copyright (C) 2008  Daniel Gollub <gollub@b1-systems.de>
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

#ifndef _OPENSYNC_PLUGIN_ADVANCEDOPTIONS_PRIVATE_H_
#define _OPENSYNC_PLUGIN_ADVANCEDOPTIONS_PRIVATE_H_

/**
 * @defgroup OSyncPluginAdvancedOptionsPrivateAPI OpenSync Plugin Advanced Options Private
 * @ingroup OSyncPluginPrivate
 */

/*@{*/

/**
 * @brief Gives information about advanced plugin options
 **/
struct OSyncPluginAdvancedOption {

	/** Display Name - this appears in the UI */
	char *displayname;

	/** Max Occurs */
	unsigned int maxoccurs;

	/** Max Size */
	unsigned int max;

	/** Min Size */
	unsigned int min;

	/** Option Name - kind of ID for the option. Internal use. Should not appear any UI. */
	char *name;

	/** List of Advanced Option Parameters (OSyncPluginAdvancedOptionParameter) */
	OSyncList *parameters;

	/** Type */
	OSyncPluginAdvancedOptionType type;

	/** List of Value Enumeration - consisits of (char*) */
	OSyncList *valenum;

	/** The actual option value. */
	char *value;

	/** Object reference counting */
	int ref_count;
};

struct OSyncPluginAdvancedOptionParameter {

	/** Display Name - this appears in the UI */
	char *displayname;

	/** Option Name - kind of ID for the option. Internal use. Should not appear any UI. */
	char *name;

	/** Type */
	OSyncPluginAdvancedOptionType type;

	/** List of Value Enumeration - consisits of (char*) */
	OSyncList *valenum;

	/** The actual parameter value. */
	char *value;

	/** Object reference counting */
	int ref_count;
};

/*@}*/

#endif /* _OPENSYNC_PLUGIN_ADVANCEDOPTIONS_PRIVATE_H_ */

