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

#ifndef SCHNAPPS_PLUGIN_POINT_SET_RENDER_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_POINT_SET_RENDER_DOCK_TAB_H_

#include <schnapps/plugins/point_set_render/plugin_point_set_render_export.h>
#include <schnapps/plugins/point_set_render/map_parameters.h>

#include <ui_point_set_render.h>

#include <QColorDialog>

namespace cgogn { namespace rendering { class VBO; } }

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap0Handler;
}

class SCHNApps;
class View;
class Object;

namespace plugin_point_set_render
{

class Plugin_PointSetRender;
using CMap0Handler = plugin_cmap_provider::CMap0Handler;

class PLUGIN_POINT_SET_RENDER_EXPORT PointSetRender_DockTab : public QWidget, public Ui::PointSetRender_TabWidget
{
	Q_OBJECT

public:

	PointSetRender_DockTab(SCHNApps* s, Plugin_PointSetRender* p);
	~PointSetRender_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_PointSetRender* plugin_;

	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;

	CMap0Handler* selected_map_;

	bool updating_ui_;

	QColorDialog* color_dial_;
	int current_color_dial_;
	QColor vertex_color_;

private slots:

	// slots called from UI signals
	void selected_map_changed();

	void position_vbo_changed(int index);

	void color_vbo_changed(int index);
	void render_vertices_changed(bool b);
	void vertex_scale_factor_changed(int i);

	void vertex_color_clicked();
	void color_selected();

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);

	// slots called from View signals
	void object_linked(Object* o);
	void object_unlinked(Object* o);

	// slots called from CMap0Handler signals
	void selected_map_vbo_added(cgogn::rendering::VBO* vbo);
	void selected_map_vbo_removed(cgogn::rendering::VBO* vbo);

private:

	void map_linked(CMap0Handler* mh);
	void map_unlinked(CMap0Handler* mh);

public:

	// methods used to update the UI from the plugin
	void set_position_vbo(cgogn::rendering::VBO* vbo);
	void set_color_vbo(cgogn::rendering::VBO* vbo);
	void set_render_vertices(bool b);
	void set_vertex_color(const QColor& color);
	void set_vertex_scale_factor(float sf);


	CMap0Handler* selected_map() { return selected_map_; }
	void refresh_ui();

};

} // namespace plugin_surface_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_POINT_SET_RENDER_DOCK_TAB_H_
