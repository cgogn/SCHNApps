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

#ifndef SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_H_
#define SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_H_

#include <schnapps/plugins/cage_3d_deformation/dll.h>
#include <schnapps/plugins/cage_3d_deformation/map_parameters.h>

#include <schnapps/core/plugin_processing.h>

namespace schnapps
{

namespace plugin_cage_3d_deformation
{

class Cage3dDeformation_Dialog;

/**
* @brief Cage 3d deformation
*/
class SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_API Plugin_Cage3dDeformation : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_Cage3dDeformation();
	~Plugin_Cage3dDeformation() override {}
	static QString plugin_name();

	MapParameters& get_parameters(MapHandlerGen* map);

private:

	bool enable() override;
	void disable() override;

private slots:

	// slots called from SCHNApps signals
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);
	void schnapps_closing();

	// slots called from MapHandler signals
	void attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
	void attribute_changed(cgogn::Orbit orbit, const QString& attribute_name);
	void connectivity_changed();

	// slots called from action signals
	void open_dialog();

public slots:

	void set_deformed_position_attribute(MapHandlerGen* map, const QString& name, bool update_dialog);
	void set_control_map(MapHandlerGen* map, CMap2Handler* control, bool update_dialog);
	void set_control_position_attribute(MapHandlerGen* map, const QString& name, bool update_dialog);
	void toggle_control(MapHandlerGen* map, bool update_dialog);

private:

	Cage3dDeformation_Dialog* cage_3d_deformation_dialog_;
	QAction* setup_cage3d_deformation_action;

	QString setting_auto_load_control_position_attribute_;
	QString setting_auto_load_deformed_position_attribute_;

	std::map<MapHandlerGen*, MapParameters> parameter_set_;
};

} // namespace plugin_cage_3d_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_H_
