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

#include <schnapps/core/dll.h>

#include <cgogn/core/utils/numerics.h>
#include <cgogn/geometry/types/geometry_traits.h>
#include <unsupported/Eigen/AlignedVector3>

namespace cgogn
{

class MapBaseData;

struct CMap2Type;
template <typename MAP_TYPE>
class CMap2_T;

struct CMap3Type;
template <typename MAP_TYPE>
class CMap3_T;

}

namespace schnapps
{

using namespace cgogn::numerics;

using MapBaseData = cgogn::MapBaseData;
using CMap2 = cgogn::CMap2_T<cgogn::CMap2Type>;
using CMap3 = cgogn::CMap3_T<cgogn::CMap3Type>;

enum CellType : uint16
{
	Dart_Cell = 0,
	Vertex_Cell,
	Edge_Cell,
	Face_Cell,
	Volume_Cell,
	Unknown
};

static const std::size_t NB_CELL_TYPES = CellType::Volume_Cell + 1;

SCHNAPPS_CORE_API std::string cell_type_name(CellType ct);
SCHNAPPS_CORE_API CellType cell_type(const std::string& name);

using VEC4F = Eigen::Vector4f;
using VEC4D = Eigen::Vector4d;
using VEC3F = Eigen::Vector3f;
using VEC3D = Eigen::Vector3d;
using VEC2F = Eigen::Vector2f;
using VEC2D = Eigen::Vector2d;

using MAT2F = Eigen::Matrix2f;
using MAT2D = Eigen::Matrix2d;
using MAT3F = Eigen::Matrix3f;
using MAT3D = Eigen::Matrix3d;
using MAT4F = Eigen::Matrix4f;
using MAT4D = Eigen::Matrix4d;

using AVEC3F = Eigen::AlignedVector3<float32>;
using AVEC3D = Eigen::AlignedVector3<float64>;

#ifdef SCHNAPPS_DOUBLE_PRECISION
using VEC2 = VEC2D;
#ifdef SCHNAPPS_USE_ALIGNEDVEC3
using VEC3 = AVEC3D;
#else
using VEC3 = VEC3D;
#endif
using VEC4 = VEC4D;

using MAT22 = MAT2D;
using MAT33 = MAT3D;
using MAT44 = MAT4D;
#else
#ifdef SCHNAPPS_SINGLE_PRECISION
using VEC4 = VEC4F;
#ifdef SCHNAPPS_USE_ALIGNEDVEC3
using VEC3 = AVEC3F;
#else
using VEC3 = VEC3F;
#endif
using VEC2 = VEC2F;

using MAT22 = MAT2F;
using MAT33 = MAT3F;
using MAT44 = MAT4F;
#else
#error Neither SCHNAPPS_SINGLE_PRECISION or SCHNAPPS_DOUBLE_PRECISION is defined.
#endif
#endif

using SCALAR = cgogn::geometry::vector_traits<VEC3>::Scalar;

template <typename MAP_TYPE>
class MapHandler;

using CMap2Handler = MapHandler<CMap2>;
using CMap3Handler = MapHandler<CMap3>;

} // namespace schnapps

#endif // SCHNAPPS_CORE_TYPES_H_
