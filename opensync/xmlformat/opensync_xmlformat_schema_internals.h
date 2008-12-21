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

#ifndef OPENSYNC_XMLFORMAT_SCHEMA_INTERNALS_H_
#define OPENSYNC_XMLFORMAT_SCHEMA_INTERNALS_H_

/**
 * @defgroup OSyncXMLFormatSchemaInternlaAPI OpenSync XMLFormat Schema Internals
 * @ingroup OSyncXMLFormatPrivate
 * @brief The internal part of the OSyncXMLFormatSchema
 *
 */
/*@{*/

/**
 * @brief Create new OSyncXMLFormatSchema for xmlformat
 * @param xmlformat The pointer to a xmlformat object. xmlformat->objtype is used to identify the schema file
 * @param path The individual schema path. If NULL the default OPENSYNC_SCHEMASDIR is used.
 * @param error The error which will hold the info in case of an error
 * @return new OSyncXMLFormatSchema or NULL in case of an error
 */
OSYNC_TEST_EXPORT OSyncXMLFormatSchema *osync_xmlformat_schema_new(OSyncXMLFormat *xmlformat, const char *path, OSyncError **error);


/**
 * @brief Get a  schema for the xmlformat.
 *
 * This function creates only one instance of a schema for each objtype. If a xmlformat is passed as a parameter with the same
 * objtype as a xmlformat prior the returned pointer to a OSyncXMLFormatSchema instance is the same as before.
 *
 * @param xmlformat The pointer to a xmlformat object
 * @param schemadir Path of the dir where the schema files are found.
 * @param error The error which will hold the info in case of an error
 * @return Pointer to a instance of OSyncXMLFormatSchema
 */
OSYNC_TEST_EXPORT OSyncXMLFormatSchema *osync_xmlformat_schema_get_instance_with_path(OSyncXMLFormat *xmlformat, const char *schemadir, OSyncError **error);

#endif /* OPENSYNC_XMLFORMAT_SCHEMA_INTERNALS_H_ */
