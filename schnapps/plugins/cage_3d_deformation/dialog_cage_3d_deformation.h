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

#ifndef SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_DIALOG_H_
#define SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_DIALOG_H_

#include <schnapps/plugins/cage_3d_deformation/dll.h>

#include <schnapps/core/map_handler.h>

#include <ui_dialog_cage_3d_deformation.h>

namespace schnapps
{

class SCHNApps;

namespace plugin_cage_3d_deformation
{

class Plugin_Cage3dDeformation;

class SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_API Cage3dDeformation_Dialog : public QDialog, public Ui::Cage3dDeformation_Dialog
{
	Q_OBJECT

public:

	Cage3dDeformation_Dialog(SCHNApps* s, Plugin_Cage3dDeformation* p);

private:

	SCHNApps* schnapps_;
	Plugin_Cage3dDeformation* plugin_;

	CMap2Handler* selected_control_map_;
	MapHandlerGen* selected_deformed_map_;

	bool updating_ui_;

private slots:

	// slots called from UI signals
	void selected_control_map_changed();
	void control_position_attribute_changed(int index);
	void selected_deformed_map_changed();
	void deformed_position_attribute_changed(int index);
	void toggle_control();

	// slots called from SCHNApps signals
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);

	// slots called from MapHandlerGen signals
	void selected_control_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
	void selected_deformed_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);

public:

	CMap2Handler* selected_control_map() const { return selected_control_map_; }

	// methods used to update the UI from the plugin
	void set_control_position_attribute(const QString& name);
	void set_selected_deformed_map(MapHandlerGen* deformed_map);
	void set_deformed_position_attribute(const QString& name);
	void set_linked(bool state);

private:

	// internal UI cascading updates
	void update_after_selected_control_map_changed();
	void update_after_selected_deformed_map_changed();
};

} // namespace plugin_cage_3d_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_DIALOG_H_
