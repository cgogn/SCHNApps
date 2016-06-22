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

#ifndef SCHNAPPS_CORE_CONTROL_DOCK_MAP_TAB_H_
#define SCHNAPPS_CORE_CONTROL_DOCK_MAP_TAB_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/map_handler.h>

#include <ui_control_dock_map_tab_widget.h>

#include <QWidget>
#include <QString>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

class SCHNAPPS_CORE_API ControlDock_MapTab : public QWidget, public Ui::ControlDock_MapTabWidget
{
	Q_OBJECT

public:

	ControlDock_MapTab(SCHNApps* s);
	QString title() { return QString("Maps"); }

	MapHandlerGen* get_selected_map() { return selected_map_; }

	cgogn::Orbit get_current_orbit();

	void set_selected_map(const QString& map_name);

private slots:

	// slots called from UI actions
	void selected_map_changed();

	void duplicate_current_map_clicked();
	void remove_current_map_clicked();

	void show_bb_changed(bool b);
	void bb_vertex_attribute_changed(int index);
	void vertex_attribute_check_state_changed(QListWidgetItem* item);

	void cells_set_check_state_changed(QListWidgetItem* item);
	void add_cells_set();

	// slots called from SCHNApps signals
	void map_added(MapHandlerGen* m);
	void map_removed(MapHandlerGen* m);

	// slots called from selected MapHandler signals
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void selected_map_bb_vertex_attribute_changed(const QString& name);
	void selected_map_vbo_added(cgogn::rendering::VBO* vbo);
	void selected_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void selected_map_connectivity_changed();
	void selected_map_cells_set_added(cgogn::Orbit orbit, const QString& name);

private:

	void update_selected_map_info();

protected:

	SCHNApps* schnapps_;
	MapHandlerGen* selected_map_;
	bool updating_ui_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_CONTROL_DOCK_MAP_TAB_H_
