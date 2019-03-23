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

#ifndef SCHNAPPS_PLUGIN_SURFACE_SELECTION_H_
#define SCHNAPPS_PLUGIN_SURFACE_SELECTION_H_

#include <schnapps/plugins/surface_selection/plugin_surface_selection_export.h>
#include <schnapps/plugins/surface_selection/map_parameters.h>

#include <schnapps/core/types.h>
#include <schnapps/core/plugin_interaction.h>

#include <cgogn/geometry/algos/selection.h>

namespace schnapps
{

class View;
namespace plugin_cmap_provider { class CMap2Handler; }

namespace plugin_surface_selection
{

class SurfaceSelection_DockTab;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

/**
* @brief Plugin for cells selection
*/
class PLUGIN_SURFACE_SELECTION_EXPORT Plugin_SurfaceSelection : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	using CollectorGen = cgogn::geometry::CollectorGen<VEC3>;

	Plugin_SurfaceSelection();
	inline ~Plugin_SurfaceSelection() override {}
	static QString plugin_name();

	MapParameters& parameters(View* view, CMap2Handler* mh);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	void draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	bool keyPress(View*, QKeyEvent*) override;
	bool keyRelease(View*, QKeyEvent*) override;
	bool mousePress(View*, QMouseEvent*) override;
	inline bool mouseRelease(View*, QMouseEvent*) override { return true; }
	bool mouseMove(View*, QMouseEvent*) override;
	bool wheelEvent(View*, QWheelEvent*) override;

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

	// slots called from MapHandlerGen signals
	void linked_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void linked_map_attribute_changed(cgogn::Orbit orbit, const QString& name);
	void linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void linked_map_cells_set_removed(cgogn::Orbit orbit, const QString& name);
	void linked_map_bb_changed();
	void enable_on_selected_view(Plugin* p);

	void viewer_initialized();

public:

	void set_position_attribute(View* view, CMap2Handler* mh, const QString& name, bool update_dock_tab);
	void set_normal_attribute(View* view, CMap2Handler* mh, const QString& name, bool update_dock_tab);
	void set_cells_set(View* view, CMap2Handler* mh, CMapCellsSetGen* cs, bool update_dock_tab);
	void set_selection_method(View* view, CMap2Handler* mh, SelectionMethod m, bool update_dock_tab);
	void set_vertex_scale_factor(View* view, CMap2Handler* mh, float32 sf, bool update_dock_tab);
	void set_color(View* view, CMap2Handler* mh, const QColor& color, bool update_dock_tab);

private:

	SurfaceSelection_DockTab* dock_tab_;
	std::map<View*, std::map<CMap2Handler*, MapParameters>> parameter_set_;

	bool setting_auto_enable_on_selected_view_;
	QString setting_auto_load_position_attribute_;
};

} // namespace plugin_selection

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_SELECTION_H_
