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

#include <schnapps/plugins/surface_modelisation/dialog_filtering.h>
#include <schnapps/plugins/surface_modelisation/surface_modelisation.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_surface_modelisation
{

Filtering_Dialog::Filtering_Dialog(SCHNApps* s, Plugin_SurfaceModelisation* p) :
	schnapps_(s),
	plugin_(p),
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	if (plugin_->setting("Auto load position attribute").isValid())
		setting_auto_load_position_attribute_ = plugin_->setting("Auto load position attribute").toString();
	else
		setting_auto_load_position_attribute_ = plugin_->add_setting("Auto load position attribute", "position").toString();

	if (plugin_->setting("Auto load normal attribute").isValid())
		setting_auto_load_normal_attribute_ = plugin_->setting("Auto load normal attribute").toString();
	else
		setting_auto_load_normal_attribute_ = plugin_->add_setting("Auto load normal attribute", "normal").toString();

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(button_average, SIGNAL(clicked()), this, SLOT(filter_average()));
	connect(button_bilateral, SIGNAL(clicked()), this, SLOT(filter_bilateral()));
	connect(button_laplacian, SIGNAL(clicked()), this, SLOT(filter_laplacian()));
	connect(button_taubin, SIGNAL(clicked()), this, SLOT(filter_taubin()));

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	schnapps_->foreach_object([this] (Object* o)
	{
		CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
		if (mh)
			map_added(mh);
	});

	plugin_cmap2_provider_ = static_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));
}

Filtering_Dialog::~Filtering_Dialog()
{
	disconnect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	disconnect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void Filtering_Dialog::selected_map_changed()
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	selected_map_ = nullptr;

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		selected_map_ = plugin_cmap2_provider_->map(map_name);
	}

	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	refresh_ui();
}

void Filtering_Dialog::filter_average()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		plugin_->filter_average(map_name, position_name);
	}
}

void Filtering_Dialog::filter_bilateral()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		QString normal_name = combo_normalAttribute->currentText();
		plugin_->filter_bilateral(map_name, position_name, normal_name);
	}
}

void Filtering_Dialog::filter_taubin()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		plugin_->filter_taubin(map_name, position_name);
	}
}

void Filtering_Dialog::filter_laplacian()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		plugin_->filter_laplacian(map_name, position_name);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void Filtering_Dialog::object_added(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_added(mh);
}

void Filtering_Dialog::map_added(CMap2Handler *mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void Filtering_Dialog::object_removed(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_removed(mh);
}

void Filtering_Dialog::map_removed(CMap2Handler *mh)
{
	if (selected_map_ == mh)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		selected_map_ = nullptr;
	}

	QList<QListWidgetItem*> items = list_maps->findItems(mh->name(), Qt::MatchExactly);
	if (!items.empty())
	{
		updating_ui_ = true;
		delete items[0];
		updating_ui_ = false;
	}
}

/*****************************************************************************/
// slots called from CMap2Handler signals
/*****************************************************************************/

void Filtering_Dialog::selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			updating_ui_ = true;
			combo_positionAttribute->addItem(attribute_name);
			combo_normalAttribute->addItem(attribute_name);
			updating_ui_ = false;
		}
	}
}

void Filtering_Dialog::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			updating_ui_ = true;
			int index = combo_positionAttribute->findText(attribute_name, Qt::MatchExactly);
			if (index > 0)
				combo_positionAttribute->removeItem(index);
			index = combo_normalAttribute->findText(attribute_name, Qt::MatchExactly);
			if (index > 0)
				combo_normalAttribute->removeItem(index);
			updating_ui_ = false;
		}
	}
}

void Filtering_Dialog::refresh_ui()
{
	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_normalAttribute->clear();

	if (selected_map_)
	{
		const CMap2* map = selected_map_->map();

		if (map->is_embedded<CMap2::Vertex>())
		{
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

			const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
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

			int idx = combo_positionAttribute->findText(setting_auto_load_position_attribute_);
			if (idx != -1)
				combo_positionAttribute->setCurrentIndex(idx);
			idx = combo_normalAttribute->findText(setting_auto_load_normal_attribute_);
			if (idx != -1)
				combo_normalAttribute->setCurrentIndex(idx);
		}
	}

	updating_ui_ = false;
}

} // namespace plugin_surface_modelisation

} // namespace schnapps
