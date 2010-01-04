/*
 * libopensync - A synchronization framework
 * Copyright (C) 2006  NetNix Finland Ltd <netnix@netnix.fi>
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
 * Author: Daniel Friedrich <daniel.friedrich@opensync.org>
 *
 */

#ifndef OPENSYNC_XMLFIELD_INTERNALS_H_
#define OPENSYNC_XMLFIELD_INTERNALS_H_

/**
 * @defgroup OSyncXMLFieldInternalAPI OpenSync XMLField Internals
 * @ingroup OSyncXMLFormatPrivate
 * @brief The internal part of the OSyncXMLField
 *
 */
/*@{*/

/**
 * @brief Creates a new xmlfield object from xmlNodePtr.
 *  The returned object will be freed with the xmlformat object.
 * @param node The node must be already inserted at the end of childs of the xmlDoc root element
 * @param error The error which will hold the info in case of an error
 * @return The pointer to the newly allocated xmlfield object or NULL in case of error
 */
OSyncXMLField *osync_xmlfield_new_node(xmlNodePtr node, OSyncError **error);

osync_bool osync_xmlfield_parse(OSyncXMLField *parent, xmlNodePtr node, OSyncXMLField **first_child, OSyncXMLField **last_child, OSyncError **error);

/**
 * @brief Frees a already unlinked xmlfield object
 * @param xmlfield The pointer to a xmlfield object
 */
void osync_xmlfield_free(OSyncXMLField *xmlfield);


/**
 * @brief Links a xmlfield object from a xmlformat object after a other xmlfield object of a other xmlformat object
 * @param xmlfield The pointer to a xmlfield object
 * @param to_link The pointer to a xmlfield object
 */
void osync_xmlfield_adopt_xmlfield_after_field(OSyncXMLField *xmlfield, OSyncXMLField *to_link);

/**
 * @brief Compare the names of two xmlfields
 * @param xmlfield1 The pointer to a xmlfield object
 * @param xmlfield2 The pointer to a xmlfield object
 * @returns same as strcmp(), 0 is equal
 */
int osync_xmlfield_compare_stdlib(const void *xmlfield1, const void *xmlfield2);

/**
 * @brief Compare the key names of two xmlfield key nodes
 * @param key1 The pointer to a xmlNodePtr
 * @param key2 The pointer to a xmlNodePtr
 * @returns same as strcmp(), 0 is equal
 */
int osync_xmlfield_key_compare_stdlib(const void *key1, const void *key2);

/**
 * @brief Get the last child xmlfield
 * @param xmlfield Pointer to the xmlfield object
 * @return A pointer to the last child xmlfield or NULL if there is no child xmlfield
 */
OSYNC_TEST_EXPORT OSyncXMLField *osync_xmlfield_get_last_child(OSyncXMLField *xmlfield);

/*@}*/

#endif /*OPENSYNC_XMLFIELD_INTERNALS_H_*/
