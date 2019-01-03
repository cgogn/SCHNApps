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

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_cage_3d_deformation
{

Cage3dDeformation_Dialog::Cage3dDeformation_Dialog(SCHNApps* s, Plugin_Cage3dDeformation* p) :
	schnapps_(s),
	plugin_(p),
	selected_deformed_map_(nullptr),
	selected_control_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	list_control_maps->setEnabled(false);
	combo_controlPositionAttribute->setEnabled(false);
	button_linkUnlink->setEnabled(false);

	connect(list_deformed_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_deformed_map_changed()));
	connect(combo_deformedPositionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(deformed_position_attribute_changed(int)));
	connect(list_control_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_control_map_changed()));
	connect(combo_controlPositionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(control_position_attribute_changed(int)));

	connect(button_linkUnlink, SIGNAL(clicked()), this, SLOT(toggle_control()));

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

Cage3dDeformation_Dialog::~Cage3dDeformation_Dialog()
{
	disconnect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	disconnect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void Cage3dDeformation_Dialog::selected_deformed_map_changed()
{
	if (selected_deformed_map_)
	{
		disconnect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_deformed_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	QList<QListWidgetItem*> currentItems = list_deformed_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		selected_deformed_map_ = plugin_cmap2_provider_->map(map_name);

		if (selected_deformed_map_)
		{
			connect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));
			connect(selected_deformed_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_removed(cgogn::Orbit, const QString&)));
		}

		update_after_selected_deformed_map_changed();
	}
	else
	{
		selected_deformed_map_ = nullptr;
		list_control_maps->setEnabled(false);
		combo_controlPositionAttribute->setEnabled(false);
		button_linkUnlink->setEnabled(false);
	}
}

void Cage3dDeformation_Dialog::deformed_position_attribute_changed(int)
{
	if (!updating_ui_)
		plugin_->set_deformed_position_attribute(selected_deformed_map_, combo_deformedPositionAttribute->currentText(), false);
}

void Cage3dDeformation_Dialog::selected_control_map_changed()
{
	if (selected_control_map_)
	{
		disconnect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_control_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	QList<QListWidgetItem*> currentItems = list_control_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		selected_control_map_ = plugin_cmap2_provider_->map(map_name);
		if (!updating_ui_)
			plugin_->set_control_map(selected_deformed_map_, selected_control_map_, false);

		if (selected_control_map_)
		{
			connect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));
			connect(selected_control_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_removed(cgogn::Orbit, const QString&)));
		}

		update_after_selected_control_map_changed();
	}
}

void Cage3dDeformation_Dialog::control_position_attribute_changed(int)
{
	if (!updating_ui_)
		plugin_->set_control_position_attribute(selected_deformed_map_, combo_controlPositionAttribute->currentText(), false);
}

void Cage3dDeformation_Dialog::toggle_control()
{
	if (selected_deformed_map_)
		plugin_->toggle_control(selected_deformed_map_, true);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void Cage3dDeformation_Dialog::object_added(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_added(mh);
}

void Cage3dDeformation_Dialog::map_added(CMap2Handler* mh)
{
	updating_ui_ = true;
	list_deformed_maps->addItem(mh->name());
	list_control_maps->addItem(mh->name());
	updating_ui_ = false;
}

void Cage3dDeformation_Dialog::object_removed(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_removed(mh);
}

void Cage3dDeformation_Dialog::map_removed(CMap2Handler* mh)
{
	if (selected_deformed_map_ == mh)
	{
		disconnect(selected_deformed_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_deformed_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_removed(cgogn::Orbit, const QString&)));
		selected_deformed_map_ = nullptr;
	}
	QList<QListWidgetItem*> deformed_items = list_deformed_maps->findItems(mh->name(), Qt::MatchExactly);
	if (!deformed_items.empty())
		delete deformed_items[0];

	if (selected_control_map_ == mh)
	{
		disconnect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_control_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_removed(cgogn::Orbit, const QString&)));
		selected_control_map_ = nullptr;
	}
	QList<QListWidgetItem*> control_items = list_control_maps->findItems(mh->name(), Qt::MatchExactly);
	if (!control_items.empty())
		delete control_items[0];
}

/*****************************************************************************/
// slots called from CMap2Handler signals
/*****************************************************************************/

void Cage3dDeformation_Dialog::selected_deformed_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_deformed_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			updating_ui_ = true;
			combo_deformedPositionAttribute->addItem(attribute_name);
			updating_ui_ = false;
		}
	}
}

void Cage3dDeformation_Dialog::selected_deformed_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_deformed_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			int index = combo_deformedPositionAttribute->findText(attribute_name, Qt::MatchExactly);
			if (index > 0)
			{
				updating_ui_ = true;
				combo_deformedPositionAttribute->removeItem(index);
				updating_ui_ = false;
			}
		}
	}
}

void Cage3dDeformation_Dialog::selected_control_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map2 = selected_control_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			updating_ui_ = true;
			combo_controlPositionAttribute->addItem(attribute_name);
			updating_ui_ = false;
		}
	}
}

void Cage3dDeformation_Dialog::selected_control_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_control_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			int index = combo_controlPositionAttribute->findText(attribute_name, Qt::MatchExactly);
			if (index > 0)
			{
				updating_ui_ = true;
				combo_controlPositionAttribute->removeItem(index);
				updating_ui_ = false;
			}
		}
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

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

void Cage3dDeformation_Dialog::set_selected_control_map(CMap2Handler* map)
{
	if (selected_control_map_)
		disconnect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));

	selected_control_map_ = map;

	if (selected_control_map_)
		connect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_control_map_attribute_added(cgogn::Orbit, const QString&)));

	updating_ui_ = true;
	QList<QListWidgetItem*> foundItems = list_control_maps->findItems(selected_control_map_->name(), Qt::MatchExactly);
	if (!foundItems.empty())
		list_control_maps->setCurrentItem(foundItems[0]);
	updating_ui_ = false;

	update_after_selected_control_map_changed();
}

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

void Cage3dDeformation_Dialog::set_linked(bool state)
{
	updating_ui_ = true;
	list_control_maps->setEnabled(!state);
	combo_controlPositionAttribute->setEnabled(!state);
	button_linkUnlink->setText(state ? "Unlink" : "Link");
	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void Cage3dDeformation_Dialog::update_after_selected_deformed_map_changed()
{
	updating_ui_ = true;
	combo_deformedPositionAttribute->clear();
	combo_deformedPositionAttribute->addItem("- select attribute -");

	if (selected_deformed_map_)
	{
		const CMap2* map2 = selected_deformed_map_->map();
		if (map2->is_embedded<CMap2::Vertex>())
		{
			const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
			const std::vector<std::string>& names = container.names();
			const std::vector<std::string>& type_names = container.type_names();
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

			const MapParameters& p = plugin_->parameters(selected_deformed_map_);

			const CMap2::VertexAttribute<VEC3>& pos = p.deformed_position_attribute();
			unsigned int i = 1;
			for (std::size_t j = 0u; j < names.size(); ++j)
			{
				QString name = QString::fromStdString(names[j]);
				QString type = QString::fromStdString(type_names[j]);
				if (type == vec3_type_name)
				{
					combo_deformedPositionAttribute->addItem(name);
					if (pos.is_valid() && QString::fromStdString(pos.name()) == name)
						combo_deformedPositionAttribute->setCurrentIndex(i);

					++i;
				}
			}

			if (!p.linked())
			{
				list_control_maps->setEnabled(true);
				combo_controlPositionAttribute->setEnabled(true);
				button_linkUnlink->setEnabled(true);
				button_linkUnlink->setText("Link");
			}
			else
			{
				list_control_maps->setEnabled(false);
				combo_controlPositionAttribute->setEnabled(false);
				button_linkUnlink->setEnabled(true);
				button_linkUnlink->setText("Unlink");
			}

			CMap2Handler* control_map = p.control_map();
			if (control_map)
			{
				QList<QListWidgetItem*> items = list_control_maps->findItems(control_map->name(), Qt::MatchExactly);
				if (!items.empty())
					list_control_maps->setCurrentItem(items[0]);
			}
			else
			{
				list_control_maps->setCurrentItem(nullptr);
				if (selected_control_map_)
					disconnect(selected_control_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_deformed_map_attribute_added(cgogn::Orbit, const QString&)));
				selected_control_map_ = nullptr;
				update_after_selected_control_map_changed();
			}
		}
	}
	updating_ui_ = false;
}

void Cage3dDeformation_Dialog::update_after_selected_control_map_changed()
{
	updating_ui_ = true;
	combo_controlPositionAttribute->clear();
	combo_controlPositionAttribute->addItem("- select attribute -");

	if (selected_deformed_map_ && selected_control_map_)
	{
		const CMap2* map2 = selected_control_map_->map();
		if (map2->is_embedded<CMap2::Vertex::ORBIT>())
		{
			const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
			const std::vector<std::string>& names = container.names();
			const std::vector<std::string>& type_names = container.type_names();
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

			const MapParameters& p = plugin_->parameters(selected_deformed_map_);

			unsigned int i = 1;
			for (std::size_t j = 0u; j < names.size(); ++j)
			{
				QString name = QString::fromStdString(names[j]);
				QString type = QString::fromStdString(type_names[j]);
				if (type == vec3_type_name)
				{
					combo_controlPositionAttribute->addItem(name);
					const CMap2::VertexAttribute<VEC3>& pos = p.control_position_attribute();
					if (pos.is_valid() && QString::fromStdString(pos.name()) == name)
						combo_controlPositionAttribute->setCurrentIndex(i);

					++i;
				}
			}
		}
	}
	updating_ui_ = false;
}

} // namespace plugin_cage_3d_deformation

} // namespace schnapps
