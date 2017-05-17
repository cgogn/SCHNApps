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

#include <dialog_compute_normal.h>

#include <surface_differential_properties.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace plugin_sdp
{

ComputeNormal_Dialog::ComputeNormal_Dialog(SCHNApps* s, Plugin_SurfaceDifferentialProperties* p) :
	schnapps_(s),
	plugin_(p),
	selected_map_(nullptr)
{
	setupUi(this);

	if (plugin_->get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = plugin_->get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = plugin_->add_setting("Auto load position attribute", "position").toString();

	if (plugin_->get_setting("Auto load normal attribute").isValid())
		setting_auto_load_normal_attribute_ = plugin_->get_setting("Auto load normal attribute").toString();
	else
		setting_auto_load_normal_attribute_ = plugin_->add_setting("Auto load normal attribute", "normal").toString();

	normal_attribute_name->setText(setting_auto_load_normal_attribute_);
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(this, SIGNAL(accepted()), this, SLOT(compute_normal()));
	connect(button_apply, SIGNAL(clicked()), this, SLOT(compute_normal()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });
}

void ComputeNormal_Dialog::compute_normal()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		QString position_name = combo_positionAttribute->currentText();

		QString normal_name;
		if (normal_attribute_name->text().isEmpty())
			normal_name = combo_normalAttribute->currentText();
		else
			normal_name = normal_attribute_name->text();

		bool create_vbo = enableVBO->isChecked();
		bool auto_update = currentItems[0]->checkState() == Qt::Checked;

		plugin_->compute_normal(map_name, position_name, normal_name, create_vbo, auto_update);
	}
}

void ComputeNormal_Dialog::selected_map_changed()
{
	if (selected_map_)
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		combo_positionAttribute->clear();
		combo_normalAttribute->clear();

		const QString& map_name = currentItems[0]->text();
		MapHandlerGen* mhg = schnapps_->get_map(map_name);
		selected_map_ = dynamic_cast<CMap2Handler*>(mhg);

		if (selected_map_)
		{
			const CMap2* map2 = selected_map_->get_map();
			if (map2->is_embedded<CMap2::Vertex::ORBIT>())
			{
				QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.names();
				const std::vector<std::string>& type_names = container.type_names();

				for (std::size_t i = 0u; i < names.size(); ++i)
				{
					QString name = QString::fromStdString(names[i]);
					QString type = QString::fromStdString(type_names[i]);
					if (type == vec3_type_name)
					{
						combo_positionAttribute->addItem(name);
						if (name == setting_auto_load_position_attribute_)
							combo_positionAttribute->setCurrentIndex(combo_positionAttribute->count() - 1);
						combo_normalAttribute->addItem(name);
						if (name == setting_auto_load_normal_attribute_)
							combo_normalAttribute->setCurrentIndex(combo_normalAttribute->count() - 1);
					}
				}
			}
			connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		}
	}
	else
		selected_map_ = nullptr;
}

void ComputeNormal_Dialog::map_added(MapHandlerGen* map)
{
	if (map->dimension() == 2)
	{
		QListWidgetItem* item = new QListWidgetItem(map->get_name(), list_maps);
		item->setCheckState(Qt::Unchecked);
	}
}

void ComputeNormal_Dialog::map_removed(MapHandlerGen* map)
{
	QList<QListWidgetItem*> items = list_maps->findItems(map->get_name(), Qt::MatchExactly);
	if (!items.empty())
		delete items[0];

	if (selected_map_ == map)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		selected_map_ = nullptr;
	}
}

void ComputeNormal_Dialog::selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map2 = selected_map_->get_map();
		const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			combo_positionAttribute->addItem(attribute_name);
			combo_normalAttribute->addItem(attribute_name);
		}
	}
}

} // namespace plugin_sdp

} // namespace schnapps
