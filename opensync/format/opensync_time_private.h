/*
 * libopensync - A synchronization framework
 * Copyright (C) 2004-2005  Armin Bauer <armin.bauer@opensync.org>
 * Copyright (C) 2006-2008 Daniel Gollub <gollub@b1-systems.de>
 * Copyright (C) 2007 Chris Frey <cdfrey@netdirect.ca>
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

#ifndef _OPENSYNC_TIME_PRIVATE_H_
#define _OPENSYNC_TIME_PRIVATE_H_

#include <glib.h>

/** @brief Function remove dashes from datestamp and colon
 * 
 * @param timestamp The timestamp which gets cleaned
 * @returns valid vtime stamp in YYYYMMDD[THHMMDD[Z]] (the caller is responsible for freeing)
 */
static char *osync_time_timestamp_remove_dash(const char *timestamp);

/** @brief Functions converts timestamps of vcal in localtime or UTC. 
 * 
 * @param vcal The vcalendar which has to be converted.
 * @param toUTC If TRUE conversion from localtime to UTC.
 * @param error An OSyncError struct
 * @return timestamp modified vcalendar 
 */ 
char *osync_time_convert_entry(const char *vcal, osync_bool toUTC, OSyncError **error);

/** @brief Function converts a UTC vtime stamp to a localtime vtime stamp
 * 
 * @param entry The whole vcal entry as GString which gets modified. 
 * @param field The field name which should be modified. 
 * @param toUTC The toggle in which direction we convert. TRUE = convert to UTC
 * @param error An OSyncError struct
 */ 
static void osync_time_convert_time_field(GString *entry, const char *field, osync_bool toUTC, OSyncError **error);

#endif /* _OPENSYNC_TIME_PRIVATE_H_ */
