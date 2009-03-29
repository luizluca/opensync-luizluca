/*
 * libopensync - A synchronization framework
 * Copyright (C) 2009 Bjoern Ricks <bjoern.ricks@gmail.com>
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

#ifndef _OPENSYNC_CONVERTER_INTERNALS_H_
#define _OPENSYNC_CONVERTER_INTERNALS_H_

/** @brief Returns the number of converters in a converter path
 * @param path Pointer to the converter path
 * @returns the number of converters in the specified path
 */
OSYNC_TEST_EXPORT unsigned int osync_converter_path_num_edges(OSyncFormatConverterPath *path);

/** @brief Returns the nth converter in a converter path
 * @param path Pointer to the converter path
 * @param nth The position of the converter to retrieve
 * @returns the converter at the specified index
 */
OSYNC_TEST_EXPORT OSyncFormatConverter *osync_converter_path_nth_edge(OSyncFormatConverterPath *path, unsigned int nth);


#endif /* _OPENSYNC_CONVERTER_INTERNALS_H_ */
