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

#include <dialog_subdivision.h>

#include <surface_modelisation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

namespace plugin_surface_modelisation
{

Subdivision_Dialog::Subdivision_Dialog(SCHNApps* s, Plugin_SurfaceModelisation* p) :
	schnapps_(s),
	plugin_(p),
	selected_map_(nullptr)
{
	setupUi(this);

	if (plugin_->get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = plugin_->get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = plugin_->add_setting("Auto load position attribute", "position").toString();

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(button_loop, SIGNAL(clicked()), this, SLOT(subdivide_loop()));
	connect(button_catmull_clark, SIGNAL(clicked()), this, SLOT(subdivide_catmull_clark()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });
}

void Subdivision_Dialog::subdivide_loop()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		plugin_->subdivide_loop(map_name, position_name);
	}
}

void Subdivision_Dialog::subdivide_catmull_clark()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		plugin_->subdivide_catmull_clark(map_name, position_name);
	}
}

void Subdivision_Dialog::selected_map_changed()
{
	if (selected_map_)
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		combo_positionAttribute->clear();

		const QString& map_name = currentItems[0]->text();
		MapHandlerGen* mhg = schnapps_->get_map(map_name);
		selected_map_ = dynamic_cast<CMap2Handler*>(mhg);

		if (selected_map_)
		{
			const CMap2* map2 = selected_map_->get_map();
			if (map2->is_embedded<CMap2::Vertex::ORBIT>())
			{
				QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
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
					}
				}
			}
			connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		}
	}
	else
		selected_map_ = nullptr;
}

void Subdivision_Dialog::map_added(MapHandlerGen* map)
{
	if (map->dimension() == 2)
	{
		QListWidgetItem* item = new QListWidgetItem(map->get_name(), list_maps);
		item->setCheckState(Qt::Unchecked);
	}
}

void Subdivision_Dialog::map_removed(MapHandlerGen* map)
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

void Subdivision_Dialog::selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map2 = selected_map_->get_map();
		const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			combo_positionAttribute->addItem(attribute_name);
		}
	}
}

} // namespace plugin_surface_modelisation

} // namespace schnapps
