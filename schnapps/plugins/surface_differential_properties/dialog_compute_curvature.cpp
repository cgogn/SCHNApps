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

#include <schnapps/plugins/surface_differential_properties/dialog_compute_curvature.h>
#include <schnapps/plugins/surface_differential_properties/surface_differential_properties.h>

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

	if (plugin_->get_setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = plugin_->get_setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = plugin_->add_setting("Auto load position attribute", "position").toString();

	if (plugin_->get_setting("Auto load normal attribute").isValid())
		setting_auto_load_normal_attribute_ = plugin_->get_setting("Auto load normal attribute").toString();
	else
		setting_auto_load_normal_attribute_ = plugin_->add_setting("Auto load normal attribute", "normal").toString();

	if (plugin_->get_setting("Default Kmax attribute name").isValid())
		setting_default_Kmax_attribute_name_ = plugin_->get_setting("Default Kmax attribute name").toString();
	else
		setting_default_Kmax_attribute_name_ = plugin_->add_setting("Default Kmax attribute name", "Kmax").toString();

	if (plugin_->get_setting("Default kmax attribute name").isValid())
		setting_default_kmax_attribute_name_ = plugin_->get_setting("Default kmax attribute name").toString();
	else
		setting_default_kmax_attribute_name_ = plugin_->add_setting("Default kmax attribute name", "kmax").toString();

	if (plugin_->get_setting("Default Kmin attribute name").isValid())
		setting_default_Kmin_attribute_name_ = plugin_->get_setting("Default Kmin attribute name").toString();
	else
		setting_default_Kmin_attribute_name_ = plugin_->add_setting("Default Kmin attribute name", "Kmin").toString();

	if (plugin_->get_setting("Default kmin attribute name").isValid())
		setting_default_kmin_attribute_name_ = plugin_->get_setting("Default kmin attribute name").toString();
	else
		setting_default_kmin_attribute_name_ = plugin_->add_setting("Default kmin attribute name", "kmin").toString();

	if (plugin_->get_setting("Default Knormal attribute name").isValid())
		setting_default_Knormal_attribute_name_ = plugin_->get_setting("Default Knormal attribute name").toString();
	else
		setting_default_Knormal_attribute_name_ = plugin_->add_setting("Default Knormal attribute name", "Knormal").toString();

	if (plugin_->get_setting("Default kmean attribute name").isValid())
		setting_default_kmean_attribute_name_ = plugin_->get_setting("Default kmean attribute name").toString();
	else
		setting_default_kmean_attribute_name_ = plugin_->add_setting("Default kmean attribute name", "kmean").toString();

	if (plugin_->get_setting("Default kgaussian attribute name").isValid())
		setting_default_kgaussian_attribute_name_ = plugin_->get_setting("Default kgaussian attribute name").toString();
	else
		setting_default_kgaussian_attribute_name_ = plugin_->add_setting("Default kgaussian attribute name", "kgaussian").toString();

	Kmax_attribute_name->setText(setting_default_Kmax_attribute_name_);
	kmax_attribute_name->setText(setting_default_kmax_attribute_name_);
	Kmin_attribute_name->setText(setting_default_Kmin_attribute_name_);
	kmin_attribute_name->setText(setting_default_kmin_attribute_name_);
	Knormal_attribute_name->setText(setting_default_Knormal_attribute_name_);
	kmean_attribute_name->setText(setting_default_kmean_attribute_name_);
	kgaussian_attribute_name->setText(setting_default_kgaussian_attribute_name_);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(this, SIGNAL(accepted()), this, SLOT(compute_curvature()));
	connect(button_apply, SIGNAL(clicked()), this, SLOT(compute_curvature()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

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
		QString Kmax_name = Kmax_attribute_name->text();
		bool create_vbo_Kmax = check_create_vbo_Kmax->isChecked();
		QString kmax_name = kmax_attribute_name->text();
		bool create_vbo_kmax = check_create_vbo_kmax->isChecked();
		QString Kmin_name = Kmin_attribute_name->text();
		bool create_vbo_Kmin = check_create_vbo_Kmin->isChecked();
		QString kmin_name = kmin_attribute_name->text();
		bool create_vbo_kmin = check_create_vbo_kmin->isChecked();
		QString Knormal_name = Knormal_attribute_name->text();
		bool create_vbo_Knormal = check_create_vbo_Knormal->isChecked();

		bool compute_kmean = check_compute_kmean->checkState() == Qt::Checked;
		QString kmean_name = kmean_attribute_name->text();
		bool create_vbo_kmean = check_create_vbo_kmean->isChecked();
		bool compute_kgaussian = check_compute_kgaussian->checkState() == Qt::Checked;
		QString kgaussian_name = kgaussian_attribute_name->text();
		bool create_vbo_kgaussian = check_create_vbo_kgaussian->isChecked();

		bool auto_update = currentItems[0]->checkState() == Qt::Checked;

		plugin_->compute_curvature(
			map_name,
			position_name, normal_name,
			Kmax_name, create_vbo_Kmax,
			kmax_name, create_vbo_kmax,
			Kmin_name, create_vbo_Kmin,
			kmin_name, create_vbo_kmin,
			Knormal_name, create_vbo_Knormal,
			compute_kmean, kmean_name, create_vbo_kmean,
			compute_kgaussian, kgaussian_name, create_vbo_kgaussian,
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

		const QString& map_name = currentItems[0]->text();
		MapHandlerGen* mhg = schnapps_->get_map(map_name);
		selected_map_ = dynamic_cast<CMap2Handler*>(mhg);

		if (selected_map_)
		{
			const CMap2* map2 = selected_map_->get_map();
			if (map2->is_embedded<CMap2::Vertex::ORBIT>())
			{
				QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

				const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
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
					}
				}

				if (plugin_->has_compute_curvature_last_parameters(selected_map_))
				{
					const Plugin_SurfaceDifferentialProperties::ComputeCurvatureParameters& p = plugin_->get_compute_curvature_last_parameters(selected_map_);

					int idx = combo_positionAttribute->findText(p.position_name_);
					if (idx == -1)
						idx = combo_positionAttribute->findText(setting_auto_load_position_attribute_);
					if (idx != -1)
						combo_positionAttribute->setCurrentIndex(idx);

					idx = combo_normalAttribute->findText(p.normal_name_);
					if (idx == -1)
						idx = combo_normalAttribute->findText(setting_auto_load_normal_attribute_);
					if (idx != -1)
						combo_normalAttribute->setCurrentIndex(idx);

					Kmax_attribute_name->setText(p.Kmax_name_);
					check_create_vbo_Kmax->setChecked(p.create_vbo_Kmax_);
					kmax_attribute_name->setText(p.kmax_name_);
					check_create_vbo_kmax->setChecked(p.create_vbo_kmax_);
					Kmin_attribute_name->setText(p.Kmin_name_);
					check_create_vbo_Kmin->setChecked(p.create_vbo_Kmin_);
					kmin_attribute_name->setText(p.kmin_name_);
					check_create_vbo_kmin->setChecked(p.create_vbo_kmin_);
					Knormal_attribute_name->setText(p.Knormal_name_);
					check_create_vbo_Knormal->setChecked(p.create_vbo_Knormal_);
					check_compute_kmean->setChecked(p.compute_kmean_);
					kmean_attribute_name->setText(p.kmean_name_);
					check_create_vbo_kmean->setChecked(p.create_vbo_kmean_);
					check_compute_kgaussian->setChecked(p.compute_kgaussian_);
					kgaussian_attribute_name->setText(p.kgaussian_name_);
					check_create_vbo_kgaussian->setChecked(p.create_vbo_kgaussian_);
				}
				else
				{
					int idx = combo_positionAttribute->findText(setting_auto_load_position_attribute_);
					if (idx != -1)
						combo_positionAttribute->setCurrentIndex(idx);

					idx = combo_normalAttribute->findText(setting_auto_load_normal_attribute_);
					if (idx != -1)
						combo_normalAttribute->setCurrentIndex(idx);

					Kmax_attribute_name->setText(setting_default_Kmax_attribute_name_);
					check_create_vbo_Kmax->setChecked(false);
					kmax_attribute_name->setText(setting_default_kmax_attribute_name_);
					check_create_vbo_kmax->setChecked(false);
					Kmin_attribute_name->setText(setting_default_Kmin_attribute_name_);
					check_create_vbo_Kmin->setChecked(false);
					kmin_attribute_name->setText(setting_default_kmin_attribute_name_);
					check_create_vbo_kmin->setChecked(false);
					Knormal_attribute_name->setText(setting_default_Knormal_attribute_name_);
					check_create_vbo_Knormal->setChecked(false);
					check_compute_kmean->setChecked(false);
					kmean_attribute_name->setText(setting_default_kmean_attribute_name_);
					check_create_vbo_kmean->setChecked(false);
					check_compute_kgaussian->setChecked(false);
					kgaussian_attribute_name->setText(setting_default_kgaussian_attribute_name_);
					check_create_vbo_kgaussian->setChecked(false);
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

		const CMap2* map2 = selected_map_->get_map();
		const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
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
