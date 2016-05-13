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

ComputeNormal_Dialog::ComputeNormal_Dialog(SCHNApps* s) :
	schnapps_(s),
	selected_map_(nullptr)
{
	setupUi(this);

	normal_attribute_name->setText("normal");

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(add_map_to_list(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(remove_map_from_list(MapHandlerGen*)));

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	schnapps_->foreach_map([this] (MapHandlerGen* map)
	{
		QListWidgetItem* item = new QListWidgetItem(map->get_name(), list_maps);
		item->setCheckState(Qt::Unchecked);
	});
}

void ComputeNormal_Dialog::selected_map_changed()
{
//	if (selected_map_)
//		disconnect(selected_map_, SIGNAL(attribute_added(unsigned int, const QString&)), this, SLOT(add_attribute_to_list(unsigned int, const QString&)));

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		combo_positionAttribute->clear();
		combo_normalAttribute->clear();

		const QString& map_name = currentItems[0]->text();
		selected_map_ = schnapps_->get_map(map_name);

		const MapBaseData* map = selected_map_->get_map();
		const CMap2* map2 = dynamic_cast<const CMap2*>(map);

		if (map2 && map2->is_embedded<CMap2::Vertex::ORBIT>())
		{
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

			const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->get_attribute_container<CMap2::Vertex::ORBIT>();
			const std::vector<std::string>& names = container.get_names();
			const std::vector<std::string>& type_names = container.get_type_names();

			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
				QString type = QString::fromStdString(type_names[i]);
				if (type == vec3_type_name)
				{
					combo_positionAttribute->addItem(name);
					combo_normalAttribute->addItem(name);
				}
			}
		}

//		connect(selected_map_, SIGNAL(attribute_added(unsigned int, const QString&)), this, SLOT(add_attribute_to_list(unsigned int, const QString&)));
	}
	else
		selected_map_ = nullptr;
}

void ComputeNormal_Dialog::add_map_to_list(MapHandlerGen* map)
{
	QListWidgetItem* item = new QListWidgetItem(map->get_name(), list_maps);
	item->setCheckState(Qt::Unchecked);
}

void ComputeNormal_Dialog::remove_map_from_list(MapHandlerGen* map)
{
	QList<QListWidgetItem*> items = list_maps->findItems(map->get_name(), Qt::MatchExactly);
	if (!items.empty())
		delete items[0];

	if (selected_map_ == map)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(unsigned int, const QString&)), this, SLOT(add_attribute_to_list(unsigned int, const QString&)));
		selected_map_ = nullptr;
	}
}

//void ComputeNormal_Dialog::add_attribute_to_list(unsigned int orbit, const QString& attr_name)
//{
//	if (orbit == CMap2::Vertex::ORBIT)
//	{
//		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
//		const QString& attr_type_name = selected_map_->getAttributeTypeName(orbit, attr_name);
//		if (attr_type_name == vec3_type_name)
//		{
//			combo_positionAttribute->addItem(attr_name);
//			combo_normalAttribute->addItem(attr_name);
//		}
//	}
//}

} // namespace schnapps
