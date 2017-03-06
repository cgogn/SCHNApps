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

#include "dll.h"
#include <ui_surface_render_scalar.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_surface_render_scalar
{

class Plugin_SurfaceRenderScalar;

struct MapParameters;

class SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_API SurfaceRenderScalar_DockTab : public QWidget, public Ui::SurfaceRenderScalar_TabWidget
{
	Q_OBJECT

	friend class Plugin_SurfaceRenderScalar;

public:

	SurfaceRenderScalar_DockTab(SCHNApps* s, Plugin_SurfaceRenderScalar* p);

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceRenderScalar* plugin_;

	bool updating_ui_;

private slots:

	void position_vbo_changed(int index);
	void selected_scalar_vbo_changed(QListWidgetItem* item, QListWidgetItem* old);
	void color_map_changed(int index);
	void expansion_changed(int i);
	void show_iso_lines_changed(bool b);
	void nb_iso_levels_changed(int i);

private:

	void add_position_vbo(const QString& name);
	void remove_position_vbo(const QString& name);
	void add_scalar_vbo(const QString& name);
	void remove_scalar_vbo(const QString& name);

	void update_map_parameters(MapHandlerGen* map, const MapParameters& p);
};

} // namespace plugin_surface_render_scalar

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_DOCK_TAB_H_
