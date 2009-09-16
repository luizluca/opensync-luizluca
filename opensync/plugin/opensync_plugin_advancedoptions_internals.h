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

#ifndef _OPENSYNC_PLUGIN_ADVANCEDOPTIONS_INTERNALS_
#define _OPENSYNC_PLUGIN_ADVANCEDOPTIONS_INTERNALS_

/**
 * @defgroup OSyncPluginConfigAdvancedOptionsInternalsAPI OpenSync Plugin Config Advanced Options
 * @ingroup OSyncPluginInternals
 * @brief Functions to get and set a plugin's custom configuration options
 *
 */

/*@{*/

OSYNC_TEST_EXPORT OSyncPluginAdvancedOptionType osync_plugin_advancedoption_type_string_to_val(const char *typestr);

/*@}*/

#endif /*_OPENSYNC_PLUGIN_ADVANCEDOPTIONS_INTERNALS_*/

