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

MapParameters& Plugin_Cage3dDeformation::parameters(CMap2Handler* mh)
{
	cgogn_message_assert(mh, "Try to access parameters for null map");

	if (parameter_set_.count(mh) == 0)
	{
		MapParameters& p = parameter_set_[mh];
		p.deformed_map_ = mh;
		return p;
	}
	else
		return parameter_set_[mh];
}

bool Plugin_Cage3dDeformation::enable()
{
	if (setting("Auto load deformed position attribute").isValid())
		setting_auto_load_deformed_position_attribute_ = setting("Auto load deformed position attribute").toString();
	else
		setting_auto_load_deformed_position_attribute_ = add_setting("Auto load deformedposition attribute", "position").toString();

	if (setting("Auto load control position attribute").isValid())
		setting_auto_load_control_position_attribute_ = setting("Auto load control position attribute").toString();
	else
		setting_auto_load_control_position_attribute_ = add_setting("Auto load control position attribute", "position").toString();

	cage_3d_deformation_dialog_ = new Cage3dDeformation_Dialog(this->schnapps_, this);

	setup_cage3d_deformation_action = schnapps_->add_menu_action("Deformation;Cage 3d", "setup 3d cage deformation");
	connect(setup_cage3d_deformation_action, SIGNAL(triggered()), this, SLOT(open_dialog()));

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_object([this] (Object* o)
	{
		CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
		if (mh)
			map_added(mh);
	});

	plugin_cmap2_provider_ = static_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));

	return true;
}

void Plugin_Cage3dDeformation::disable()
{
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	disconnect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	disconnect(setup_cage3d_deformation_action, SIGNAL(triggered()), this, SLOT(open_dialog()));

	schnapps_->remove_menu_action(setup_cage3d_deformation_action);

	delete cage_3d_deformation_dialog_;
}

void Plugin_Cage3dDeformation::object_added(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_added(mh);
}

void Plugin_Cage3dDeformation::map_added(CMap2Handler* mh)
{
	connect(mh, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
	connect(mh, SIGNAL(connectivity_changed()), this, SLOT(connectivity_changed()));
	connect(mh, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(attribute_added(cgogn::Orbit, const QString&)));
}

void Plugin_Cage3dDeformation::object_removed(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_removed(mh);
}

void Plugin_Cage3dDeformation::map_removed(CMap2Handler* mh)
{
	disconnect(mh, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
	disconnect(mh, SIGNAL(connectivity_changed()), this, SLOT(connectivity_changed()));
	disconnect(mh, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(attribute_added(cgogn::Orbit, const QString&)));
}

void Plugin_Cage3dDeformation::schnapps_closing()
{
	cage_3d_deformation_dialog_->close();
}

void Plugin_Cage3dDeformation::attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		CMap2Handler* mh = static_cast<CMap2Handler*>(sender());
		for (auto& it : parameter_set_)
		{
			MapParameters& p = it.second;
			if (it.first == mh)
			{
				if (!p.deformed_position_.is_valid() && attribute_name == setting_auto_load_deformed_position_attribute_)
					set_deformed_position_attribute(mh, attribute_name, true);
			}
			else
			{
				if (p.control_map_ == mh && !p.control_position_.is_valid() && attribute_name == setting_auto_load_control_position_attribute_)
					set_control_position_attribute(mh, attribute_name, true);
			}
		}
	}
}

void Plugin_Cage3dDeformation::attribute_changed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		CMap2Handler* mh = static_cast<CMap2Handler*>(sender());
		for (auto& it : parameter_set_)
		{
			MapParameters& p = it.second;
			if (p.updating_)
				return;
			if (it.first == mh)
			{
				if (p.linked_ && p.deformed_position_.is_valid() && QString::fromStdString(p.deformed_position_.name()) == attribute_name)
					p.update_control_map();
			}
			else
			{
				if (p.linked_ && p.control_map_ == mh && p.control_position_.is_valid() && QString::fromStdString(p.control_position_.name()) == attribute_name)
					p.update_deformed_map();
			}
		}
	}
}

void Plugin_Cage3dDeformation::connectivity_changed()
{
	CMap2Handler* mh = static_cast<CMap2Handler*>(sender());
	for (auto& it : parameter_set_)
	{
		MapParameters& p = it.second;
		if (p.linked_ && (it.first == mh || p.control_map_ == mh))
		{
			p.toggle_control();
			p.toggle_control();
			p.update_deformed_map();
		}
	}
}

void Plugin_Cage3dDeformation::open_dialog()
{
	cage_3d_deformation_dialog_->show();
}

/******************************************************************************/
/*                             PUBLIC INTERFACE                               */
/******************************************************************************/

void Plugin_Cage3dDeformation::set_deformed_position_attribute(CMap2Handler* mh, const QString& name, bool update_dialog)
{
	MapParameters& p = parameters(mh);
	p.set_deformed_position_attribute(name);
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == mh)
		cage_3d_deformation_dialog_->set_deformed_position_attribute(name);
}

void Plugin_Cage3dDeformation::set_control_map(CMap2Handler* mh, CMap2Handler* control, bool update_dialog)
{
	MapParameters& p = parameters(mh);
	p.set_control_map(control);
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == mh)
		cage_3d_deformation_dialog_->set_selected_control_map(control);
}

void Plugin_Cage3dDeformation::set_control_position_attribute(CMap2Handler* mh, const QString& name, bool update_dialog)
{
	MapParameters& p = parameters(mh);
	p.set_control_position_attribute(name);
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == mh)
		cage_3d_deformation_dialog_->set_control_position_attribute(name);
}

void Plugin_Cage3dDeformation::toggle_control(CMap2Handler* mh, bool update_dialog)
{
	MapParameters& p = parameters(mh);
	bool state = p.toggle_control();
	if (update_dialog && cage_3d_deformation_dialog_->selected_deformed_map() == mh)
		cage_3d_deformation_dialog_->set_linked(state);
}

} // namespace plugin_cage_3d_deformation

} // namespace schnapps
