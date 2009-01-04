/*
 * libopensync - A synchronization framework
 * Copyright (C) 2008  Bjoern Ricks <bjoern.ricks@gmail.com>
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
 * Author: Bjoern Ricks <bjoern.ricks@gmail.com>
 *
 */

#ifndef OPENSYNC_XMLFORMAT_SCHEMA_H_
#define OPENSYNC_XMLFORMAT_SCHEMA_H_

/**
 * @defgroup OSyncXMFormatSchemaAPI OpenSync XMLFormat Schema
 * @ingroup OSyncXMLFormat
 * @brief The public part of the OSyncXMLFormatSchema
 *
 */
/*@{*/

/**
 * @brief Create new OSyncXMLFormatSchema for a certain object type
 *
 * @param objtype The object type name to create a OSyncXMLFormatSchema object for 
 * @param error The error which will hold the info in case of an error
 * @return new OSyncXMLFormatSchema or NULL in case of an error
 */
OSYNC_EXPORT OSyncXMLFormatSchema *osync_xmlformat_schema_new(const char *objtype, OSyncError **error);

/**
 * @brief Decrement the reference counter. The OSyncXMLFormatSchema object will
 *  be freed if the reference count reaches zero.
 * @param schema Pointer to the OSyncXMLFormatSchema to be freed
 */
OSYNC_EXPORT void osync_xmlformat_schema_unref(OSyncXMLFormatSchema *schema);

/**
 * @brief Increments the reference counter
 * @param osyncschema Pointer to the OSyncXMLFormatSchema object
 */
OSYNC_EXPORT OSyncXMLFormatSchema *osync_xmlformat_schema_ref(OSyncXMLFormatSchema *osyncschema);

/**
 * @brief Validate an xmlformat against a schema
 * @param xmlformat Pointer to the xmlformat object to validate
 * @param schema Pointer to the OSyncXMLFormatSchema object
 * @param error The error which will hold the info in case of an error
 * @return TRUE if the xmlformat is valid, FALSE otherwise
 */
OSYNC_EXPORT osync_bool osync_xmlformat_schema_validate(OSyncXMLFormatSchema *schema, OSyncXMLFormat *xmlformat, OSyncError **error);

/*@}*/

#endif /* OPENSYNC_XMLFORMAT_SCHEMA_H_ */
