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

#include <QAction>

namespace cgogn { enum Orbit: numerics::uint32; }

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap2Handler;
}

namespace plugin_cage_3d_deformation
{

class Cage3dDeformation_Dialog;

using CMap2Handler = plugin_cmap_provider::CMap2Handler;

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

	MapParameters& parameters(CMap2Handler* mh);

private:

	bool enable() override;
	void disable() override;

private slots:

	// slots called from SCHNApps signals
	void object_added(Object* o);
	void object_removed(Object* o);
	void schnapps_closing();

	// slots called from MapHandler signals
	void attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
	void attribute_changed(cgogn::Orbit orbit, const QString& attribute_name);
	void connectivity_changed();

	// slots called from action signals
	void open_dialog();

private:

	void map_added(CMap2Handler* mh);
	void map_removed(CMap2Handler* mh);

public:

	void set_deformed_position_attribute(CMap2Handler* mh, const QString& name, bool update_dialog);
	void set_control_map(CMap2Handler* mh, CMap2Handler* control, bool update_dialog);
	void set_control_position_attribute(CMap2Handler* mh, const QString& name, bool update_dialog);
	void toggle_control(CMap2Handler* mh, bool update_dialog);

private:

	Cage3dDeformation_Dialog* cage_3d_deformation_dialog_;
	QAction* setup_cage3d_deformation_action;

	QString setting_auto_load_control_position_attribute_;
	QString setting_auto_load_deformed_position_attribute_;

	std::map<CMap2Handler*, MapParameters> parameter_set_;

	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;
};

} // namespace plugin_cage_3d_deformation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CAGE_3D_DEFORMATION_H_
