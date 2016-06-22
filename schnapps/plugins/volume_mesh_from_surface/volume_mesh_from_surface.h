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

#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>
#include <volume_mesh_from_surface_dock_tab.h>
#include <QAction>
#include <memory>

namespace schnapps
{

class Plugin_VolumeMeshFromSurface : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:
	using Map2 = schnapps::CMap2;
	using Map3 = schnapps::CMap3;
	using MapHandler2 = schnapps::MapHandler<Map2>;
	using MapHandler3 = schnapps::MapHandler<Map3>;
private:
	virtual bool enable() override;
	virtual void disable() override;

	std::unique_ptr<VolumeMeshFromSurface_DockTab> dock_tab_;
private:
	QString	tetgen_args;

public slots:
	void generate_button_pressed();
	void tetgen_args_updated(QString str);
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_H_
