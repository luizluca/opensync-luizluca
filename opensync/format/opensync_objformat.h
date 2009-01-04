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

#ifndef _OPENSYNC_OBJFORMAT_H_
#define _OPENSYNC_OBJFORMAT_H_

/**
 * @defgroup OSyncFormat OpenSync Format Module
 * @ingroup OSyncPublic
 * @defgroup OSyncObjFormatAPI OpenSync Object Format
 * @ingroup OSyncFormat
 * @brief Functions for handling object formats
 */
/*@{*/

typedef void * (* OSyncFormatInitializeFunc) (OSyncError **error);
typedef void (* OSyncFormatFinalizeFunc) (void *userdata);

typedef OSyncConvCmpResult (* OSyncFormatCompareFunc) (const char *leftdata, unsigned int leftsize, const char *rightdata, unsigned int rightsize);
typedef osync_bool (* OSyncFormatCopyFunc) (const char *input, unsigned int inpsize, char **output, unsigned int *outpsize, OSyncError **error);
typedef osync_bool (* OSyncFormatDuplicateFunc) (const char *uid, const char *input, unsigned int insize, char **newuid, char **output, unsigned int *outsize, osync_bool *dirty, OSyncError **error);
typedef void (* OSyncFormatCreateFunc) (char **data, unsigned int *size);
typedef void (* OSyncFormatDestroyFunc) (char *data, unsigned int size);
typedef char *(* OSyncFormatPrintFunc) (const char *data, unsigned int size);
typedef time_t (* OSyncFormatRevisionFunc) (const char *data, unsigned int size, OSyncError **error);
typedef osync_bool (* OSyncFormatMarshalFunc) (const char *input, unsigned int inpsize, OSyncMessage *message, OSyncError **error);
typedef osync_bool (* OSyncFormatDemarshalFunc) (OSyncMessage *message, char **output, unsigned int *outpsize, OSyncError **error);
typedef osync_bool (* OSyncFormatValidateFunc) (const char *data, unsigned int size, void *user_data, OSyncError **error);

typedef osync_bool (* OSyncFormatMergeFunc) (const char *input, unsigned int inpsize, char **output, unsigned int *outpsize,  const char *entire, unsigned int entsize, OSyncCapabilities *caps, OSyncError **error);
typedef osync_bool (* OSyncFormatDemergeFunc) (const char *input, unsigned int inpsize, char **output, unsigned int *outpsize, OSyncCapabilities *caps, OSyncError **error);

/**
 * @brief Creates a new object format
 * @param name the name of the object format
 * @param objtype_name the name of the object type
 * @param error Pointer to an error struct
 * @return The pointer to the newly allocated object format or NULL in case of error
 */
OSYNC_EXPORT OSyncObjFormat *osync_objformat_new(const char *name, const char *objtype_name, OSyncError **error);

/*! @brief Increase the reference count on an object format
 * 
 * @param format Pointer to the object format
 * 
 */
OSYNC_EXPORT OSyncObjFormat *osync_objformat_ref(OSyncObjFormat *format);

/*! @brief Decrease the reference count on an object format
 * 
 * @param format Pointer to the object format
 * 
 */
OSYNC_EXPORT void osync_objformat_unref(OSyncObjFormat *format);

/**
 * @brief Returns the name of an object format
 * @param format Pointer to the object format
 * @return The name of the specified object format
 */
OSYNC_EXPORT const char *osync_objformat_get_name(OSyncObjFormat *format);

/**
 * @brief Returns the object type of an object format
 * @param format Pointer to the object format
 * @return The name of the specified object format's object type
 */
OSYNC_EXPORT const char *osync_objformat_get_objtype(OSyncObjFormat *format);

/**
 * @brief Returns revision of the supplied data in specified format 
 *
 * @param format Pointer to the object format
 * @param data Pointer to the object to get the revision
 * @param size Size in bytes of the object specified by the data parameter
 * @param error Pointer to an error struct
 * @returns Revision of the specified object in seconds since 1970, -1 on error
 */
OSYNC_EXPORT time_t osync_objformat_get_revision(OSyncObjFormat *format, const char *data, unsigned int size, OSyncError **error);

/**
 * @brief Sets the optional initialize function for an object format
 *
 * The initialize function can be used to allocate internal structures or other
 * required data, which get passed as void pointer to each object format plugin
 * call.
 *
 * This option set require that also a finalize function get set.
 *
 * @param format Pointer to the object format
 * @param initialize_func The initialize function to use
 */
OSYNC_EXPORT void osync_objformat_set_initialize_func(OSyncObjFormat *format, OSyncFormatInitializeFunc initialize_func);

/**
 * @brief Sets the optional finalize function for an object format
 *
 * The finalize function has to be use to release memory which got allocated
 * with the object format initialize function.
 *
 * @param format Pointer to the object format
 * @param finalize_func The finalize function to use
 */
OSYNC_EXPORT void osync_objformat_set_finalize_func(OSyncObjFormat *format, OSyncFormatFinalizeFunc finalize_func);

/**
 * @brief Sets the optional compare function for an object format
 *
 * The compare function can be used to compare two objects in your object 
 * format. This is optional - if you prefer, you can instead provide a 
 * conversion to and from the xml format and let all the comparison be done 
 * there.
 *
 * @param format Pointer to the object format
 * @param cmp_func The compare function to use
 */
OSYNC_EXPORT void osync_objformat_set_compare_func(OSyncObjFormat *format, OSyncFormatCompareFunc cmp_func);

/**
 * @brief Sets the destroy function for an object format
 *
 * The destroy function is used to free data structures allocated by your format.
 *
 * @param format Pointer to the object format
 * @param destroy_func The destroy function to use
 */
OSYNC_EXPORT void osync_objformat_set_destroy_func(OSyncObjFormat *format, OSyncFormatDestroyFunc destroy_func);

/**
 * @brief Set copy function for the specified format
 * @param format Pointer to the object format
 * @param copy_func Copy function to set
 */
OSYNC_EXPORT void osync_objformat_set_copy_func(OSyncObjFormat *format, OSyncFormatCopyFunc copy_func);

/**
 * @brief Sets the duplicate function for an object format
 *
 * The duplicate function can be used to duplicate an object in your format.
 * Duplication does not mean to make two objects out of one, but to change 
 * the uid of the object in such a way that it differs from the original uid.
 *
 * Most formats will never need this.
 *
 * @param format Pointer to the object format
 * @param dupe_func The duplicate function to use
 */
OSYNC_EXPORT void osync_objformat_set_duplicate_func(OSyncObjFormat *format, OSyncFormatDuplicateFunc dupe_func);

/**
 * @brief Set object creation function of the specified format 
 *
 * @param format Pointer to the object format
 * @param create_func Create function
 */
OSYNC_EXPORT void osync_objformat_set_create_func(OSyncObjFormat *format, OSyncFormatCreateFunc create_func);

/**
 * @brief Sets the print function for an object format
 *
 * If your format is not in a human readable format already, you should set
 * the print function to a function that returns a human readable string 
 * describing the object as closely as possible. This information will be 
 * used by the user to decide which object to pick when there is a conflict.
 *
 * @param format Pointer to the object format
 * @param print_func The print function to use
 */
OSYNC_EXPORT void osync_objformat_set_print_func(OSyncObjFormat *format, OSyncFormatPrintFunc print_func);

/**
 * @brief Sets the revision function for an object format
 *
 * @param format Pointer to the object format
 * @param revision_func The revision function to set
 */
OSYNC_EXPORT void osync_objformat_set_revision_func(OSyncObjFormat *format, OSyncFormatRevisionFunc revision_func);

/**
 * @brief Sets the marshal function for an object format
 *
 * @param format Pointer to the object format
 * @param marshal_func The marshal function to set
 */
OSYNC_EXPORT void osync_objformat_set_marshal_func(OSyncObjFormat *format, OSyncFormatMarshalFunc marshal_func);

/**
 * @brief Sets the demarshal function for an object format
 *
 * @param format Pointer to the object format
 * @param demarshal_func The demarshal function to set
 */
OSYNC_EXPORT void osync_objformat_set_demarshal_func(OSyncObjFormat *format, OSyncFormatDemarshalFunc marshal_func);

/**
 * @brief Sets the optional validation function for an object format
 *
 * The validation function can be used to validate data for the specific
 * format. This is optional. The validation should be fast, since once this
 * function is registered this get called during conversion for every change.
 *
 * @param format Pointer to the object format
 * @param validate_func The validation function to use
 */
OSYNC_EXPORT void osync_objformat_set_validate_func(OSyncObjFormat *format, OSyncFormatValidateFunc validate_func);

/**
 * @brief Sets the optional merge function for an object format
 *
 * The merge function can be used to merge data for the specific
 * format. This is optional.
 *
 * @param format Pointer to the object format
 * @param merge_func The merge function to use
 */
OSYNC_EXPORT void osync_objformat_set_merge_func(OSyncObjFormat *format, OSyncFormatMergeFunc merge_func);

/**
 * @brief Sets the optional demerge function for an object format
 *
 * The demerge function can be used to demerge data for the specific
 * format. This is optional.
 *
 * @param format Pointer to the object format
 * @param demerge_func The demerge function to use
 */
OSYNC_EXPORT void osync_objformat_set_demerge_func(OSyncObjFormat *format, OSyncFormatDemergeFunc demerge_func);

/**
 * @brief Prints the specified object
 *
 * Uses the object format's print function if set, otherwise the object's
 * data will be returned as a string.
 *
 * @param format Pointer to the object format
 * @param data Pointer to the object to destroy
 * @param size Size in bytes of the object specified by the data parameter
 * @returns Human readable string of the specified object. Caller is responsible for freeing the string
 */
OSYNC_EXPORT char *osync_objformat_print(OSyncObjFormat *format, const char *data, unsigned int size);

/*@}*/

#endif /* _OPENSYNC_OBJFORMAT_H_ */
