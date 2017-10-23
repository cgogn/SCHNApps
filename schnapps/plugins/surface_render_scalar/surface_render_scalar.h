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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_

#include <schnapps/plugins/surface_render_scalar/dll.h>

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <schnapps/plugins/surface_render_scalar/surface_render_scalar_dock_tab.h>
#include <schnapps/plugins/surface_render_scalar/map_parameters.h>

namespace schnapps
{

namespace plugin_surface_render_scalar
{

/**
* @brief Plugin that renders color-coded scalar values on surface vertices
*/
class SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_API Plugin_SurfaceRenderScalar : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	inline Plugin_SurfaceRenderScalar() {}

	~Plugin_SurfaceRenderScalar() override {}

	MapParameters& get_parameters(View* view, MapHandlerGen* map);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	inline bool keyPress(View*, QKeyEvent*) override { return true; }
	inline bool keyRelease(View*, QKeyEvent*) override { return true; }
	inline bool mousePress(View*, QMouseEvent*) override { return true; }
	inline bool mouseRelease(View*, QMouseEvent*) override { return true; }
	inline bool mouseMove(View*, QMouseEvent*) override { return true; }
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

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();
	void linked_map_attribute_changed(cgogn::Orbit orbit, const QString& attribute_name);

	void viewer_initialized();

public slots:

	void set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_scalar_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_color_map(View* view, MapHandlerGen* map, cgogn::rendering::ShaderScalarPerVertex::ColorMap cm, bool update_dock_tab);
	void set_auto_update_min_max(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_scalar_min(View* view, MapHandlerGen* map, double d, bool update_dock_tab);
	void set_scalar_max(View* view, MapHandlerGen* map, double d, bool update_dock_tab);
	void set_expansion(View* view, MapHandlerGen* map, int32 i, bool update_dock_tab);
	void set_show_iso_lines(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_nb_iso_levels(View* view, MapHandlerGen* map, int32 i, bool update_dock_tab);

private:

	SurfaceRenderScalar_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;

	QString setting_auto_load_position_attribute_;
};

} // namespace plugin_surface_render_scalar

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_
