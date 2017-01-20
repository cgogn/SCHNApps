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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_H_
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_H_

#include "dll.h"
#include <volume_mesh_from_surface_dialog.h>
#include <schnapps/core/plugin_processing.h>
#include <cgogn/core/cmap/cmap2.h>

namespace schnapps
{

class MapHandlerGen;
template<typename>
class MapHandler;

namespace plugin_image
{
class Plugin_Image;
class Image3D;
} // namespace plugin_image

namespace plugin_vmfs
{

class Plugin_VolumeMeshFromSurface;

struct SCHNAPPS_PLUGIN_VMFS_API TetgenParameters
{
	TetgenParameters();
	std::string tetgen_command_line;
};

struct SCHNAPPS_PLUGIN_VMFS_API NetgenParameters
{
	NetgenParameters();
	bool uselocalh;

	double maxh;
	double minh;

	double fineness;
	double grading;

	double elementsperedge;
	double elementspercurve;

	bool closeedgeenable;
	double closeedgefact;

	bool minedgelenenable;
	double minedgelen;

	bool second_order;
	bool quad_dominated;

	char * meshsize_filename;

	bool optsurfmeshenable;
	bool optvolmeshenable;

	int optsteps_3d;
	int optsteps_2d;

	// Philippose - 13/09/2010
	// Added a couple more parameters into the meshing parameters list
	// from Netgen into Nglib
	bool invert_tets;
	bool invert_trigs;

	bool check_overlap;
	bool check_overlapping_boundary;
};

struct SCHNAPPS_PLUGIN_VMFS_API CGALParameters
{
	CGALParameters();

	float64 cell_size_;
	float64 cell_radius_edge_ratio_;
	float64 facet_angle_;
	float64 facet_size_;
	float64 facet_distance_;

	bool do_odt_;
	bool do_odt_freeze_;
	int32 odt_max_iter_;
	float64 odt_convergence_;
	float64 odt_freeze_bound_;

	bool do_lloyd_;
	bool do_lloyd_freeze_;
	int32 lloyd_max_iter_;
	float64 lloyd_convergence_;
	float64 lloyd_freeze_bound_;

	bool do_perturber_;
	float64 perturber_sliver_bound_;
	bool do_exuder_;
	float64 exuder_sliver_bound_;
};

struct SCHNAPPS_PLUGIN_VMFS_API MeshGeneratorParameters
{
	MeshGeneratorParameters();
	CGALParameters cgal;
	NetgenParameters netgen;
	TetgenParameters tetgen;
};

class SCHNAPPS_PLUGIN_VMFS_API Plugin_VolumeMeshFromSurface : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class VolumeMeshFromSurfaceDialog;

public:

	using Map2 = schnapps::CMap2;
	using Map3 = schnapps::CMap3;
	using MapHandler2 = schnapps::MapHandler<Map2>;
	using MapHandler3 = schnapps::MapHandler<Map3>;

	Plugin_VolumeMeshFromSurface();

	MapHandler3* generate_netgen(MapHandler2* mh2, CMap2::VertexAttribute<VEC3> position_att, const NetgenParameters& params);
	MapHandler3* generate_tetgen(MapHandler2* mh2, CMap2::VertexAttribute<VEC3> position_att, const std::string& tetgen_args);
	MapHandler3* generate_cgal(MapHandler2* mh2, CMap2::VertexAttribute<VEC3> position_att, const CGALParameters& params);
	MapHandler3* generate_cgal(plugin_image::Image3D const * im, const CGALParameters& params);

private:

	virtual bool enable() override;
	virtual void disable() override;

	QAction* gen_mesh_action_;
	plugin_image::Plugin_Image* plugin_image_;
	MeshGeneratorParameters generation_parameters_;
	std::unique_ptr<VolumeMeshFromSurfaceDialog> dialog_;

public slots:
	void generate_button_netgen_pressed();
	void generate_button_tetgen_pressed();
	void generate_button_cgal_pressed();
	void plugin_enabled(Plugin*);
	void plugin_disabled(Plugin*);
};

} // namespace plugin_vmfs

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_H_
