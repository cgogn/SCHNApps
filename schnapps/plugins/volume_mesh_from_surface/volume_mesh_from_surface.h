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

#include <volume_mesh_from_surface_dock_tab.h>
#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace plugin_vmfs
{

class Plugin_VolumeMeshFromSurface;

struct MapParameters
{
	friend class Plugin_VolumeMeshFromSurface;

	std::string tetgen_command_line;

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

	MapParameters();
};

class Plugin_VolumeMeshFromSurface : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class VolumeMeshFromSurface_DockTab;
public:
	using Map2 = schnapps::CMap2;
	using Map3 = schnapps::CMap3;
	using MapHandler2 = schnapps::MapHandler<Map2>;
	using MapHandler3 = schnapps::MapHandler<Map3>;

private:
	virtual bool enable() override;
	virtual void disable() override;

	std::map<MapHandlerGen*, MapParameters> parameter_set_;
	std::unique_ptr<VolumeMeshFromSurface_DockTab> dock_tab_;
	QString	tetgen_args;

private slots:
	void selected_map_changed(MapHandlerGen*, MapHandlerGen*);

public slots:
	void generate_button_tetgen_pressed();
	void generate_button_cgal_pressed();
	void tetgen_args_updated(QString str);
};

} // namespace plugin_vmfs
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_H_
