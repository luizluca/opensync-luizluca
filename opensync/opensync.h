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

#ifndef HAVE_OPENSYNC_H
#define HAVE_OPENSYNC_H

#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus

#define OPENSYNC_BEGIN_DECLS extern "C" {
#define OPENSYNC_END_DECLS }

#else

#define OPENSYNC_BEGIN_DECLS
#define OPENSYNC_END_DECLS

#endif

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define __func__ __FUNCTION__
#define OSYNC_EXPORT __declspec(dllexport)

#elif __GNUC__ 
#include <unistd.h>
#define OSYNC_EXPORT __attribute__ ((visibility("default")))

#elif __sun 
#include <unistd.h>
#define OSYNC_EXPORT __global 

#else
#define OSYNC_EXPORT
#endif

/* Some symbols are only exported in the opensync    */
/* testing library                                   */
/* opensync_testing_EXPORTS is defined from CMAKE    */
/* when the build target is the testing library      */
#ifdef opensync_testing_EXPORTS
#define OSYNC_TEST_EXPORT OSYNC_EXPORT
#else
#define OSYNC_TEST_EXPORT
#endif

OPENSYNC_BEGIN_DECLS

/**
 * @defgroup OSyncPrivate OpenSync Private API
 * @defgroup OSyncPublic OpenSync Public API
 */

/**************************************************************
 * Versions 
 *************************************************************/

#define OSYNC_GROUP_MAJOR_VERSION 1
#define OSYNC_GROUP_MINOR_VERSION 0

#define OSYNC_MEMBER_MAJOR_VERSION 1
#define OSYNC_MEMBER_MINOR_VERSION 0

#define OSYNC_PLUGIN_MAJOR_VERSION 1
#define OSYNC_PLUGIN_MINOR_VERSION 0


/**************************************************************
 * Defines
 *************************************************************/
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define __NULLSTR(x) x ? x : "(NULL)"

#ifndef NDEBUG
#define osync_assert(x) if (!(x)) { fprintf(stderr, "%s:%i:E:%s: Assertion \"" #x "\" failed\n", __FILE__, __LINE__, __func__); abort();}
#define osync_assert_msg(x, msg) if (!(x)) { fprintf(stderr, "%s:%i:E:%s: %s\n", __FILE__, __LINE__, __func__, msg); abort();}
#else
#define osync_assert(x)
#define osync_assert_msg(x, msg)
#endif

#define osync_return_if_fail(condition) do {    \
    if (!(condition)) {                         \
      return;                                   \
    } } while (0)

#define osync_return_val_if_fail(condition, val) do {   \
    if (!(condition)) {                                 \
      return (val);                                     \
    } } while (0)


#define osync_return_if_fail_and_set_error(condition, error, errortype, format, ...) do {   \
    if (!(condition)) {                                                                     \
        osync_error_set(error, errortype, format, __VA_ARGS__ );                            \
        return;                                                                             \
    }Ê} while(0)

#define osync_return_val_if_fail_and_set_error(condition, val, error, errortype, format, ...) do { \
    if (!(condition)) {                                                                            \
        osync_error_set(error, errortype, format, __VA_ARGS__ );                                   \
        return (val);                                                                              \
    }Ê} while(0)

typedef int osync_bool;

/**************************************************************
 * Enums
 *************************************************************/

typedef enum {
	OSYNC_START_TYPE_UNKNOWN,
	OSYNC_START_TYPE_PROCESS,
	OSYNC_START_TYPE_THREAD,
	OSYNC_START_TYPE_EXTERNAL
} OSyncStartType;

/*! @ingroup OSyncChangeCmds
 * @brief The possible returns of a change comparison
 */
typedef enum {
	/** The result is unknown, there was a error */
	OSYNC_CONV_DATA_UNKNOWN = 0,
	/** The changes are not the same */
	OSYNC_CONV_DATA_MISMATCH = 1,
	/** The changs are not the same but look similar */
	OSYNC_CONV_DATA_SIMILAR = 2,
	/** The changes are exactly the same */
	OSYNC_CONV_DATA_SAME = 3
} OSyncConvCmpResult;

/*! 
 * @ingroup OSyncChange
 * @brief The changetypes of a change object */
typedef enum  {
	/** Unknown changetype */
	OSYNC_CHANGE_TYPE_UNKNOWN = 0,
	/** Object was added */
	OSYNC_CHANGE_TYPE_ADDED = 1,
	/** Object is unmodifed */
	OSYNC_CHANGE_TYPE_UNMODIFIED = 2,
	/** Object is deleted */
	OSYNC_CHANGE_TYPE_DELETED = 3,
	/** Object has been modified */
	OSYNC_CHANGE_TYPE_MODIFIED = 4
} OSyncChangeType;

/**************************************************************
 * Structs
 *************************************************************/

/* Archive component */
typedef struct OSyncArchive OSyncArchive;

/* Data component */
typedef struct OSyncData OSyncData;
typedef struct OSyncChange OSyncChange;

/* Database component */
typedef struct OSyncDB OSyncDB;
 
/* Format component */
typedef struct OSyncFormatEnv OSyncFormatEnv;
typedef struct OSyncObjFormat OSyncObjFormat;
typedef struct OSyncFormatConverterPath OSyncFormatConverterPath;
typedef struct OSyncFormatConverter OSyncFormatConverter;
typedef struct OSyncObjFormatSink OSyncObjFormatSink;

/* Plugin component */
typedef struct OSyncContext OSyncContext;
typedef struct OSyncPlugin OSyncPlugin;
typedef struct OSyncPluginInfo OSyncPluginInfo;
typedef struct OSyncPluginEnv OSyncPluginEnv;
typedef struct OSyncObjTypeSink OSyncObjTypeSink;
typedef struct OSyncPluginConfig OSyncPluginConfig;
typedef struct OSyncPluginAuthentication OSyncPluginAuthentication;
typedef struct OSyncPluginAdvancedOption OSyncPluginAdvancedOption;
typedef struct OSyncPluginAdvancedOptionParameter OSyncPluginAdvancedOptionParameter;
typedef struct OSyncPluginConnection OSyncPluginConnection;
typedef struct OSyncPluginLocalization OSyncPluginLocalization;
typedef struct OSyncPluginResource OSyncPluginResource;

/* Engine component */
typedef struct OSyncEngine OSyncEngine;
typedef struct OSyncObjEngine OSyncObjEngine;
typedef struct OSyncSinkEngine OSyncSinkEngine;
typedef struct OSyncMappingEntryEngine OSyncMappingEntryEngine;
typedef struct OSyncMappingEngine OSyncMappingEngine;

/* Client component */
typedef struct OSyncClient OSyncClient;
typedef struct OSyncClientProxy OSyncClientProxy;

/* Mapping component */
typedef struct OSyncMapping OSyncMapping;
typedef struct OSyncMappingTable OSyncMappingTable;
typedef struct OSyncMappingEntry OSyncMappingEntry;

/* Module component */
typedef struct OSyncModule OSyncModule;

/* Helper component */
typedef struct OSyncAnchor OSyncAnchor;
typedef struct OSyncHashTable OSyncHashTable;

/* IPC component */
typedef struct OSyncMessage OSyncMessage;
typedef struct OSyncQueue OSyncQueue;

/* Group component */
typedef struct OSyncGroup OSyncGroup;
typedef struct OSyncGroupEnv OSyncGroupEnv;
typedef struct OSyncMember OSyncMember;

/* Merger component */
typedef struct OSyncCapabilities OSyncCapabilities;
typedef struct OSyncCapability OSyncCapability;

/* XMLFormat component */
typedef struct OSyncXMLFormat OSyncXMLFormat;
typedef struct OSyncXMLFormatSchema OSyncXMLFormatSchema;
typedef struct OSyncXMLField OSyncXMLField;
typedef struct OSyncXMLFieldList OSyncXMLFieldList;

/* Common component */
typedef struct OSyncError OSyncError;
typedef struct OSyncMarshal OSyncMarshal;

/* Version component */
typedef struct OSyncVersion OSyncVersion;

/*! @brief Returns the version of opensync
 * 
 * Returns a string identifying the major and minor version
 * of opensync (something like "0.11")
 * 
 * @returns String with version
 * 
 */
OSYNC_EXPORT const char *osync_get_version(void);

OPENSYNC_END_DECLS

#include "opensync/opensync-support.h"
#include "opensync-error.h"

#endif
