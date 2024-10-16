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

#ifndef SCHNAPPS_PLUGIN_VOLUME_RENDER_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_VOLUME_RENDER_DOCK_TAB_H_

#include <schnapps/plugins/volume_render/plugin_volume_render_export.h>
#include <schnapps/core/types.h>
#include <ui_volume_render.h>

#include <QColorDialog>

namespace cgogn {
    enum Orbit: numerics::uint32;
    namespace rendering { class VBO; }
}

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap3Handler;
}

class SCHNApps;
class View;
class Object;

namespace plugin_volume_render
{

class Plugin_VolumeRender;
using CMap3Handler = plugin_cmap_provider::CMap3Handler;

class PLUGIN_VOLUME_RENDER_EXPORT VolumeRender_DockTab : public QWidget, public Ui::VolumeRender_TabWidget
{
	Q_OBJECT

public:

	VolumeRender_DockTab(SCHNApps* s, Plugin_VolumeRender* p);
	~VolumeRender_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_VolumeRender* plugin_;

	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;

	CMap3Handler* selected_map_;

	bool updating_ui_;

	QColorDialog* color_dial_;
	int current_color_dial_;
	QColor vertex_color_;
	QColor edge_color_;
	QColor face_color_;

    QString previousSelection_;

private slots:

	// slots called from UI signals
	void selected_map_changed();

	void position_vbo_changed(int index);
	void render_vertices_changed(bool b);
	void render_edges_changed(bool b);
	void render_faces_changed(bool b);
	void render_topology_changed(bool b);
	void apply_clipping_plane_changed(bool b);
	void apply_grid_clipping_plane_changed(bool b);
	void vertex_scale_factor_changed(int i);
	void volume_explode_factor_changed(int i);
	void transparency_enabled_changed(bool b);
	void transparency_factor_changed(int n);

	void clipping_plane_i_changed(int i);
	void clipping_plane_j_changed(int i);
	void clipping_plane_k_changed(int i);

	void clipping_plane_i_changed_2(int i);
	void clipping_plane_j_changed_2(int i);
	void clipping_plane_k_changed_2(int i);

    void selected_volume_scalar_changed();
    void color_map_changed(int i);
	void vertex_color_clicked();
	void edge_color_clicked();
	void face_color_clicked();
	void color_selected();

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);

	// slots called from View signals
	void object_linked(Object* o);
	void object_unlinked(Object* o);

	// slots called from MapHandlerGen signals
	void selected_map_vbo_added(cgogn::rendering::VBO* vbo);
	void selected_map_vbo_removed(cgogn::rendering::VBO* vbo);
    void selected_map_attribute_added(cgogn::Orbit o, QString name);
    void selected_map_attribute_removed(cgogn::Orbit o, const QString& name);

private:

	void map_linked(CMap3Handler* mh);
	void map_unlinked(CMap3Handler* mh);

public:

	// methods used to update the UI from the plugin
	void set_position_vbo(cgogn::rendering::VBO* vbo);
	void set_render_vertices(bool b);
	void set_render_edges(bool b);
	void set_render_faces(bool b);
	void set_render_topology(bool b);
	void set_apply_clipping_plane(bool b);
	void set_apply_grid_clipping_plane(bool b);
	void set_vertex_color(const QColor& color);
	void set_edge_color(const QColor& color);
	void set_face_color(const QColor& color);
	void set_vertex_scale_factor(float sf);
	void set_volume_explode_factor(float vef);
	void set_transparency_enabled(bool b);
	void set_transparency_factor(int tf);

	void set_grid_clipping_plane(int x, int y, int z);
	void set_grid_clipping_plane2(int x, int y, int z);

	CMap3Handler* selected_map() { return selected_map_; }
	void refresh_ui();

private:

	// internal UI cascading updates
	void update_after_use_transparency_changed();
};

} // namespace plugin_volume_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_RENDER_DOCK_TAB_H_
