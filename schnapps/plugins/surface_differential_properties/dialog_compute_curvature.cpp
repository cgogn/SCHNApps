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

ComputeCurvature_Dialog::ComputeCurvature_Dialog(SCHNApps* s, Plugin_SurfaceDifferentialProperties* p) :
	schnapps_(s),
	plugin_(p),
	selected_map_(nullptr)
{
	setupUi(this);

	setting_auto_load_position_attribute_ = plugin_->get_setting("Auto load position attribute");
	if (!setting_auto_load_position_attribute_.isValid())
		setting_auto_load_position_attribute_ = plugin_->add_setting("Auto load position attribute", "position");

	setting_auto_load_normal_attribute_ = plugin_->get_setting("Auto load normal attribute");
	if (!setting_auto_load_normal_attribute_.isValid())
		setting_auto_load_normal_attribute_ = plugin_->add_setting("Auto load normal attribute", "normal");

	setting_auto_load_Kmax_attribute_ = plugin_->get_setting("Auto load Kmax attribute");
	if (!setting_auto_load_Kmax_attribute_.isValid())
		setting_auto_load_Kmax_attribute_ = plugin_->add_setting("Auto load Kmax attribute", "Kmax");

	setting_auto_load_kmax_attribute_ = plugin_->get_setting("Auto load kmax attribute");
	if (!setting_auto_load_kmax_attribute_.isValid())
		setting_auto_load_kmax_attribute_ = plugin_->add_setting("Auto load kmax attribute", "kmax");

	setting_auto_load_Kmin_attribute_ = plugin_->get_setting("Auto load Kmin attribute");
	if (!setting_auto_load_Kmin_attribute_.isValid())
		setting_auto_load_Kmin_attribute_ = plugin_->add_setting("Auto load Kmin attribute", "Kmin");

	setting_auto_load_kmin_attribute_ = plugin_->get_setting("Auto load kmin attribute");
	if (!setting_auto_load_kmin_attribute_.isValid())
		setting_auto_load_kmin_attribute_ = plugin_->add_setting("Auto load kmin attribute", "kmin");

	setting_auto_load_Knormal_attribute_ = plugin_->get_setting("Auto load Knormal attribute");
	if (!setting_auto_load_Knormal_attribute_.isValid())
		setting_auto_load_Knormal_attribute_ = plugin_->add_setting("Auto load Knormal attribute", "Knormal");

	Kmax_attribute_name->setText(setting_auto_load_Kmax_attribute_.toString());
	kmax_attribute_name->setText(setting_auto_load_kmax_attribute_.toString());
	Kmin_attribute_name->setText(setting_auto_load_Kmin_attribute_.toString());
	kmin_attribute_name->setText(setting_auto_load_kmin_attribute_.toString());
	Knormal_attribute_name->setText(setting_auto_load_Knormal_attribute_.toString());

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(this, SIGNAL(accepted()), this, SLOT(compute_curvature()));
	connect(button_apply, SIGNAL(clicked()), this, SLOT(compute_curvature()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });
}

void ComputeCurvature_Dialog::compute_curvature()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		QString position_name = combo_positionAttribute->currentText();
		QString normal_name = combo_normalAttribute->currentText();

		QString Kmax_name;
		if (Kmax_attribute_name->text().isEmpty())
			Kmax_name = combo_KmaxAttribute->currentText();
		else
			Kmax_name = Kmax_attribute_name->text();

		QString kmax_name;
		if (kmax_attribute_name->text().isEmpty())
			kmax_name = combo_kmaxAttribute->currentText();
		else
			kmax_name = kmax_attribute_name->text();

		QString Kmin_name;
		if (Kmin_attribute_name->text().isEmpty())
			Kmin_name = combo_KminAttribute->currentText();
		else
			Kmin_name = Kmin_attribute_name->text();

		QString kmin_name;
		if (kmin_attribute_name->text().isEmpty())
			kmin_name = combo_kminAttribute->currentText();
		else
			kmin_name = kmin_attribute_name->text();

		QString Knormal_name;
		if (Knormal_attribute_name->text().isEmpty())
			Knormal_name = combo_KnormalAttribute->currentText();
		else
			Knormal_name = Knormal_attribute_name->text();

		bool compute_kmean = check_computeKmean->checkState() == Qt::Checked;
		bool compute_kgaussian = check_computeKgaussian->checkState() == Qt::Checked;
		bool auto_update = currentItems[0]->checkState() == Qt::Checked;

		plugin_->compute_curvature(
			map_name,
			position_name, normal_name,
			Kmax_name, kmax_name, Kmin_name, kmin_name, Knormal_name,
			compute_kmean, compute_kgaussian,
			auto_update
		);
	}
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
						if (name == setting_auto_load_position_attribute_.toString())
							combo_positionAttribute->setCurrentIndex(combo_positionAttribute->count() - 1);
						combo_normalAttribute->addItem(name);
						if (name == setting_auto_load_normal_attribute_.toString())
							combo_normalAttribute->setCurrentIndex(combo_normalAttribute->count() - 1);
						combo_KmaxAttribute->addItem(name);
						if (name == setting_auto_load_Kmax_attribute_.toString())
							combo_KmaxAttribute->setCurrentIndex(combo_KmaxAttribute->count() - 1);
						combo_KminAttribute->addItem(name);
						if (name == setting_auto_load_Kmin_attribute_.toString())
							combo_KminAttribute->setCurrentIndex(combo_KminAttribute->count() - 1);
						combo_KnormalAttribute->addItem(name);
						if (name == setting_auto_load_Knormal_attribute_.toString())
							combo_KnormalAttribute->setCurrentIndex(combo_KnormalAttribute->count() - 1);
					}
					else if (type == scalar_type_name)
					{
						combo_kmaxAttribute->addItem(name);
						if (name == setting_auto_load_kmax_attribute_.toString())
							combo_kmaxAttribute->setCurrentIndex(combo_kmaxAttribute->count() - 1);
						combo_kminAttribute->addItem(name);
						if (name == setting_auto_load_kmin_attribute_.toString())
							combo_kminAttribute->setCurrentIndex(combo_kminAttribute->count() - 1);
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
