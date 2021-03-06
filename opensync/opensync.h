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

#elif defined(__GNUC__) 
#include <unistd.h>
#define OSYNC_EXPORT __attribute__ ((visibility("default")))

#elif defined(__sun) 
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

/* Mark function as depercated for a more stable API
 * (Copied from libsyncml)
 */
#if __GNUC__ - 0 > 3 || (__GNUC__ - 0 == 3 && __GNUC_MINOR__ - 0 >= 2)
  /* gcc >= 3.2 */
# define OSYNC_DEPRECATED __attribute__ ((deprecated))
#elif defined(_MSC_VER) && (_MSC_VER >= 1300) && (_MSC_VER < 1400)
  /* msvc >= 7 */
# define OSYNC_DEPRECATED __declspec(deprecated)
#elif defined(_MSV_VER) && (_MSC_VER >= 1400)
  /* MS Visual Studio 2005 */
# define OSYNC_DEPRECATED
#else
# define OSYNC_DEPRECATED
#endif

#if __GNUC__
#define GCC_FORMAT_CHECK(a,b) __attribute__ ((format(printf, a, b)))
#else
#define GCC_FORMAT_CHECK(a,b)
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

#define OSYNC_CAPS_MAJOR_VERSION 1
#define OSYNC_CAPS_MINOR_VERSION 0

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
        osync_error_set(error, errortype, format,## __VA_ARGS__);                            \
        return;                                                                             \
    } } while(0)

#define osync_return_val_if_fail_and_set_error(condition, val, error, errortype, format, ...) do { \
    if (!(condition)) {                                                                            \
        osync_error_set(error, errortype, format,## __VA_ARGS__);                                   \
        return (val);                                                                              \
    } } while(0)

typedef int osync_bool;
typedef int osync_memberid;
typedef int osync_mappingid;
typedef int osync_groupid;
typedef long long int osync_archiveid;
typedef long long int osync_messageid;

/**************************************************************
 * Enums
 *************************************************************/

/*! @ingroup OSyncPlugin
 * @brief The possible start types of a plugin or client proxy
 *
 * Determines how the sync process is to be run.
 */
typedef enum {
	/** The start type is unknown (has not been set) */
	OSYNC_START_TYPE_UNKNOWN,
	/** Start as child process */
	OSYNC_START_TYPE_PROCESS,
	/** Start in a separate thread */
	OSYNC_START_TYPE_THREAD,
	/** Start as a separate external process */
	OSYNC_START_TYPE_EXTERNAL
} OSyncStartType;

/*! @ingroup OSyncChangeCmds
 * @brief The possible returns of a change comparison
 */
typedef enum {
	/** The result is unknown, there was an error */
	OSYNC_CONV_DATA_UNKNOWN = 0,
	/** The changes are not the same */
	OSYNC_CONV_DATA_MISMATCH = 1,
	/** The changes are not the same but look similar */
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
typedef struct OSyncMerger OSyncMerger;
typedef struct OSyncCapsConverter OSyncCapsConverter;

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
typedef struct OSyncPluginExternalPlugin OSyncPluginExternalPlugin;

/* Engine component */
typedef struct OSyncEngine OSyncEngine;
typedef struct OSyncObjEngine OSyncObjEngine;
typedef struct OSyncSinkEngine OSyncSinkEngine;
typedef struct OSyncMappingEntryEngine OSyncMappingEntryEngine;
typedef struct OSyncMappingEngine OSyncMappingEngine;

typedef struct  OSyncEngineMemberUpdate OSyncEngineMemberUpdate;
typedef struct  OSyncEngineChangeUpdate OSyncEngineChangeUpdate;
typedef struct  OSyncEngineMappingUpdate OSyncEngineMappingUpdate;
typedef struct  OSyncEngineUpdate OSyncEngineUpdate;

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
typedef struct OSyncSinkStateDB OSyncSinkStateDB;
typedef struct OSyncHashTable OSyncHashTable;

/* IPC component */
typedef struct OSyncMessage OSyncMessage;
typedef struct OSyncQueue OSyncQueue;

/* Group component */
typedef struct OSyncGroup OSyncGroup;
typedef struct OSyncGroupEnv OSyncGroupEnv;
typedef struct OSyncMember OSyncMember;

/* Capabilities component */
typedef struct OSyncCapabilities OSyncCapabilities;
typedef struct OSyncCapability OSyncCapability;
typedef struct OSyncCapabilityParameter OSyncCapabilityParameter;
typedef struct OSyncCapabilitiesObjType OSyncCapabilitiesObjType;

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

#include "opensync/opensync-common.h"
#include "opensync/opensync-debug.h"

#endif
