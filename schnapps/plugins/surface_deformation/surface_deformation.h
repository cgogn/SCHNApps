/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#ifndef SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_H_
#define SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_H_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>

#include <surface_deformation_dock_tab.h>

#include <map_parameters.h>

namespace schnapps
{

namespace plugin_surface_deformation
{

/**
* @brief Surface deformation
*/
class SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_API Plugin_SurfaceDeformation : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceDeformation() :
		drag_init_(false),
		dragging_(false)
	{}
	~Plugin_SurfaceDeformation() override {}

	MapParameters& get_parameters(MapHandlerGen* map);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	inline void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override {}

	bool keyPress(View* view, QKeyEvent* event) override;
	inline bool keyRelease(View*, QKeyEvent*) override { return true; }
	inline bool mousePress(View*, QMouseEvent*) override { return true; }
	inline bool mouseRelease(View*, QMouseEvent*) override { return true; }
	bool mouseMove(View* view, QMouseEvent* event) override;
	inline bool wheelEvent(View*, QWheelEvent*) override { return true; }

	inline void resizeGL(View*, int, int) override {}

	void view_linked(View*) override;
	void view_unlinked(View*) override;

private slots:

	// slots called from View signals
	void map_linked(MapHandlerGen* map);
	void map_unlinked(MapHandlerGen* map);

private:

	void add_linked_map(View* view, MapHandlerGen* map);
	void remove_linked_map(View* view, MapHandlerGen* map);

private slots:

	// slots called from MapHandlerGen signals
	void linked_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void linked_map_cells_set_removed(CellType ct, const QString& name);

public slots:

	void set_position_attribute(MapHandlerGen* map, const QString& name, bool update_dock_tab);
	void set_free_vertex_set(MapHandlerGen* map, CellsSetGen* cs, bool update_dock_tab);
	void set_handle_vertex_set(MapHandlerGen* map, CellsSetGen* cs, bool update_dock_tab);
	void start_stop(MapHandlerGen* map, bool update_dock_tab);

	void as_rigid_as_possible(MapHandlerGen* map);

private:

	SurfaceDeformation_DockTab* dock_tab_;
	std::map<MapHandlerGen*, MapParameters> parameter_set_;

	QString setting_auto_load_position_attribute_;

	bool drag_init_;
	bool dragging_;
	SCALAR drag_z_;
	qoglviewer::Vec drag_previous_;
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_H_
