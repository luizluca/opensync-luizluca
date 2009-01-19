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

#ifndef _OPENSYNC_OBJFORMAT_INTERNALS_H_
#define _OPENSYNC_OBJFORMAT_INTERNALS_H_

/**
 * @defgroup OSyncObjFormatInternalAPI OpenSync Object Format Internals
 * @ingroup OSyncFormatPrivate
 * @brief The internal part of the OSyncObjFormat API
 */
/*@{*/

/**
 * @brief Initialize an object format
 *
 * Call the object format specific initialize function. If no finalize function for
 * this object format got registered this function is NOOP.
 *
 * Functions returns FALSE when error by the initialize_func call get set.
 *
 * @param format Pointer to the object format
 * @param error Pointer to error struct, which get failed when initialization fails
 * @returns TRUE on success, FALSE otherwise 
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_initialize(OSyncObjFormat *format, OSyncError **error);

/**
 * @brief Finalize an object format
 *
 * Call the finalize function of an object format. If no finalize function for
 * this object format got registered this function is NOOP.
 *
 * @param format Pointer to the object format
 */
OSYNC_TEST_EXPORT void osync_objformat_finalize(OSyncObjFormat *format);

/**
 * @brief Compares two objects of the same object format
 *
 * Compares two objects of the same object format using the format's compare function
 *
 * @param format Pointer to the object format
 * @param leftdata Pointer to the object to compare
 * @param leftsize the size in bytes of the object specified by the leftdata parameter
 * @param rightdata Pointer to the other object to compare
 * @param rightsize the size in bytes of the object specified by the rightdata parameter
 * @returns the comparison result
 */
OSYNC_TEST_EXPORT OSyncConvCmpResult osync_objformat_compare(OSyncObjFormat *format, const char *leftdata, unsigned int leftsize, const char *rightdata, unsigned int rightsize);

/**
 * @brief Duplicate an object of the specified format
 *
 * Duplication does not mean to make two objects out of one, but to change
 * the uid of the object in such a way that it differs from the original uid.
 *
 * @param format Pointer to the object format
 * @param uid The uid of the object
 * @param input Pointer to the object to duplicate
 * @param insize Size in bytes of the object specified by the input parameter
 * @param newuid The new uid for the duplicate object
 * @param output Pointer to a pointer to be set to the duplicate object
 * @param outsize Pointer to a variable to be set to the size of the duplicate object
 * @param dirty Reference of dirty flag. Dirty flags determines if change still needs to be multiplied/written
 * @param error Pointer to an error struct
 * @return TRUE if the duplication succeeded, FALSE otherwise.
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_duplicate(OSyncObjFormat *format, const char *uid, const char *input, unsigned int insize, char **newuid, char **output, unsigned int *outsize, osync_bool *dirty, OSyncError **error);

/**
 * @brief Object creation function of the specified format
 *
 * @param format Pointer to the object format
 * @param data Pointer to the data
 * @param size Size of the data
 */
OSYNC_TEST_EXPORT void osync_objformat_create(OSyncObjFormat *format, char **data, unsigned int *size);

/**
 * @brief Destroy an object of the specified format
 * @param format Pointer to the object format
 * @param data Pointer to the object to destroy
 * @param size Size in bytes of the object specified by the data parameter
 */
OSYNC_TEST_EXPORT void osync_objformat_destroy(OSyncObjFormat *format, char *data, unsigned int size);

/**
 * @brief Copy data in the specified way of the format
 * @param format Pointer to the object format
 * @param indata Source to copy
 * @param insize Size of source
 * @param outdata Copy destination
 * @param outsize Size of copy
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_copy(OSyncObjFormat *format, const char *indata, unsigned int insize, char **outdata, unsigned int *outsize, OSyncError **error);

/**
 * @brief Compares the names of two object formats
 * @param leftformat Pointer to the object format to compare
 * @param rightformat Pointer to the other object format to compare
 * @return TRUE if the two object format names are equal, false otherwise
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_is_equal(OSyncObjFormat *leftformat, OSyncObjFormat *rightformat);

/**
 * @brief Checks if the format needs to be marshaled or not.
 *
 * @param format Pointer to the object format
 * @returns TRUE if format needs to be marshaled, FALSE otherwise
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_must_marshal(OSyncObjFormat *format);

/**
 * @brief Marshals supplied input in format specific way into a serialized OSyncMarshal
 *
 * @param format Pointer to the object format
 * @param input Data to marshal
 * @param inpsize Size of supplied data
 * @param marshal Marshaled data in a OSyncMarshal
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_marshal(OSyncObjFormat *format, const char *input, unsigned int inpsize, OSyncMarshal *marshal, OSyncError **error);

/**
 * @brief Demarshals supplied OSyncMarshal in format specific way
 *
 * @param format Pointer to the object format
 * @param marshal Marshaled data as OSyncMarshal
 * @param output Data to store unserialized Message content
 * @param outpsize Size of demarshled data in output parameter
 * @param error Pointer to an error struct
 * @returns TRUE on success, FALSE otherwise
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_demarshal(OSyncObjFormat *format, OSyncMarshal *marshal, char **output, unsigned int *outpsize, OSyncError **error);

/**
 * @brief Validate supplied data in format specific way
 *
 * @param format Pointer to the object format
 * @param data Pointer to the object to validate
 * @param size Size in bytes of the object specified by the data parameter
 * @param error Pointer to an error struct
 * @returns TRUE if data passed format specific validation, otherwise FALSE

 */
osync_bool osync_objformat_validate(OSyncObjFormat *format, const char *data, unsigned int size, OSyncError **error);

/**
 * @brief Check if specific format requires validation
 *
 * If a validation function is set for the specific format then validation is
 * required for every conversion which results in this object format.
 *
 * @param format Pointer to the object format
 * @returns TRUE if validation is required for this format, otherwise FALSE

 */
osync_bool osync_objformat_must_validate(OSyncObjFormat *format);

/**
 * @brief Merge supplied data in format specific way
 *
 * @param format Pointer to the object format
 * @param data Reference of buffer which stores data to merge
 * @param size Reference of size in bytes of the buffer specified by the data paramter
 * @param entire
 * @param entsize Size in bytes of the base data buffer specified by the entire parameter
 * @param caps The capabilities list which describes what must get merged: entire -> input
 * @param error Pointer to an error struct
 * @returns TRUE if data got merged successfully, otherwise FALSE
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_merge(OSyncObjFormat *format,
		char **data, unsigned int *size,
		const char *entire, unsigned int entsize,
		OSyncCapabilities *caps, OSyncError **error);

/**
 * @brief Demerge supplied data in format specific way
 *
 * @param format Pointer to the object format
 * @param data Pointer to the buffer which should get demerged 
 * @param size Reference of size in bytes of the buffer specified by the data parameter
 * @param caps The capabilities list which describes what must get demerged
 * @param error Pointer to an error struct
 * @returns TRUE if data got demerged successfully, otherwise FALSE
 */
OSYNC_TEST_EXPORT osync_bool osync_objformat_demerge(OSyncObjFormat *format,
		char **data, unsigned int *size,
		OSyncCapabilities *caps, OSyncError **error);

/**
 * @brief Check if specific format is able to merge/demerge 
 *
 * If merge and demerge function are set for the specific format then merger
 * and demerger get invoked. Merger/Demerge still can be disabled in group
 * configuration, by disabling the merger, or not providing capabilities for
 * the member.
 *
 * @param format Pointer to the object format
 * @returns TRUE if merge and demerge is support for this format, otherwise FALSE
 */
osync_bool osync_objformat_has_merger(OSyncObjFormat *format);

/*@}*/

#endif /* _OPENSYNC_OBJFORMAT_INTERNALS_H_ */

