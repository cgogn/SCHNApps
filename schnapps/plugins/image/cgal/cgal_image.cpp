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
	_image* image = ::_initImage();


	image->vectMode = im.get_nb_components() == 1u ? VM_SCALAR : VM_NON_INTERLACED;
	const auto& dims = im.get_image_dimensions();
	image->xdim = dims[0];
	image->ydim = dims[1];
	image->zdim = dims[2];
	image->vdim = im.get_nb_components();

	const auto& voxel_dims = im.get_voxel_dimensions();
	image->vx = voxel_dims[0];
	image->vy = voxel_dims[1];
	image->vz = voxel_dims[2];

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

	image->endianness = cgogn::internal::cgogn_is_little_endian? END_LITTLE : END_BIG;
	image->wdim = im.get_data_size();

	switch(im.get_data_type()) {
		case DataType::FLOAT:
		case DataType::DOUBLE: image->wordKind = WK_FLOAT; break;
		default:
			image->wordKind = WK_FIXED; break;
	}

	switch(im.get_data_type()) {
		case DataType::UINT8:
		case DataType::UINT16:
		case DataType::UINT32:
		case DataType::UINT64: image->sign = SGN_UNSIGNED; break;
		default:
			image->sign = SGN_SIGNED; break;
	}

	image->data = ::ImageIO_alloc(dims[0]*dims[1]*dims[2]*image->wdim * image->vdim);
	std::memcpy(image->data,im.data(), dims[0]*dims[1]*dims[2]*image->wdim * image->vdim);

	return CGAL::Image_3(image);
}


} // namespace plugin_image
} // namespace schnapps
