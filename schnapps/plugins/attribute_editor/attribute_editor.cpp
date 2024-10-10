/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Attribute Editor                                                      *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#include <schnapps/plugins/attribute_editor/attribute_editor.h>
#include <schnapps/plugins/attribute_editor/add_attribute_dialog.h>
#include <schnapps/plugins/attribute_editor/edit_attribute_dialog.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>

#include <QAction>

namespace schnapps
{

namespace plugin_attribute_editor
{

AttributeEditorPlugin::AttributeEditorPlugin() :
	add_attribute_action_(nullptr),
	add_attribute_dialog_(nullptr),
	edit_attribute_dialog_(nullptr),
	plugin_cmap_provider_(nullptr)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString AttributeEditorPlugin::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

const plugin_cmap_provider::Plugin_CMapProvider *AttributeEditorPlugin::cmap_provider() const
{
	return plugin_cmap_provider_;
}

bool AttributeEditorPlugin::enable()
{
	plugin_cmap_provider_  = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));

	add_attribute_dialog_ = new AddAttributeDialog(schnapps_, this);
	edit_attribute_dialog_ = new EditAttributeDialog(schnapps_, this);

	add_attribute_action_ = schnapps_->add_menu_action("Attribute;Add attribute", "Add attribute");
	edit_attribute_action_ = schnapps_->add_menu_action("Attribute;Edit attribute", "Edit attribute");

	connect(add_attribute_action_, SIGNAL(triggered()), this, SLOT(add_attribute_dialog()));
	connect(edit_attribute_action_, SIGNAL(triggered()), this, SLOT(edit_attribute_dialog()));

	return true;
}

void AttributeEditorPlugin::disable()
{
	disconnect(add_attribute_action_, SIGNAL(triggered()), this, SLOT(add_attribute_dialog()));
	disconnect(edit_attribute_action_, SIGNAL(triggered()), this, SLOT(edit_attribute_dialog()));

	schnapps_->remove_menu_action(add_attribute_action_);
	schnapps_->remove_menu_action(edit_attribute_action_);

	delete add_attribute_dialog_;
	delete edit_attribute_dialog_;
}

void AttributeEditorPlugin::add_attribute_dialog()
{
	add_attribute_dialog_->show();
}

void AttributeEditorPlugin::edit_attribute_dialog()
{
	edit_attribute_dialog_->show();
}

cgogn::Orbit orbit_from_string(const QString& str)
{
	static std::map<QString, cgogn::Orbit> m {
	{QString::fromStdString("PHI1"), cgogn::Orbit::PHI1},
	{QString::fromStdString("PHI2"), cgogn::Orbit::PHI2},
	{QString::fromStdString("PHI21"), cgogn::Orbit::PHI21},
	{QString::fromStdString("PHI1_PHI2"), cgogn::Orbit::PHI1_PHI2},
	{QString::fromStdString("PHI1_PHI3"), cgogn::Orbit::PHI1_PHI3},
	{QString::fromStdString("PHI2_PHI3"), cgogn::Orbit::PHI2_PHI3},
	{QString::fromStdString("PHI21_PHI31"), cgogn::Orbit::PHI21_PHI31},
	{QString::fromStdString("PHI1_PHI2_PHI3"), cgogn::Orbit::PHI1_PHI2_PHI3}
	};
	return m[str];

}

QStringList get_attribute_names(const plugin_cmap_provider::CMapHandlerGen* mhg, cgogn::Orbit orb)
{
	QStringList res;
	const auto& cont = mhg->map()->attribute_container(orb);

	const auto& names = cont.names();
	res.reserve(int32(names.size()));
	for (const auto& name : names)
		res.push_back(QString::fromStdString(name));
	return res;
}

const std::vector<cgogn::Orbit> available_orbits(const plugin_cmap_provider::CMapHandlerGen* mhg)
{
	static const std::map<plugin_cmap_provider::CMapType, std::vector<cgogn::Orbit>> m =
	{
		{plugin_cmap_provider::CMapType::CMAP0, {cgogn::Orbit::DART}},
		{plugin_cmap_provider::CMapType::CMAP1, {cgogn::Orbit::DART, cgogn::Orbit::PHI1}},
		{plugin_cmap_provider::CMapType::CMAP2, {cgogn::Orbit::DART, cgogn::Orbit::PHI1, cgogn::Orbit::PHI2, cgogn::Orbit::PHI21, cgogn::Orbit::PHI1_PHI2}},
		{plugin_cmap_provider::CMapType::CMAP3, {cgogn::Orbit::DART, cgogn::Orbit::PHI1, cgogn::Orbit::PHI2, cgogn::Orbit::PHI21, cgogn::Orbit::PHI1_PHI2, cgogn::Orbit::PHI1_PHI3, cgogn::Orbit::PHI2_PHI3, cgogn::Orbit::PHI21_PHI31, cgogn::Orbit::PHI1_PHI2_PHI3}},
		{plugin_cmap_provider::CMapType::UNDIRECTED_GRAPH, {cgogn::Orbit::DART, cgogn::Orbit::PHI1, cgogn::Orbit::PHI2, cgogn::Orbit::PHI21}}
	};


	const auto it = m.find(mhg->type());
	if (it != m.end())
		return it->second;
	else
		return std::vector<cgogn::Orbit>();
}

} // namespace plugin_attribute_editor

} // namespace schnapps
