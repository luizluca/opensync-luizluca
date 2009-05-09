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

#ifndef _OPENSYNC_TIME_INTERNALS_H_
#define _OPENSYNC_TIME_INTERNALS_H_

#ifdef _WIN32
/* Windows does not provide gmtime_r and localtime_r */
/* This module uses these two functions internally   */
/* They are also exported for testing purposes       */
OSYNC_TEST_EXPORT inline struct tm* gmtime_r (const time_t *clock, struct tm *result);
OSYNC_TEST_EXPORT inline struct tm* localtime_r (const time_t *clock, struct tm *result);
#endif /* _WIN32 */


#endif /* _OPENSYNC_TIME_INTERNALS_H_ */

