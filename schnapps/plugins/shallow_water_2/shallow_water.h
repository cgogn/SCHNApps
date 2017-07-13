/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#ifndef SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_
#define SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_

#include "dll.h"
#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>

#include <shallow_water_dock_tab.h>

namespace schnapps
{

namespace plugin_shallow_water_2
{

/**
* @brief Shallow water simulation
*/
class SCHNAPPS_PLUGIN_SHALLOW_WATER_2_API Plugin_ShallowWater : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_ShallowWater() {}
	~Plugin_ShallowWater() override {}

private:

	bool enable() override;
	void disable() override;

public slots:

	void init();
	void start();
	void stop();
	bool is_simu_running();

private slots:

	void update_draw_data();
	void update_time_step();
	void execute_time_step();

private:

	enum FaceType: uint8
	{
		TRI_CORNER = 0,
		TRI_CENTRAL,
		QUAD
	};

	void try_subdivision();
	void try_simplification();
	void subdivide_face(CMap2::Face f, CMap2::CellMarker<CMap2::Face::ORBIT>& subdivided);
	void simplify_face(CMap2::Face f);

	cgogn::Dart oldest_dart(CMap2::Face f);
	uint8 face_level(CMap2::Face f);
	FaceType face_type(CMap2::Face f);

	ShallowWater_DockTab* dock_tab_;

	SCALAR t_;
	SCALAR dt_;

	QTimer* draw_timer_;
	std::chrono::high_resolution_clock::time_point start_time_;
	std::future<void> simu_future_;
	std::atomic_bool simu_running_;
	std::mutex simu_data_access_;

	CMap2Handler* map_;
	CMap2* map2_;
	std::unique_ptr<CMap2::QuickTraversor> qtrav_;

	CMap2::VertexAttribute<VEC3> position_; // vertices position

	CMap2::CDartAttribute<uint8> dart_level_; // dart insertion level
	CMap2::FaceAttribute<uint32> face_subd_id_; // face subdivision id
	CMap2::FaceAttribute<bool> tri_face_; // face is triangle or not
};

} // namespace plugin_shallow_water_2

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_
