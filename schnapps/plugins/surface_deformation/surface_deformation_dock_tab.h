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

#ifndef SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_DOCK_TAB_H_

#include "dll.h"
#include <ui_surface_deformation.h>

#include <map_parameters.h>

namespace schnapps
{

class SCHNApps;
class View;
class MapHandlerGen;

namespace plugin_surface_deformation
{

class Plugin_SurfaceDeformation;

class SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_API SurfaceDeformation_DockTab : public QWidget, public Ui::SurfaceDeformation_TabWidget
{
	Q_OBJECT

public:

	SurfaceDeformation_DockTab(SCHNApps* s, Plugin_SurfaceDeformation* p);
	~SurfaceDeformation_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceDeformation* plugin_;

	CMap2Handler* selected_map_;
	bool updating_ui_;

private slots:

	// slots called from UI signals
	void position_attribute_changed(int index);
	void free_vertex_set_changed(int index);
	void handle_vertex_set_changed(int index);
	void start_stop_button_clicked();

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);
	void selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur);

	// slots called from MapHandlerGen signals
	void selected_map_cells_set_added(CellType ct, const QString& name);
	void selected_map_cells_set_removed(CellType ct, const QString& name);
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name);

public:

	// methods used to update the UI from the plugin
	void set_position_attribute(const QString& name);
	void set_free_vertex_set(CellsSetGen* cs);
	void set_handle_vertex_set(CellsSetGen* cs);
	void set_deformation_initialized(bool b);

	void refresh_ui();
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_DOCK_TAB_H_
