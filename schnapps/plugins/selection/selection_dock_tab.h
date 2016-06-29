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

#ifndef SCHNAPPS_PLUGIN_SELECTION_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SELECTION_DOCK_TAB_H_

#include <ui_selection.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

class SCHNApps;
class Plugin_Selection;

struct MapParameters;

class Selection_DockTab : public QWidget, public Ui::Selection_TabWidget
{
	Q_OBJECT

	friend class Plugin_Selection;

public:

	Selection_DockTab(SCHNApps* s, Plugin_Selection* p);

private slots:

	void position_attribute_changed(int index);
	void normal_attribute_changed(int index);
	void selection_method_changed(int index);
	void cell_type_changed(int index);
	void cells_set_changed(int index);
	void selected_map_cells_set_added(CellType ct, const QString& name);
	void selected_map_vertex_attribute_added(const QString& name);
	void selected_map_vertex_attribute_removed(const QString& name);
	void vertices_scale_factor_changed(int i);
	void color_changed(int i);
	void clear_clicked();

private:

	void set_current_cells_set(CellsSetGen* cs);
	void update_map_parameters(MapHandlerGen* map, const MapParameters& p);

	SCHNApps* schnapps_;
	Plugin_Selection* plugin_;

	bool updating_ui_;

	CellsSetGen* current_cells_set_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SELECTION_DOCK_TAB_H_
