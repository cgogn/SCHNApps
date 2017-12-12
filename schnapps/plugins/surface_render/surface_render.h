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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_H_

#include <schnapps/plugins/surface_render/dll.h>

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <schnapps/plugins/surface_render/surface_render_dock_tab.h>
#include <schnapps/plugins/surface_render/map_parameters.h>

#include <QAction>
#include <map>

namespace schnapps
{

namespace plugin_surface_render
{

/**
* @brief Plugin for surface rendering
*/
class SCHNAPPS_PLUGIN_SURFACE_RENDER_API Plugin_SurfaceRender : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceRender();
	inline ~Plugin_SurfaceRender() override {}
	static QString plugin_name();

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

#ifdef USE_TRANSPARENCY
	void add_transparency(View* view, MapHandlerGen* map);
	void remove_transparency(View* view, MapHandlerGen* map);
	void change_transparency(View* view, MapHandlerGen* map);
#endif

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

	void viewer_initialized();

	void enable_on_selected_view(Plugin* p);

public slots:

	void set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_normal_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_color_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_render_vertices(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_render_edges(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_render_faces(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_render_backfaces(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_face_style(View* view, MapHandlerGen* map, MapParameters::FaceShadingStyle s, bool update_dock_tab);
	void set_render_boundary(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_vertex_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab);
	void set_edge_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab);
	void set_front_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab);
	void set_back_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab);
	void set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf, bool update_dock_tab);
	void set_transparency_enabled(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_transparency_factor(View* view, MapHandlerGen* map, int32 tf, bool update_dock_tab);

private:

	SurfaceRender_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;

	bool setting_auto_enable_on_selected_view_;
	QString setting_auto_load_position_attribute_;
	QString setting_auto_load_normal_attribute_;
	QString setting_auto_load_color_attribute_;

#ifdef USE_TRANSPARENCY
	PluginInteraction* plugin_transparency_;
#endif
};

} // namespace plugin_surface_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
