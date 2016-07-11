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

#include <dialog_compute_curvature.h>

#include <surface_differential_properties.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{
namespace plugin_sdp
{

ComputeCurvature_Dialog::ComputeCurvature_Dialog(SCHNApps* s) :
	schnapps_(s),
	selected_map_(nullptr)
{
	setupUi(this);

	Kmax_attribute_name->setText("Kmax");
	kmax_attribute_name->setText("kmax");
	Kmin_attribute_name->setText("Kmin");
	kmin_attribute_name->setText("kmin");
	Knormal_attribute_name->setText("Knormal");

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });
}

void ComputeCurvature_Dialog::selected_map_changed()
{
	if(selected_map_)
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if(!currentItems.empty())
	{
		combo_positionAttribute->clear();
		combo_normalAttribute->clear();
		combo_KmaxAttribute->clear();
		combo_KminAttribute->clear();
		combo_KnormalAttribute->clear();
		combo_kmaxAttribute->clear();
		combo_kminAttribute->clear();

		const QString& map_name = currentItems[0]->text();
		MapHandlerGen* mhg = schnapps_->get_map(map_name);
		selected_map_ = dynamic_cast<MapHandler<CMap2>*>(mhg);

		if (selected_map_)
		{
			const CMap2* map2 = selected_map_->get_map();
			if (map2->is_embedded<CMap2::Vertex::ORBIT>())
			{
				QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
				QString scalar_type_name = QString::fromStdString(cgogn::name_of_type(SCALAR()));

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
						combo_normalAttribute->addItem(name);
						combo_KmaxAttribute->addItem(name);
						combo_KminAttribute->addItem(name);
						combo_KnormalAttribute->addItem(name);
					}
					else if (type == scalar_type_name)
					{
						combo_kmaxAttribute->addItem(name);
						combo_kminAttribute->addItem(name);
					}
				}
			}
			connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		}
	}
	else
		selected_map_ = nullptr;
}

void ComputeCurvature_Dialog::map_added(MapHandlerGen* map)
{
	if (map->dimension() == 2)
	{
		QListWidgetItem* item = new QListWidgetItem(map->get_name(), list_maps);
		item->setCheckState(Qt::Unchecked);
	}
}

void ComputeCurvature_Dialog::map_removed(MapHandlerGen* map)
{
	QList<QListWidgetItem*> items = list_maps->findItems(map->get_name(), Qt::MatchExactly);
	if (!items.empty())
		delete items[0];

	if (selected_map_ == map)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(unsigned int, const QString&)));
		selected_map_ = nullptr;
	}
}

void ComputeCurvature_Dialog::selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
		QString scalar_type_name = QString::fromStdString(cgogn::name_of_type(SCALAR()));

		const CMap2* map2 = selected_map_->get_map();
		const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			combo_positionAttribute->addItem(attribute_name);
			combo_normalAttribute->addItem(attribute_name);
			combo_KmaxAttribute->addItem(attribute_name);
			combo_KminAttribute->addItem(attribute_name);
			combo_KnormalAttribute->addItem(attribute_name);
		}
		else if (attribute_type_name == scalar_type_name)
		{
			combo_kmaxAttribute->addItem(attribute_name);
			combo_kminAttribute->addItem(attribute_name);
		}
	}
}

} // namespace plugin_sdp
} // namespace schnapps
