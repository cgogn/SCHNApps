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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_

#include <schnapps/plugins/surface_render/dll.h>
#include <schnapps/plugins/surface_render/map_parameters.h>

#include <ui_surface_render.h>

#include <QColorDialog>

namespace cgogn { namespace rendering { class VBO; } }

namespace schnapps
{

namespace plugin_cmap2_provider
{
class Plugin_CMap2Provider;
class CMap2Handler;
}

class SCHNApps;
class View;
class Object;

namespace plugin_surface_render
{

class Plugin_SurfaceRender;
using CMap2Handler = plugin_cmap2_provider::CMap2Handler;

class SCHNAPPS_PLUGIN_SURFACE_RENDER_API SurfaceRender_DockTab : public QWidget, public Ui::SurfaceRender_TabWidget
{
	Q_OBJECT

public:

	SurfaceRender_DockTab(SCHNApps* s, Plugin_SurfaceRender* p);
	~SurfaceRender_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceRender* plugin_;

	plugin_cmap2_provider::Plugin_CMap2Provider* plugin_cmap2_provider_;

	CMap2Handler* selected_map_;

	bool updating_ui_;

	QColorDialog* color_dial_;
	int current_color_dial_;
	QColor vertex_color_;
	QColor edge_color_;
	QColor front_color_;
	QColor back_color_;

private slots:

	// slots called from UI signals
	void selected_map_changed();

	void position_vbo_changed(int index);
	void normal_vbo_changed(int index);
	void color_vbo_changed(int index);
	void render_vertices_changed(bool b);
	void vertex_scale_factor_changed(int i);
	void render_edges_changed(bool b);
	void render_faces_changed(bool b);
	void render_backfaces_changed(bool b);
	void face_style_changed(QAbstractButton* b);
	void render_boundary_changed(bool b);
	void transparency_enabled_changed(bool b);
	void transparency_factor_changed(int n);

	void vertex_color_clicked();
	void edge_color_clicked();
	void front_color_clicked();
	void back_color_clicked();
	void both_color_clicked();
	void color_selected();

	// slots called from SCHNApps signals
	void object_added(Object* o);
	void object_removed(Object* o);
	void selected_view_changed(View* old, View* cur);

	// slots called from CMap2Handler signals
	void selected_map_vbo_added(cgogn::rendering::VBO* vbo);
	void selected_map_vbo_removed(cgogn::rendering::VBO* vbo);

private:

	void map_added(CMap2Handler* mh);
	void map_removed(CMap2Handler* mh);

public:

	// methods used to update the UI from the plugin
	void set_position_vbo(cgogn::rendering::VBO* vbo);
	void set_normal_vbo(cgogn::rendering::VBO* vbo);
	void set_color_vbo(cgogn::rendering::VBO* vbo);
	void set_render_vertices(bool b);
	void set_render_edges(bool b);
	void set_render_faces(bool b);
	void set_render_backfaces(bool b);
	void set_face_style(MapParameters::FaceShadingStyle s);
	void set_render_boundary(bool b);
	void set_vertex_color(const QColor& color);
	void set_edge_color(const QColor& color);
	void set_front_color(const QColor& color);
	void set_back_color(const QColor& color);
	void set_vertex_scale_factor(float sf);
	void set_transparency_enabled(bool b);
	void set_transparency_factor(int tf);

	CMap2Handler* selected_map() { return selected_map_; }
	void refresh_ui();

private:

	// internal UI cascading updates
	void update_after_use_transparency_changed();
};

} // namespace plugin_surface_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_
