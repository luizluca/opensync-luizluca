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

#ifndef _OPENSYNC_OBJFORMAT_PRIVATE_H_
#define _OPENSYNC_OBJFORMAT_PRIVATE_H_

/**
 * @defgroup OSyncFormatPrivate OpenSync Format Module Private
 * @ingroup OSyncPrivate
 * @defgroup OSyncObjFormatPrivateAPI OpenSync Object Format Private
 * @ingroup OSyncFormatPrivate
 * @brief The private part of the OSyncObjFormat API
 *
 */

/*@{*/

/*! @brief Represent a format for a object type
 */
struct OSyncObjFormat {
	int ref_count;
	/** The name of the format */
	char *name;
	/** The object type that is normally represented in this format.
	 * Example: A VCard normally represents a contact. so, objtype_name
	 * would be "contact" */
	char *objtype_name;

	/** userdata pointer returned by initialize_func */
	void *user_data;

	/** Initialize format plugin function */
	OSyncFormatInitializeFunc initialize_func;
	/** Finalize format plugin function */
	OSyncFormatFinalizeFunc finalize_func;

	OSyncFormatCompareFunc cmp_func;
	OSyncFormatDuplicateFunc duplicate_func;
	OSyncFormatCopyFunc copy_func;
	OSyncFormatCreateFunc create_func;
	OSyncFormatDestroyFunc destroy_func;
	OSyncFormatPrintFunc print_func;
	OSyncFormatRevisionFunc revision_func;
	OSyncFormatMarshalFunc marshal_func;
	OSyncFormatDemarshalFunc demarshal_func;
	OSyncFormatValidateFunc validate_func;
};

/*@}*/

#endif /* _OPENSYNC_OBJFORMAT_PRIVATE_H_ */

