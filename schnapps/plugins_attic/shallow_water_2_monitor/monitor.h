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

#ifndef SCHNAPPS_PLUGIN_SHALLOW_WATER_2_MONITOR_H_
#define SCHNAPPS_PLUGIN_SHALLOW_WATER_2_MONITOR_H_

#include "dll.h"
#include <schnapps/core/plugin_processing.h>

#include <schnapps/core/view.h>

#include <schnapps/plugins/shallow_water_2/shallow_water.h>
#include <schnapps/plugins/surface_render/surface_render.h>
#include <schnapps/plugins/surface_render_scalar/surface_render_scalar.h>

namespace schnapps
{

namespace plugin_shallow_water_2_monitor
{

/**
* @brief Shallow water 2 monitor plugin
*/
class SCHNAPPS_PLUGIN_SHALLOW_WATER_2_MONITOR_API Plugin_Shallow_Water_2_Monitor : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_Shallow_Water_2_Monitor();
	~Plugin_Shallow_Water_2_Monitor() override {}
	static QString plugin_name();

public slots:

	void run_script();
	void check_simu_state();

private:

	bool enable() override;
	void disable() override;

	MapHandlerGen* load(const QString& dir);

	View* v_;
	plugin_shallow_water_2::Plugin_ShallowWater* shallow_water_;
	plugin_surface_render::Plugin_SurfaceRender* render_;
	plugin_surface_render_scalar::Plugin_SurfaceRenderScalar* render_scalar_;

	std::vector<std::function<void()>> f_;
	uint32 current_f_;

    //chifaa

    //end chifaa

	QTimer* check_simu_timer_;

	QAction* shallow_water_action;
};

} // namespace plugin_shallow_water_2_monitor

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_2_MONITOR_H_
