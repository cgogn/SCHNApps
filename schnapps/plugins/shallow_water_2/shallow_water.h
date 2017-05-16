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
	bool is_running();

private slots:

	void execute_time_step();
	void try_subdivision();
	void try_simplification();
	void subdivide_face(CMap2::Face f);
	void simplify_face(CMap2::Face f);

	cgogn::Dart oldest_dart(CMap2::Face f);

private:

	ShallowWater_DockTab* dock_tab_;

	SCALAR t_;
	SCALAR dt_;
	QTimer* timer_;
	bool connectivity_changed_;

	CMap2Handler* map_;
	CMap2* map2_;

	CMap2::VertexAttribute<VEC3> position_; // vertices position

	CMap2::CDartAttribute<uint8> dart_level_; // dart insertion level
	CMap2::FaceAttribute<uint8> face_level_; // face level
	CMap2::FaceAttribute<uint8> face_type_; // face type : 0 -> corner, 1 -> center
};

} // namespace plugin_shallow_water_2

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_
