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

#include <schnapps/core/types.h>

#include <ui_dialog_cage_3d_deformation.h>

namespace cgogn { enum Orbit: numerics::uint32; }

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap2Handler;
}

class SCHNApps;
class Object;

namespace plugin_cage_3d_deformation
{

class Plugin_Cage3dDeformation;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

class SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_API Cage3dDeformation_Dialog : public QDialog, public Ui::Cage3dDeformation_Dialog
{
	Q_OBJECT

public:

	Cage3dDeformation_Dialog(SCHNApps* s, Plugin_Cage3dDeformation* p);
	~Cage3dDeformation_Dialog() override;

private:

	SCHNApps* schnapps_;
	Plugin_Cage3dDeformation* plugin_;

	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;

	CMap2Handler* selected_deformed_map_;
	CMap2Handler* selected_control_map_;

	bool updating_ui_;

private slots:

	// slots called from UI signals
	void selected_deformed_map_changed();
	void deformed_position_attribute_changed(int index);
	void selected_control_map_changed();
	void control_position_attribute_changed(int index);
	void toggle_control();

	// slots called from SCHNApps signals
	void object_added(Object* o);
	void object_removed(Object* o);

	// slots called from CMap2Handler signals
	void selected_deformed_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
	void selected_deformed_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name);
	void selected_control_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
	void selected_control_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name);

public:

	CMap2Handler* selected_control_map() const { return selected_control_map_; }
	CMap2Handler* selected_deformed_map() const { return selected_deformed_map_; }

	// methods used to update the UI from the plugin
	void set_deformed_position_attribute(const QString& name);
	void set_selected_control_map(CMap2Handler* control_map);
	void set_control_position_attribute(const QString& name);
	void set_linked(bool state);

private:

	void map_added(CMap2Handler* mh);
	void map_removed(CMap2Handler* mh);

	// internal UI cascading updates
	void update_after_selected_deformed_map_changed();
	void update_after_selected_control_map_changed();
};

} // namespace plugin_cage_3d_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_DIALOG_H_
