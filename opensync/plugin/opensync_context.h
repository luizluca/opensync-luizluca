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

#ifndef _OPENSYNC_CONTEXT_H
#define _OPENSYNC_CONTEXT_H

OPENSYNC_BEGIN_DECLS

/**
 * @defgroup OSyncContextAPI OpenSync Context
 * @ingroup OSyncPlugin
 * Public part of OpenSync Context
 */

/*@{*/

typedef void (* OSyncContextCallbackFn)(void *, OSyncError *);
typedef void (* OSyncContextChangeFn) (OSyncChange *, void *);
typedef void (* OSyncContextSlowSyncFn) (void *);

OSYNC_EXPORT OSyncContext *osync_context_new(OSyncError **error);
OSYNC_EXPORT OSyncContext *osync_context_ref(OSyncContext *context);
OSYNC_EXPORT void osync_context_unref(OSyncContext *context);

OSYNC_EXPORT void osync_context_set_callback(OSyncContext *context, OSyncContextCallbackFn callback, void *userdata);
OSYNC_EXPORT void osync_context_set_changes_callback(OSyncContext *context, OSyncContextChangeFn changes);
OSYNC_EXPORT void osync_context_set_slowsync_callback(OSyncContext *context, OSyncContextSlowSyncFn slowsync_func, void *userdata);
OSYNC_EXPORT void osync_context_set_warning_callback(OSyncContext *context, OSyncContextCallbackFn warning);

OSYNC_EXPORT void osync_context_report_error(OSyncContext *context, OSyncErrorType type, const char *format, ...) GCC_FORMAT_CHECK(3, 4);
OSYNC_EXPORT void osync_context_report_success(OSyncContext *context);
OSYNC_EXPORT void osync_context_report_osyncerror(OSyncContext *context, OSyncError *error);

OSYNC_EXPORT void osync_context_report_osyncwarning(OSyncContext *context, OSyncError *error);
OSYNC_EXPORT void osync_context_report_change(OSyncContext *context, OSyncChange *change);
OSYNC_EXPORT void osync_context_report_slowsync(OSyncContext *contextr);
OSYNC_EXPORT void osync_context_report_uid_update(OSyncContext *ctx, const char *olduid, const char *newuid);

/*@}*/

OPENSYNC_END_DECLS

#endif //_OPENSYNC_CONTEXT_H
