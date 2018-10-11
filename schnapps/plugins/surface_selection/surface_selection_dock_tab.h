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

#ifndef SCHNAPPS_PLUGIN_SURFACE_SELECTION_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SURFACE_SELECTION_DOCK_TAB_H_

#include <schnapps/plugins/surface_selection/dll.h>

#include <schnapps/core/types.h>

#include <ui_surface_selection.h>

namespace cgogn { enum Orbit: numerics::uint32; }

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

namespace plugin_surface_selection
{

enum SelectionMethod: unsigned int;
class Plugin_SurfaceSelection;
using CMap2Handler = plugin_cmap2_provider::CMap2Handler;
using CMap2CellsSetGen = plugin_cmap2_provider::CMap2CellsSetGen;

class SCHNAPPS_PLUGIN_SURFACE_SELECTION_API SurfaceSelection_DockTab : public QWidget, public Ui::SurfaceSelection_TabWidget
{
	Q_OBJECT

public:

	SurfaceSelection_DockTab(SCHNApps* s, Plugin_SurfaceSelection* p);
	~SurfaceSelection_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceSelection* plugin_;

	plugin_cmap2_provider::Plugin_CMap2Provider* plugin_cmap2_provider_;

	CMap2Handler* selected_map_;

	bool updating_ui_;

	cgogn::Orbit orbit_from_index(int index);
	int index_from_orbit(cgogn::Orbit orbit);

private slots:

	// slots called from UI signals
	void selected_map_changed();

	void position_attribute_changed(int index);
	void normal_attribute_changed(int index);
	void cell_type_changed(int index);
	void cells_set_changed(int index);
	void selection_method_changed(int index);
	void clear_clicked();
	void vertex_scale_factor_changed(int i);
	void color_changed(int i);

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);

	// slots called from View signals
	void object_linked(Object* o);
	void object_unlinked(Object* o);

	// slots called from CMap2Handler signals
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
	void set_normal_attribute(const QString& name);
	void set_cells_set(CMap2CellsSetGen* cs);
	void set_selection_method(SelectionMethod m);
	void set_vertex_scale_factor(float sf);
	void set_color(const QColor& color);

	CMap2Handler* selected_map() { return selected_map_; }
	void refresh_ui();

private:

	// internal UI cascading updates
	void update_after_cells_set_changed();
	void update_after_selection_method_changed();
};

} // namespace plugin_surface_selection

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_SELECTION_DOCK_TAB_H_
