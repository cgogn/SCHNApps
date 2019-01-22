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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_DOCK_TAB_H_

#include <schnapps/plugins/surface_render_scalar/plugin_surface_render_scalar_export.h>

#include <cgogn/rendering/shaders/shader_scalar_per_vertex.h>

#include <ui_surface_render_scalar.h>

namespace cgogn { namespace rendering { class VBO; } }

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap2Handler;
}

class SCHNApps;
class View;
class Object;

namespace plugin_surface_render_scalar
{

class Plugin_SurfaceRenderScalar;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

class PLUGIN_SURFACE_RENDER_SCALAR_EXPORT SurfaceRenderScalar_DockTab : public QWidget, public Ui::SurfaceRenderScalar_TabWidget
{
	Q_OBJECT

public:

	SurfaceRenderScalar_DockTab(SCHNApps* s, Plugin_SurfaceRenderScalar* p);
	~SurfaceRenderScalar_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceRenderScalar* plugin_;

	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;

	CMap2Handler* selected_map_;

	bool updating_ui_;

private slots:

	// slots called from UI signals
	void selected_map_changed();

	void position_vbo_changed(int index);
	void selected_scalar_vbo_changed(QListWidgetItem* item, QListWidgetItem* old);
	void color_map_changed(int index);
	void auto_update_min_max_changed(bool b);
	void scalar_min_changed(double d);
	void scalar_max_changed(double d);
	void expansion_changed(int i);
	void show_iso_lines_changed(bool b);
	void nb_iso_levels_changed(int i);

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);

	// slots called from View signals
	void object_linked(Object* o);
	void object_unlinked(Object* o);

	// slots called from CMap2Handler signals
	void selected_map_vbo_added(cgogn::rendering::VBO* vbo);
	void selected_map_vbo_removed(cgogn::rendering::VBO* vbo);

private:

	void map_linked(CMap2Handler* mh);
	void map_unlinked(CMap2Handler* mh);

public:

	// methods used to update the UI from the plugin
	void set_position_vbo(cgogn::rendering::VBO* vbo);
	void set_scalar_vbo(cgogn::rendering::VBO* vbo);
	void set_color_map(cgogn::rendering::ShaderScalarPerVertex::ColorMap cm);
	void set_auto_update_min_max(bool b);
	void set_scalar_min(double d);
	void set_scalar_max(double d);
	void set_expansion(int i);
	void set_show_iso_lines(bool b);
	void set_nb_iso_levels(int i);

	CMap2Handler* selected_map() { return selected_map_; }
	void refresh_ui();

private:

	// internal UI cascading updates
	void update_after_scalar_vbo_changed();
	void update_after_auto_update_min_max_changed();
};

} // namespace plugin_surface_render_scalar

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_DOCK_TAB_H_
