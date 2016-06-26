/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Render                                                         *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* Inspired by the surface render plugin                                        *
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

#include <ui_volume_render.h>

#include <QColorDialog>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_volume_render
{

class Plugin_VolumeRender;
struct MapParameters;

class VolumeRender_DockTab : public QWidget, public Ui::VolumeRender_TabWidget
{
	Q_OBJECT

	friend class Plugin_VolumeRender;

public:

	VolumeRender_DockTab(SCHNApps* s, Plugin_VolumeRender* p);

private:

	SCHNApps* schnapps_;
	Plugin_VolumeRender* plugin_;

	QColorDialog* color_dial_;
	int current_color_dial_;

	QColor vertex_color_;
	QColor edge_color_;
	QColor face_color_;

	bool updating_ui_;

private slots:

	void position_vbo_changed(int index);
	void color_vbo_changed(int index);
	void render_vertices_changed(bool b);
	void vertices_scale_factor_changed(int i);
	void vertices_scale_factor_pressed();
	void render_edges_changed(bool b);
	void render_faces_changed(bool b);
	void render_boundary_changed(bool b);
	void explode_volumes_changed(int);

	void vertex_color_clicked();
	void edge_color_clicked();
	void face_color_clicked();
	void color_selected();

private:

	void add_position_vbo(QString name);
	void remove_position_vbo(QString name);
	void add_color_vbo(QString name);
	void remove_color_vbo(QString name);

	void update_map_parameters(MapHandlerGen* map, const MapParameters& p);
};

} // namespace plugin_volume_render
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_RENDER_DOCK_TAB_H_
