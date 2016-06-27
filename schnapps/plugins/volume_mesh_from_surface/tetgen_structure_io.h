/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Mesh From Surface                                              *
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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_TETGEN_STRUCTURE_IO_H
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_TETGEN_STRUCTURE_IO_H

#include "dll.h"
#include <schnapps/core/map_handler.h>
#include <tetgen/tetgen.h>
#include <cgogn/io/volume_import.h>
#include <cgogn/geometry/types/geometry_traits.h>

namespace schnapps
{

namespace plugin_vmfs
{

class SCHNAPPS_PLUGIN_VMFS_API TetgenStructureVolumeImport : public cgogn::io::VolumeImport<schnapps::CMap3::MapTraits>
{
public:
	using Inherit = cgogn::io::VolumeImport<schnapps::CMap3::MapTraits>;
	using Self = TetgenStructureVolumeImport;
	using Scalar = typename cgogn::geometry::vector_traits<VEC3>::Scalar;
	template <typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;

	explicit TetgenStructureVolumeImport(tetgenio * tetgen_output);
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(TetgenStructureVolumeImport);

protected:
	virtual bool import_file_impl(const std::string& /*filename*/) override;

private:
	tetgenio* volume_;
};

SCHNAPPS_PLUGIN_VMFS_API std::unique_ptr<tetgenio> export_tetgen(CMap2& map, const CMap2::VertexAttribute<VEC3>& pos);

} // namespace plugin_vmfs
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_TETGEN_STRUCTURE_IO_H
