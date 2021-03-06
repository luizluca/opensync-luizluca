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

#ifndef OPENSYNC_INTERNALS_H_
#define OPENSYNC_INTERNALS_H_

#include <glib.h>
#include <gmodule.h>
#include <string.h>
#include <glib/gprintf.h>
#include <glib/gstdio.h>
#include <sys/stat.h>

#include "config.h"

#include <errno.h>
#ifndef _WIN32
extern int errno;
#endif

#ifdef __GNUC__
#define OSYNC_UNUSED __attribute__ ((unused))
#else
#define OSYNC_UNUSED
#endif

#include "opensync/common/opensync_common_internals.h"

#endif /* OPENSYNC_INTERNALS_H_ */

