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

#ifndef SCHNAPPS_PLUGIN_VOLUME_RENDER_H_
#define SCHNAPPS_PLUGIN_VOLUME_RENDER_H_

#include <schnapps/plugins/volume_render/dll.h>
#include <schnapps/plugins/volume_render/map_parameters.h>

#include <schnapps/core/types.h>
#include <schnapps/core/plugin_interaction.h>

#include <QAction>
#include <map>

namespace schnapps
{

class View;
namespace plugin_cmap_provider { class CMap3Handler; }

#ifdef USE_TRANSPARENCY
namespace plugin_render_transparency { class Plugin_RenderTransparency; }
#endif

namespace plugin_volume_render
{

class VolumeRender_DockTab;
using CMap3Handler = plugin_cmap_provider::CMap3Handler;

/**
* @brief Plugin for volume rendering
*/
class SCHNAPPS_PLUGIN_VOLUME_RENDER_API Plugin_VolumeRender : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_VolumeRender();
	inline ~Plugin_VolumeRender() override {}
	static QString plugin_name();

	MapParameters& parameters(View* view, CMap3Handler* mh);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	void draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	inline bool keyPress(View*, QKeyEvent*) override { return true; }
	inline bool keyRelease(View*, QKeyEvent*) override { return true; }
	bool mousePress(View*, QMouseEvent*) override;
	bool mouseRelease(View*, QMouseEvent*) override;
	bool mouseMove(View*, QMouseEvent*) override;
	inline bool wheelEvent(View*, QWheelEvent*) override { return true; }

	inline void resizeGL(View*, int, int) override {}

	void view_linked(View*) override;
	void view_unlinked(View*) override;

private slots:

	// slots called from View signals
	void object_linked(Object* o);
	void object_unlinked(Object* o);

private:

	void add_linked_map(View* view, CMap3Handler* mh);
	void remove_linked_map(View* view, CMap3Handler* mh);

private slots:

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();
	void linked_map_connectivity_changed();
	void linked_map_attribute_changed(cgogn::Orbit orbit, const QString& attribute_name);

	void viewer_initialized();

	void enable_on_selected_view(Plugin* p);

public slots:

	void set_position_vbo(View* view, CMap3Handler* mh, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_render_vertices(View* view, CMap3Handler* mh, bool b, bool update_dock_tab);
	void set_render_edges(View* view, CMap3Handler* mh, bool b, bool update_dock_tab);
	void set_render_faces(View* view, CMap3Handler* mh, bool b, bool update_dock_tab);
	void set_render_topology(View* view, CMap3Handler* mh, bool b, bool update_dock_tab);
	void set_apply_clipping_plane(View* view, CMap3Handler* mh, bool b, bool update_dock_tab);
	void set_vertex_color(View* view, CMap3Handler* mh, const QColor& color, bool update_dock_tab);
	void set_edge_color(View* view, CMap3Handler* mh, const QColor& color, bool update_dock_tab);
	void set_face_color(View* view, CMap3Handler* mh, const QColor& color, bool update_dock_tab);
	void set_vertex_scale_factor(View* view, CMap3Handler* mh, float32 sf, bool update_dock_tab);
	void set_volume_explode_factor(View* view, CMap3Handler* mh, float32 vef, bool update_dock_tab);
	void set_transparency_enabled(View* view, CMap3Handler* mh, bool b, bool update_dock_tab);
	void set_transparency_factor(View* view, CMap3Handler* mh, int32 tf, bool update_dock_tab);
	void set_filter(CMap3Handler* mh, cgogn::CellFilters* mask);

private:

	VolumeRender_DockTab* dock_tab_;
	std::map<View*, std::map<CMap3Handler*, MapParameters>> parameter_set_;

	bool setting_auto_enable_on_selected_view_;
	QString setting_auto_load_position_attribute_;

#ifdef USE_TRANSPARENCY
	plugin_render_transparency::Plugin_RenderTransparency* plugin_transparency_;
#endif
};

} // namespace plugin_volume_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_RENDER_H_
