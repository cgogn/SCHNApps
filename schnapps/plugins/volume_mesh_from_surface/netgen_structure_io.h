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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_NETGEN_STRUCTURE_IO_H
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_NETGEN_STRUCTURE_IO_H

#include <functional>
#include <schnapps/plugins/volume_mesh_from_surface/dll.h>
#include <schnapps/core/map_handler.h>
#include <cgogn/io/volume_import.h>

namespace nglib
{
	class Ng_Meshing_Parameters;
} // namespace nglib

namespace schnapps
{

namespace plugin_vmfs
{

class NetgenParameters;

class SCHNAPPS_PLUGIN_VMFS_API NetgenStructureVolumeImport : public cgogn::io::VolumeImport<VEC3>
{
public:

	using Inherit = cgogn::io::VolumeImport<VEC3>;
	using Self = NetgenStructureVolumeImport;
	using Scalar = SCALAR;
	template <typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;

	explicit NetgenStructureVolumeImport(void** netgen_data);
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(NetgenStructureVolumeImport);

	bool import_netgen_structure();

private:
	void** volume_mesh_structure_;
};

SCHNAPPS_PLUGIN_VMFS_API std::unique_ptr<void*, std::function<void(void**)>> export_netgen(CMap2& map, const CMap2::VertexAttribute<VEC3>& pos);
SCHNAPPS_PLUGIN_VMFS_API nglib::Ng_Meshing_Parameters* setup_netgen_parameters(const NetgenParameters& params);

} // namespace plugin_vmfs
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_NETGEN_STRUCTURE_IO_H
