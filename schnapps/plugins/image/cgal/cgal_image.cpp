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

#define SCHNAPPS_PLUGIN_IMAGE_DLL_EXPORT

#include "cgal_image.h"

namespace schnapps
{

namespace plugin_image
{

SCHNAPPS_PLUGIN_IMAGE_API CGAL::Image_3 export_to_cgal_image(const Image3D& im)
{
	using DataType = cgogn::io::DataType;

	const auto& dims = im.get_image_dimensions();
	const auto& voxel_dims = im.get_voxel_dimensions();

	_image* image  =_createImage(dims[0], dims[1], dims[2],
			im.get_nb_components(),
			voxel_dims[0], voxel_dims[1], voxel_dims[2],
			im.get_data_size(),
			get_cgal_word_kind(im.get_data_type()),
			get_cgal_sign(im.get_data_type()));

	image->endianness = cgogn::internal::cgogn_is_little_endian? END_LITTLE : END_BIG;

	image->vectMode = im.get_nb_components() == 1u ? VM_SCALAR : VM_NON_INTERLACED;
	const auto& trans = im.get_translation();
	image->tx = trans[0];
	image->ty = trans[1];
	image->tz = trans[2];

	const auto& rot = im.get_rotation();
	image->rx = rot[0];
	image->ry = rot[1];
	image->rz = rot[2];

	const auto& origin = im.get_origin();
	image->cx = origin[0];
	image->cy = origin[1];
	image->cz = origin[2];


	std::memcpy(image->data,im.data(), dims[0]*dims[1]*dims[2]*image->wdim * image->vdim);

	return CGAL::Image_3(image);
}

SCHNAPPS_PLUGIN_IMAGE_API WORD_KIND get_cgal_word_kind(cgogn::io::DataType data_type)
{
	using DataType = cgogn::io::DataType;

	if (data_type == DataType::FLOAT || data_type == DataType::DOUBLE)
		return WK_FLOAT;
	else
		return WK_FIXED;
}

SCHNAPPS_PLUGIN_IMAGE_API SIGN get_cgal_sign(cgogn::io::DataType data_type)
{
	using DataType = cgogn::io::DataType;

	switch(data_type) {
		case DataType::UINT8:
		case DataType::UINT16:
		case DataType::UINT32:
		case DataType::UINT64: return SGN_UNSIGNED;
		default:
			return SGN_SIGNED;
	}
}


} // namespace plugin_image
} // namespace schnapps
