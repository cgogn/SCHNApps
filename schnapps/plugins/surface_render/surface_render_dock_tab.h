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

#include <ui_surface_render.h>

#include <QColorDialog>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;
class Plugin_SurfaceRender;

struct MapParameters;

class SurfaceRender_DockTab : public QWidget, public Ui::SurfaceRender_TabWidget
{
	Q_OBJECT

	friend class Plugin_SurfaceRender;

public:

	SurfaceRender_DockTab(SCHNApps* s, Plugin_SurfaceRender* p);

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceRender* plugin_;

	QColorDialog* color_dial_;
	int current_color_dial_;

	QColor vertex_color_;
	QColor edge_color_;
	QColor front_color_;
	QColor back_color_;

	bool updating_ui_;

private slots:

	void position_vbo_changed(int index);
	void normal_vbo_changed(int index);
	void color_vbo_changed(int index);
	void render_vertices_changed(bool b);
	void vertices_scale_factor_changed(int i);
	void render_edges_changed(bool b);
	void render_faces_changed(bool b);
	void face_style_changed(QAbstractButton* b);
	void render_boundary_changed(bool b);
	void render_backface_changed(bool b);

	void vertex_color_clicked();
	void edge_color_clicked();
	void front_color_clicked();
	void back_color_clicked();
	void both_color_clicked();
	void color_selected();

private:

	void add_position_vbo(QString name);
	void remove_position_vbo(QString name);
	void add_normal_vbo(QString name);
	void remove_normal_vbo(QString name);
	void add_color_vbo(QString name);
	void remove_color_vbo(QString name);

	void update_map_parameters(MapHandlerGen* map, const MapParameters& p);
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_
