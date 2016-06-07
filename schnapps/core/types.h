/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
*                                                                              *
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

#ifndef SCHNAPPS_CORE_TYPES_H_
#define SCHNAPPS_CORE_TYPES_H_

#include <cgogn/core/utils/numerics.h>
#include <cgogn/geometry/types/geometry_traits.h>

#include <Eigen/Dense>

namespace schnapps
{

using namespace cgogn::numerics;

using VEC4F = Eigen::Vector4f;
using VEC4D = Eigen::Vector4d;
using VEC3F = Eigen::Vector3f;
using VEC3D = Eigen::Vector3d;
using VEC2F = Eigen::Vector2f;
using VEC2D = Eigen::Vector2d;

using VEC4 = VEC4D;
using VEC3 = VEC3D;
using VEC2 = VEC2D;

using SCALAR = typename cgogn::geometry::vector_traits<VEC3>::Scalar;

} // namespace schnapps

#endif // SCHNAPPS_CORE_TYPES_H_
