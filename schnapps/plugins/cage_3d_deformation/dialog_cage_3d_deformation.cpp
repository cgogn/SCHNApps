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

#include <schnapps/plugins/cage_3d_deformation/dialog_cage_3d_deformation.h>
#include <schnapps/plugins/cage_3d_deformation/cage_3d_deformation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace plugin_cage_3d_deformation
{

Cage3dDeformation_Dialog::Cage3dDeformation_Dialog(SCHNApps* s, Plugin_Cage3dDeformation* p) :
	schnapps_(s),
	plugin_(p),
	selected_control_map_(nullptr),
	selected_deformed_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	list_deformed_maps->setEnabled(false);
	combo_deformedPositionAttribute->setEnabled(false);
	button_linkUnlink->setEnabled(false);

	connect(list_control_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_control_map_changed()));
	connect(combo_controlPositionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(control_position_attribute_changed(int)));
	connect(list_deformed_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_deformed_map_changed()));
	connect(combo_deformedPositionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(deformed_position_attribute_changed(int)));

	connect(button_linkUnlink, SIGNAL(clicked()), this, SLOT(toggle_control()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void Cage3dDeformation_Dialog::selected_control_map_changed()
{
	if (selected_control_map_)
		disconnect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));

	QList<QListWidgetItem*> currentItems = list_control_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		MapHandlerGen* mhg = schnapps_->get_map(map_name);

		selected_control_map_ = dynamic_cast<CMap2Handler*>(mhg);

		if (selected_control_map_)
			connect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));

		update_after_selected_control_map_changed();
	}
	else
	{
		selected_control_map_ = nullptr;
		list_deformed_maps->setEnabled(false);
		combo_deformedPositionAttribute->setEnabled(false);
		button_linkUnlink->setEnabled(false);
	}
}

void Cage3dDeformation_Dialog::control_position_attribute_changed(int index)
{
	if (!updating_ui_)
		plugin_->set_control_position_attribute(selected_control_map_, combo_controlPositionAttribute->currentText(), false);
}

void Cage3dDeformation_Dialog::selected_deformed_map_changed()
{
	if (selected_deformed_map_)
		disconnect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));

	QList<QListWidgetItem*> currentItems = list_deformed_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		selected_deformed_map_ = schnapps_->get_map(map_name);
		if (!updating_ui_)
			plugin_->set_deformed_map(selected_control_map_, selected_deformed_map_, false);

		if (selected_deformed_map_)
			connect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));

		update_after_selected_deformed_map_changed();
	}
}

void Cage3dDeformation_Dialog::deformed_position_attribute_changed(int index)
{
	if (!updating_ui_)
		plugin_->set_deformed_position_attribute(selected_control_map_, combo_deformedPositionAttribute->currentText(), false);
}

void Cage3dDeformation_Dialog::toggle_control()
{
	plugin_->toggle_control(selected_control_map_, true);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void Cage3dDeformation_Dialog::map_added(MapHandlerGen* map)
{
	if (map->dimension() == 2)
		list_control_maps->addItem(map->get_name());

	list_deformed_maps->addItem(map->get_name());
}

void Cage3dDeformation_Dialog::map_removed(MapHandlerGen* map)
{
	if (selected_control_map_ == map)
	{
		disconnect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));
		selected_control_map_ = nullptr;
	}

	QList<QListWidgetItem*> control_items = list_control_maps->findItems(map->get_name(), Qt::MatchExactly);
	if (!control_items.empty())
		delete control_items[0];

	if (selected_deformed_map_ == map)
	{
		disconnect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));
		selected_deformed_map_ = nullptr;
	}

	QList<QListWidgetItem*> deformed_items = list_deformed_maps->findItems(map->get_name(), Qt::MatchExactly);
	if (!deformed_items.empty())
		delete deformed_items[0];
}

/*****************************************************************************/
// slots called from MapHandlerGen signals
/*****************************************************************************/

void Cage3dDeformation_Dialog::selected_control_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map2 = selected_control_map_->get_map();
		const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
			combo_controlPositionAttribute->addItem(attribute_name);
	}
}

void Cage3dDeformation_Dialog::selected_deformed_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == selected_deformed_map_->orbit(CellType::Vertex_Cell))
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const MapHandlerGen::ChunkArrayContainer<uint32>* container = selected_deformed_map_->attribute_container(CellType::Vertex_Cell);
		QString attribute_type_name = QString::fromStdString(container->get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
			combo_deformedPositionAttribute->addItem(attribute_name);
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void Cage3dDeformation_Dialog::set_control_position_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_controlPositionAttribute->findText(name);
	if (index > 0)
		combo_controlPositionAttribute->setCurrentIndex(index);
	else
		combo_controlPositionAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void Cage3dDeformation_Dialog::set_selected_deformed_map(MapHandlerGen* map)
{
	if (selected_deformed_map_)
		disconnect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));

	selected_deformed_map_ = map;

	if (selected_deformed_map_)
		connect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));

	updating_ui_ = true;
	QList<QListWidgetItem*> foundItems = list_deformed_maps->findItems(selected_deformed_map_->get_name(), Qt::MatchExactly);
	if (!foundItems.empty())
		list_deformed_maps->setCurrentItem(foundItems[0]);
	updating_ui_ = false;

	update_after_selected_deformed_map_changed();
}

void Cage3dDeformation_Dialog::set_deformed_position_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_deformedPositionAttribute->findText(name);
	if (index > 0)
		combo_deformedPositionAttribute->setCurrentIndex(index);
	else
		combo_deformedPositionAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void Cage3dDeformation_Dialog::set_linked(bool state)
{
	updating_ui_ = true;
	list_deformed_maps->setEnabled(!state);
	combo_deformedPositionAttribute->setEnabled(!state);
	button_linkUnlink->setText(state ? "Unlink" : "Link");
	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void Cage3dDeformation_Dialog::update_after_selected_control_map_changed()
{
	updating_ui_ = true;
	combo_controlPositionAttribute->clear();
	combo_controlPositionAttribute->addItem("- select attribute -");

	if (selected_control_map_)
	{
		const CMap2* map2 = selected_control_map_->get_map();
		if (map2->is_embedded<CMap2::Vertex::ORBIT>())
		{
			const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
			const std::vector<std::string>& names = container.names();
			const std::vector<std::string>& type_names = container.type_names();
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

			const MapParameters& p = plugin_->get_parameters(selected_control_map_);

			unsigned int i = 1;
			for (std::size_t j = 0u; j < names.size(); ++j)
			{
				QString name = QString::fromStdString(names[j]);
				QString type = QString::fromStdString(type_names[j]);
				if (type == vec3_type_name)
				{
					combo_controlPositionAttribute->addItem(name);
					const CMap2::VertexAttribute<VEC3>& pos = p.get_control_position_attribute();
					if (pos.is_valid() && QString::fromStdString(pos.name()) == name)
						combo_controlPositionAttribute->setCurrentIndex(i);

					++i;
				}
			}

			if (!p.get_linked())
			{
				list_deformed_maps->setEnabled(true);
				combo_deformedPositionAttribute->setEnabled(true);
				button_linkUnlink->setEnabled(true);
				button_linkUnlink->setText("Link");
			}
			else
			{
				list_deformed_maps->setEnabled(false);
				combo_deformedPositionAttribute->setEnabled(false);
				button_linkUnlink->setEnabled(true);
				button_linkUnlink->setText("Unlink");
			}

			MapHandlerGen* deformed_map = p.get_deformed_map();
			if (deformed_map)
			{
				QList<QListWidgetItem*> items = list_deformed_maps->findItems(deformed_map->get_name(), Qt::MatchExactly);
				if (!items.empty())
					list_deformed_maps->setCurrentItem(items[0]);
			}
			else
			{
				list_deformed_maps->setCurrentItem(nullptr);
				if (selected_deformed_map_)
					disconnect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));
				selected_deformed_map_ = nullptr;
				update_after_selected_deformed_map_changed();
			}
		}
	}
	updating_ui_ = false;
}

void Cage3dDeformation_Dialog::update_after_selected_deformed_map_changed()
{
	updating_ui_ = true;
	combo_deformedPositionAttribute->clear();
	combo_deformedPositionAttribute->addItem("- select attribute -");

	if (selected_control_map_ && selected_deformed_map_)
	{
		if (selected_deformed_map_->is_embedded(CellType::Vertex_Cell))
		{
			const MapHandlerGen::ChunkArrayContainer<uint32>* container = selected_deformed_map_->attribute_container(CellType::Vertex_Cell);
			const std::vector<std::string>& names = container->names();
			const std::vector<std::string>& type_names = container->type_names();
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

			const MapParameters& p = plugin_->get_parameters(selected_control_map_);

			unsigned int i = 1;
			for (std::size_t j = 0u; j < names.size(); ++j)
			{
				QString name = QString::fromStdString(names[j]);
				QString type = QString::fromStdString(type_names[j]);
				if (type == vec3_type_name)
				{
					combo_deformedPositionAttribute->addItem(name);
					const MapHandlerGen::Attribute_T<VEC3>& pos = p.get_deformed_position_attribute();
					if (pos.is_valid() && QString::fromStdString(pos.name()) == name)
						combo_deformedPositionAttribute->setCurrentIndex(i);

					++i;
				}
			}
		}
	}
	updating_ui_ = false;
}

} // namespace plugin_cage_3d_deformation

} // namespace schnapps
