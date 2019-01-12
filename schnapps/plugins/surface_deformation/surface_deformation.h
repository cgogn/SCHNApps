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

#include <schnapps/plugins/surface_deformation/dll.h>
#include <schnapps/plugins/surface_deformation/map_parameters.h>

#include <schnapps/core/types.h>
#include <schnapps/core/plugin_interaction.h>

namespace schnapps
{

class View;
namespace plugin_cmap_provider { class CMap2Handler; }

namespace plugin_surface_deformation
{

class SurfaceDeformation_DockTab;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

/**
* @brief Surface deformation
*/
class SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_API Plugin_SurfaceDeformation : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceDeformation();
	~Plugin_SurfaceDeformation() override {}
	static QString plugin_name();

	MapParameters& parameters(CMap2Handler* mh);
	bool check_docktab_activation();

private:

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4&, const QMatrix4x4&) override {}
	void draw_object(View*, Object*, const QMatrix4x4&, const QMatrix4x4&) override {}

	bool keyPress(View* view, QKeyEvent* event) override;
	inline bool keyRelease(View*, QKeyEvent*) override { return true; }
	inline bool mousePress(View*, QMouseEvent*) override { return true; }
	inline bool mouseRelease(View*, QMouseEvent*) override { return true; }
	bool mouseMove(View* view, QMouseEvent* event) override;
	inline bool wheelEvent(View*, QWheelEvent*) override { return true; }

	inline void resizeGL(View*, int, int) override {}

	void start_dragging(View* view);
	void stop_dragging(View* view);

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
	void linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void linked_map_cells_set_removed(cgogn::Orbit orbit, const QString& name);

public slots:

	void set_position_attribute(CMap2Handler* mh, const QString& name, bool update_dock_tab);
	void set_free_vertex_set(CMap2Handler* mh, CMap2CellsSet<CMap2::Vertex>* cs, bool update_dock_tab);
	void set_handle_vertex_set(CMap2Handler* mh, CMap2CellsSet<CMap2::Vertex>* cs, bool update_dock_tab);

	void initialize(CMap2Handler* mh, bool update_dock_tab);
	void stop(CMap2Handler* mh, bool update_dock_tab);

	bool as_rigid_as_possible(CMap2Handler* mh);

private:

	SurfaceDeformation_DockTab* dock_tab_;
	std::map<CMap2Handler*, MapParameters> parameter_set_;

	QString setting_auto_load_position_attribute_;

	bool drag_init_;
	bool dragging_;
	SCALAR drag_z_;
	qoglviewer::Vec drag_previous_;
};

} // namespace plugin_surface_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DEFORMATION_H_
