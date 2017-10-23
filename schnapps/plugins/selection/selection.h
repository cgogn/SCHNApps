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

#ifndef SCHNAPPS_PLUGIN_SELECTION_H_
#define SCHNAPPS_PLUGIN_SELECTION_H_

#include <schnapps/plugins/selection/dll.h>

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

#include <cgogn/geometry/algos/selection.h>

#include <schnapps/plugins/selection/map_parameters.h>

namespace schnapps
{

namespace plugin_selection
{

class Selection_DockTab;

/**
* @brief Plugin for cells selection
*/
class SCHNAPPS_PLUGIN_SELECTION_API Plugin_Selection : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	using Vertex2 = CMap2::Vertex;
	using Edge2 = CMap2::Edge;
	using Face2 = CMap2::Face;
	using Vertex3 = CMap3::Vertex;
	using Edge3 = CMap3::Edge;
	using Face3 = CMap3::Face;
	using Volume = CMap3::Volume;
	using CollectorGen = cgogn::geometry::CollectorGen<VEC3>;

	Plugin_Selection() {}
	~Plugin_Selection() override {}

	MapParameters& get_parameters(View* view, MapHandlerGen* map);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	bool keyPress(View*, QKeyEvent*) override;
	bool keyRelease(View*, QKeyEvent*) override;
	bool mousePress(View*, QMouseEvent*) override;
	inline bool mouseRelease(View*, QMouseEvent*) override { return true; }
	bool mouseMove(View*, QMouseEvent*) override;
	bool wheelEvent(View*, QWheelEvent*) override;

	inline void resizeGL(View*, int, int) override {}

	void view_linked(View*) override;
	void view_unlinked(View*) override;

	std::unique_ptr<CollectorGen> collector_within_sphere(MapHandlerGen* map, float64 radius, const MapHandlerGen::Attribute_T<VEC3>& position_att);
	std::vector<cgogn::Dart> get_picked_cells(MapHandlerGen* map, CellType ct, const MapHandlerGen::Attribute_T<VEC3>& position_att, VEC3& A, VEC3& B);

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
	void linked_map_attribute_changed(cgogn::Orbit orbit, const QString& name);
	void linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void linked_map_cells_set_removed(CellType ct, const QString& name);
	void linked_map_bb_changed();

	void viewer_initialized();

public:

	void set_position_attribute(View* view, MapHandlerGen* map, const QString& name, bool update_dock_tab);
	void set_normal_attribute(View* view, MapHandlerGen* map, const QString& name, bool update_dock_tab);
	void set_cells_set(View* view, MapHandlerGen* map, CellsSetGen* cs, bool update_dock_tab);
	void set_selection_method(View* view, MapHandlerGen* map, MapParameters::SelectionMethod m, bool update_dock_tab);
	void set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf, bool update_dock_tab);
	void set_color(View* view, MapHandlerGen* map, const QColor& color, bool update_dock_tab);

private:

	Selection_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;

	QString setting_auto_load_position_attribute_;
};

} // namespace plugin_selection

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SELECTION_H_
