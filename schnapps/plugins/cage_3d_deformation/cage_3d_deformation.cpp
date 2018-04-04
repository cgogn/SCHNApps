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

#include <schnapps/plugins/cage_3d_deformation/cage_3d_deformation.h>
#include <schnapps/plugins/cage_3d_deformation/dialog_cage_3d_deformation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

namespace schnapps
{

namespace plugin_cage_3d_deformation
{

Plugin_Cage3dDeformation::Plugin_Cage3dDeformation()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_Cage3dDeformation::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

MapParameters& Plugin_Cage3dDeformation::get_parameters(MapHandlerGen* map)
{
	cgogn_message_assert(map, "Try to access parameters for null map");

	if (parameter_set_.count(map) == 0)
	{
		MapParameters& p = parameter_set_[map];
		p.deformed_map_ = map;
		return p;
	}
	else
		return parameter_set_[map];
}

bool Plugin_Cage3dDeformation::enable()
{
	if (get_setting("Auto load deformed position attribute").isValid())
		setting_auto_load_deformed_position_attribute_ = get_setting("Auto load deformed position attribute").toString();
	else
		setting_auto_load_deformed_position_attribute_ = add_setting("Auto load deformedposition attribute", "position").toString();

	if (get_setting("Auto load control position attribute").isValid())
		setting_auto_load_control_position_attribute_ = get_setting("Auto load control position attribute").toString();
	else
		setting_auto_load_control_position_attribute_ = add_setting("Auto load control position attribute", "position").toString();

	cage_3d_deformation_dialog_ = new Cage3dDeformation_Dialog(this->schnapps_, this);

	setup_cage3d_deformation_action = schnapps_->add_menu_action("Deformation;Cage 3d", "setup 3d cage deformation");
	connect(setup_cage3d_deformation_action, SIGNAL(triggered()), this, SLOT(open_dialog()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });

	return true;
}

void Plugin_Cage3dDeformation::disable()
{
	disconnect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(setup_cage3d_deformation_action, SIGNAL(triggered()), this, SLOT(open_dialog()));
	schnapps_->remove_menu_action(setup_cage3d_deformation_action);

	delete cage_3d_deformation_dialog_;
}

void Plugin_Cage3dDeformation::map_added(MapHandlerGen *map)
{
	if (map->dimension() == 2)
		connect(map, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
	connect(map, SIGNAL(connectivity_changed()), this, SLOT(connectivity_changed()));
	connect(map, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(attribute_added(cgogn::Orbit, const QString&)));
}

void Plugin_Cage3dDeformation::map_removed(MapHandlerGen *map)
{
	if (map->dimension() == 2)
		disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
	disconnect(map, SIGNAL(connectivity_changed()), this, SLOT(connectivity_changed()));
	disconnect(map, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(attribute_added(cgogn::Orbit, const QString&)));
}

void Plugin_Cage3dDeformation::attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		for (auto& it : parameter_set_)
		{
			MapParameters& p = it.second;
			if (it.first == map)
			{
				if (!p.get_deformed_position_attribute().is_valid() && attribute_name == setting_auto_load_deformed_position_attribute_)
					set_deformed_position_attribute(map, attribute_name, true);
			}
			else
			{
				if (p.control_map_ == map && !p.get_control_position_attribute().is_valid() && attribute_name == setting_auto_load_control_position_attribute_)
					set_control_position_attribute(map, attribute_name, true);
			}
		}
	}
}

void Plugin_Cage3dDeformation::attribute_changed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		CMap2Handler* map = static_cast<CMap2Handler*>(sender());
		for (auto& it : parameter_set_)
		{
			MapParameters& p = it.second;
			if (p.get_linked() && p.get_control_map() == map && p.get_control_position_attribute_name() == attribute_name)
				p.update_deformed_map();
		}
	}
}

void Plugin_Cage3dDeformation::connectivity_changed()
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	for (auto& it : parameter_set_)
	{
		MapParameters& p = it.second;
		if (p.get_linked() && (it.first == map || p.control_map_ == map))
		{
			p.toggle_control();
			p.toggle_control();
			p.update_deformed_map();
		}
	}
}

void Plugin_Cage3dDeformation::schnapps_closing()
{
	cage_3d_deformation_dialog_->close();
}

void Plugin_Cage3dDeformation::open_dialog()
{
	cage_3d_deformation_dialog_->show();
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_Cage3dDeformation::set_deformed_position_attribute(MapHandlerGen* map, const QString& name, bool update_dialog)
{
	MapParameters& p = get_parameters(map);
	p.set_deformed_position_attribute(name);
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == map)
		cage_3d_deformation_dialog_->set_deformed_position_attribute(name);
}

void Plugin_Cage3dDeformation::set_control_map(MapHandlerGen* map, CMap2Handler* control, bool update_dialog)
{
	MapParameters& p = get_parameters(map);
	p.set_control_map(control);
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == map)
		cage_3d_deformation_dialog_->set_selected_control_map(control);
}

void Plugin_Cage3dDeformation::set_control_position_attribute(MapHandlerGen* map, const QString& name, bool update_dialog)
{
	MapParameters& p = get_parameters(map);
	p.set_control_position_attribute(name);
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == map)
		cage_3d_deformation_dialog_->set_control_position_attribute(name);
}

void Plugin_Cage3dDeformation::toggle_control(MapHandlerGen *map, bool update_dialog)
{
	MapParameters& p = get_parameters(map);
	bool state = p.toggle_control();
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == map)
		cage_3d_deformation_dialog_->set_linked(state);
}

} // namespace plugin_cage_3d_deformation

} // namespace schnapps
