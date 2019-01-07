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

#include <schnapps/plugins/surface_modelisation/dialog_subdivision.h>
#include <schnapps/plugins/surface_modelisation/surface_modelisation.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_surface_modelisation
{

Subdivision_Dialog::Subdivision_Dialog(SCHNApps* s, Plugin_SurfaceModelisation* p) :
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

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(button_loop, SIGNAL(clicked()), this, SLOT(subdivide_loop()));
	connect(button_catmull_clark, SIGNAL(clicked()), this, SLOT(subdivide_catmull_clark()));
	connect(button_lsm, SIGNAL(clicked()), this, SLOT(subdivide_lsm()));

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	schnapps_->foreach_object([this] (Object* o)
	{
		CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
		if (mh)
			map_added(mh);
	});

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
}

Subdivision_Dialog::~Subdivision_Dialog()
{
	disconnect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	disconnect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void Subdivision_Dialog::selected_map_changed()
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
		selected_map_ = plugin_cmap_provider_->cmap2(map_name);
	}

	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	refresh_ui();
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

void Subdivision_Dialog::subdivide_lsm()
{
	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = combo_positionAttribute->currentText();
		plugin_->subdivide_lsm(map_name, position_name);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void Subdivision_Dialog::object_added(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_added(mh);
}

void Subdivision_Dialog::map_added(CMap2Handler *mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void Subdivision_Dialog::object_removed(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_removed(mh);
}

void Subdivision_Dialog::map_removed(CMap2Handler *mh)
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

void Subdivision_Dialog::selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
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
			updating_ui_ = false;
		}
	}
}

void Subdivision_Dialog::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			int index = combo_positionAttribute->findText(attribute_name, Qt::MatchExactly);
			if (index > 0)
			{
				updating_ui_ = true;
				combo_positionAttribute->removeItem(index);
				updating_ui_ = false;
			}
		}
	}
}

void Subdivision_Dialog::refresh_ui()
{
	updating_ui_ = true;

	combo_positionAttribute->clear();

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
					combo_positionAttribute->addItem(name);
			}

			int idx = combo_positionAttribute->findText(setting_auto_load_position_attribute_);
			if (idx != -1)
				combo_positionAttribute->setCurrentIndex(idx);
		}
	}

	updating_ui_ = false;
}

} // namespace plugin_surface_modelisation

} // namespace schnapps
