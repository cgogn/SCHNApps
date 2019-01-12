/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Image                                                                 *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* This library is free software; you can redistribute it and/or modify it      *
* under the terms of the GNU Lesser General Public License as published by the *
* Free Software Foundation; either version 2.1 of the License, or (at your     *
* option) any later version.                                                   *
*                                                                              *
* This library is distributed in the hope that it will be useful, but WITHOUT  *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
* for more details.                                                            *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with this library; if not, write to the Free Software Foundation,      *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
*                                                                              *
* Web site: http://cgogn.unistra.fr/                                           *
* Contact information: cgogn@unistra.fr                                        *
*                                                                              *
*******************************************************************************/

#ifndef SCHNAPPS_PLUGIN_IMAGE_CGAL_IMAGE_H_
#define SCHNAPPS_PLUGIN_IMAGE_CGAL_IMAGE_H_

#include <CGAL/Image_3.h>
#include "image.h"

namespace schnapps
{

namespace plugin_image
{

SCHNAPPS_PLUGIN_IMAGE_API CGAL::Image_3	export_to_cgal_image(const schnapps::plugin_image::Image3D& im);
SCHNAPPS_PLUGIN_IMAGE_API WORD_KIND		get_cgal_word_kind(cgogn::io::DataType data_type);
SCHNAPPS_PLUGIN_IMAGE_API SIGN			get_cgal_sign(cgogn::io::DataType data_type);

} // namespace plugin_image

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_IMAGE_CGAL_IMAGE_H_
