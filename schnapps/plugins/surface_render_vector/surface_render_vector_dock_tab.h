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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_DOCK_TAB_H_

#include "dll.h"
#include <ui_surface_render_vector.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_surface_render_vector
{

class Plugin_SurfaceRenderVector;

struct MapParameters;

class SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_API SurfaceRenderVector_DockTab : public QWidget, public Ui::SurfaceRenderVector_TabWidget
{
	Q_OBJECT

	friend class Plugin_SurfaceRenderVector;

public:

	SurfaceRenderVector_DockTab(SCHNApps* s, Plugin_SurfaceRenderVector* p);

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceRenderVector* plugin_;

	bool updating_ui_;

private slots:

	void position_vbo_changed(int index);
	void selected_vector_vbo_changed(QListWidgetItem* item, QListWidgetItem* old);
	void vector_vbo_checked(QListWidgetItem* item);
	void vector_scale_factor_changed(int i);
	void vector_color_changed(int i);

private:

	void add_position_vbo(QString name);
	void remove_position_vbo(QString name);
	void add_vector_vbo(QString name);
	void remove_vector_vbo(QString name);

	void update_map_parameters(MapHandlerGen* map, const MapParameters& p);
};

} // namespace plugin_surface_render_vector
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_DOCK_TAB_H_
