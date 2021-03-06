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
 
#ifndef OPENSYNC_XMLFORMAT_INTERNALS_H_
#define OPENSYNC_XMLFORMAT_INTERNALS_H_

/**
 * @defgroup OSyncXMLFormatInternalAPI OpenSync XMLFormat Internals
 * @ingroup OSyncXMLFormatPrivate
 * @brief The private part of the OSyncXMLFormat
 * 
 */
/*@{*/

/**
 * @brief Get the name of the root node in a xmlformat
 * @param xmlformat The pointer to a xmlformat object
 * @return The name of the root node of the xmlformat
 */
const char *osync_xmlformat_root_name(OSyncXMLFormat *xmlformat);

/**
 * @brief Mark/Taint the xmlformat as unsorted
 * @param xmlformat The pointer to a xmlformat object
 */
void osync_xmlformat_set_unsorted(OSyncXMLFormat *xmlformat);

/*@}*/

#endif /* OPENSYNC_XMLFORMAT_INTERNAL_H_ */

