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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_

#include <schnapps/plugins/surface_render_vector/plugin_surface_render_vector_export.h>
#include <schnapps/plugins/surface_render_vector/map_parameters.h>

#include <schnapps/core/types.h>
#include <schnapps/core/plugin_interaction.h>

namespace schnapps
{

class View;
namespace plugin_cmap_provider { class CMap2Handler; }

namespace plugin_surface_render_vector
{

class SurfaceRenderVector_DockTab;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

/**
* @brief Plugin that renders vectors on surface vertices
*/
class PLUGIN_SURFACE_RENDER_VECTOR_EXPORT Plugin_SurfaceRenderVector : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceRenderVector();
	inline ~Plugin_SurfaceRenderVector() override {}
	static QString plugin_name();

	MapParameters& parameters(View* view, CMap2Handler* mh);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	void draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

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
	void object_linked(Object* o);
	void object_unlinked(Object* o);

private:

	void add_linked_map(View* view, CMap2Handler* mh);
	void remove_linked_map(View* view, CMap2Handler* mh);

private slots:

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();

	void viewer_initialized();

public:

	void set_position_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void add_vector_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void remove_vector_vbo(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_vector_scale_factor(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, float32 sf, bool update_dock_tab);
	void set_vector_color(View* view, CMap2Handler* mh, cgogn::rendering::VBO* vbo, const QColor& color, bool update_dock_tab);

private:

	SurfaceRenderVector_DockTab* dock_tab_;
	std::map<View*, std::map<CMap2Handler*, MapParameters>> parameter_set_;

	QString setting_auto_load_position_attribute_;
};

} // namespace plugin_surface_render_vector

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_
