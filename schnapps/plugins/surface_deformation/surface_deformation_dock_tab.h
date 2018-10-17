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

#include <schnapps/plugins/surface_deformation/dll.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/types.h>

#include <ui_surface_deformation.h>

namespace schnapps
{

namespace plugin_cmap2_provider
{
class Plugin_CMap2Provider;
class CMap2Handler;
class CMap2CellsSetGen;
}

class SCHNApps;
class View;
class Object;

namespace plugin_surface_deformation
{

class Plugin_SurfaceDeformation;
using CMap2Handler = plugin_cmap2_provider::CMap2Handler;
template <typename CELL>
using CMap2CellsSet = plugin_cmap2_provider::CMap2CellsSet<CELL>;

class SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_API SurfaceDeformation_DockTab : public QWidget, public Ui::SurfaceDeformation_TabWidget
{
	Q_OBJECT

public:

	SurfaceDeformation_DockTab(SCHNApps* s, Plugin_SurfaceDeformation* p);
	~SurfaceDeformation_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceDeformation* plugin_;

	plugin_cmap2_provider::Plugin_CMap2Provider* plugin_cmap2_provider_;

	CMap2Handler* selected_map_;

	bool updating_ui_;

private slots:

	// slots called from UI signals
	void selected_map_changed();

	void position_attribute_changed(int index);
	void free_vertex_set_changed(int index);
	void handle_vertex_set_changed(int index);
	void start_stop_button_clicked();

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);

	// slots called from View signals
	void object_linked(Object* o);
	void object_unlinked(Object* o);

	// slots called from MapHandlerGen signals
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void selected_map_cells_set_added(cgogn::Orbit orbit, const QString& name);
	void selected_map_cells_set_removed(cgogn::Orbit orbit, const QString& name);

private:

	void map_linked(CMap2Handler* mh);
	void map_unlinked(CMap2Handler* mh);

public:

	// methods used to update the UI from the plugin
	void set_position_attribute(const QString& name);
	void set_free_vertex_set(CMap2CellsSet<CMap2::Vertex>* cs);
	void set_handle_vertex_set(CMap2CellsSet<CMap2::Vertex>* cs);
	void set_deformation_initialized(bool b);

	CMap2Handler* selected_map() { return selected_map_; }
	void refresh_ui();
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_DOCK_TAB_H_
