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

#include "dll.h"
#include <ui_selection.h>

#include <map_parameters.h>

#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace plugin_selection
{

class Plugin_Selection;

class SCHNAPPS_PLUGIN_SELECTION_API Selection_DockTab : public QWidget, public Ui::Selection_TabWidget
{
	Q_OBJECT

public:

	Selection_DockTab(SCHNApps* s, Plugin_Selection* p);
	~Selection_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_Selection* plugin_;

	MapHandlerGen* selected_map_;
	bool updating_ui_;

private slots:

	// slots called from UI signals
	void position_attribute_changed(int index);
	void normal_attribute_changed(int index);
	void cell_type_changed(int index);
	void cells_set_changed(int index);
	void selection_method_changed(int index);
	void clear_clicked();
	void vertex_scale_factor_changed(int i);
	void color_changed(int i);

	// slots called from SCHNApps signals
	void selected_view_changed(View* old, View* cur);
	void selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur);

	// slots called from MapHandlerGen signals
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void selected_map_cells_set_added(CellType ct, const QString& name);
	void selected_map_cells_set_removed(CellType ct, const QString& name);

public:

	// methods used to update the UI from the plugin
	void set_position_attribute(const QString& name);
	void set_normal_attribute(const QString& name);
	void set_cells_set(CellsSetGen* cs);
	void set_selection_method(MapParameters::SelectionMethod m);
	void set_vertex_scale_factor(float sf);
	void set_color(const QColor& color);

	void refresh_ui();

private:

	// internal UI cascading updates
	void update_after_cells_set_changed();
	void update_after_selection_method_changed();
};

} // namespace plugin_selection

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SELECTION_DOCK_TAB_H_
