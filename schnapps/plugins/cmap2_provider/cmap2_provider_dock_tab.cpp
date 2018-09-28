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

#include <schnapps/plugins/cmap2_provider/cmap2_provider_dock_tab.h>
#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

namespace schnapps
{

namespace plugin_cmap2_provider
{

CMap2Provider_DockTab::CMap2Provider_DockTab(SCHNApps* s, Plugin_CMap2Provider* p) :
	schnapps_(s),
	plugin_(p)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

//	connect(button_duplicate, SIGNAL(clicked()), this, SLOT(duplicate_current_map_clicked()));
	connect(button_remove, SIGNAL(clicked()), this, SLOT(remove_current_map_clicked()));

	connect(combo_bbVertexAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(bb_vertex_attribute_changed(int)));
	connect(list_vertexAttributes, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vertex_attribute_check_state_changed(QListWidgetItem*)));

//	connect(list_dartSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
//	connect(button_dartAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
//	connect(button_dartRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));

//	connect(list_vertexSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
//	connect(button_vertexAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
//	connect(button_vertexRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));

//	connect(list_edgeSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
//	connect(button_edgeAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
//	connect(button_edgeRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));

//	connect(list_faceSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
//	connect(button_faceAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
//	connect(button_faceRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));

//	connect(list_volumeSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
//	connect(button_volumeAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
//	connect(button_volumeRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));
}

CMap2Provider_DockTab::~CMap2Provider_DockTab()
{

}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void CMap2Provider_DockTab::selected_map_changed()
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
		disconnect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
//		disconnect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
//		disconnect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
//		disconnect(selected_map_, SIGNAL(cells_set_mutually_exclusive_changed(CellType, const QString&)), this, SLOT(selected_map_cells_set_mutually_exclusive_changed(CellType, const QString&)));
	}

	selected_map_ = nullptr;

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		selected_map_ = plugin_->map(map_name);
	}

	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
		connect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
		connect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
//		connect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
//		connect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
//		connect(selected_map_, SIGNAL(cells_set_mutually_exclusive_changed(CellType, const QString&)), this, SLOT(selected_map_cells_set_mutually_exclusive_changed(CellType, const QString&)));
	}

	refresh_ui();
}

//void CMap2Provider_DockTab::duplicate_current_map_clicked()
//{
//	if (!updating_ui_ && selected_map_)
//		plugin_->duplicate_map(selected_map_->name());
//}

void CMap2Provider_DockTab::remove_current_map_clicked()
{
	if (!updating_ui_ && selected_map_)
		plugin_->remove_map(selected_map_->name());
}

void CMap2Provider_DockTab::bb_vertex_attribute_changed(int)
{
	if (!updating_ui_ && selected_map_)
		selected_map_->set_bb_vertex_attribute(combo_bbVertexAttribute->currentText());
}

void CMap2Provider_DockTab::vertex_attribute_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_ && selected_map_)
	{
		if (item->checkState() == Qt::Checked)
		{
			cgogn::rendering::VBO* vbo = selected_map_->create_vbo(item->text());
			if (!vbo)
			{
				updating_ui_ = true;
				item->setCheckState(Qt::Unchecked);
				updating_ui_ = false;
			}
		}
		else
			selected_map_->delete_vbo(item->text());
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/



/*****************************************************************************/
// slots called from CMap2Handler signals
/*****************************************************************************/

void CMap2Provider_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	switch (orbit)
	{
		case CMap2::CDart::ORBIT: {
			list_dartAttributes->addItem(name);
			break;
		}
		case CMap2::Vertex::ORBIT: {
			QListWidgetItem* item = new QListWidgetItem(name, list_vertexAttributes);
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
			const auto* cag = selected_map_->map()->attribute_container<CMap2::Vertex::ORBIT>().get_chunk_array(name.toStdString());
			if (cag)
			{
				QString type_name = QString::fromStdString(cag->type_name());
				if (type_name == vec3_type_name)
					combo_bbVertexAttribute->addItem(name);
			}
			if (selected_map_->vbo(name))
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
			break;
		}
		case CMap2::Edge::ORBIT: {
			list_edgeAttributes->addItem(name);
			break;
		}
		case CMap2::Face::ORBIT: {
			list_faceAttributes->addItem(name);
			break;
		}
		case CMap2::Volume::ORBIT: {
			list_volumeAttributes->addItem(name);
			break;
		}
		default: break;
	}
	updating_ui_ = false;
}

void CMap2Provider_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	switch (orbit)
	{
		case CMap2::CDart::ORBIT: {
			QList<QListWidgetItem*> items = list_dartAttributes->findItems(name, Qt::MatchExactly);
			if (!items.empty()) delete items[0];
			break;
		}
		case CMap2::Vertex::ORBIT: {
			QList<QListWidgetItem*> items = list_vertexAttributes->findItems(name, Qt::MatchExactly);
			if (!items.empty()) delete items[0];
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
			const auto* cag = selected_map_->map()->attribute_container<CMap2::Vertex::ORBIT>().get_chunk_array(name.toStdString());
			if (cag)
			{
				QString type_name = QString::fromStdString(cag->type_name());
				if (type_name == vec3_type_name)
				{
					int index = combo_bbVertexAttribute->findText(name, Qt::MatchExactly);
					if (index > 0)
						combo_bbVertexAttribute->removeItem(index);
				}
			}
			break;
		}
		case CMap2::Edge::ORBIT: {
			QList<QListWidgetItem*> items = list_edgeAttributes->findItems(name, Qt::MatchExactly);
			if (!items.empty()) delete items[0];
			break;
		}
		case CMap2::Face::ORBIT: {
			QList<QListWidgetItem*> items = list_faceAttributes->findItems(name, Qt::MatchExactly);
			if (!items.empty()) delete items[0];
			break;
		}
		case CMap2::Volume::ORBIT: {
			QList<QListWidgetItem*> items = list_volumeAttributes->findItems(name, Qt::MatchExactly);
			if (!items.empty()) delete items[0];
			break;
		}
		default: break;
	}
	updating_ui_ = false;
}

void CMap2Provider_DockTab::selected_map_bb_vertex_attribute_changed(const QString& name)
{
	updating_ui_ = true;
	int index = combo_bbVertexAttribute->findText(name, Qt::MatchExactly);
	if (index > 0)
		combo_bbVertexAttribute->setCurrentIndex(index);
	updating_ui_ = false;
}

void CMap2Provider_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	QList<QListWidgetItem*> items = list_vertexAttributes->findItems(QString::fromStdString(vbo->name()), Qt::MatchExactly);
	if (!items.empty())
		items[0]->setCheckState(Qt::Checked);
	updating_ui_ = false;
}

void CMap2Provider_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	QList<QListWidgetItem*> items = list_vertexAttributes->findItems(QString::fromStdString(vbo->name()), Qt::MatchExactly);
	if (!items.empty())
		items[0]->setCheckState(Qt::Unchecked);
	updating_ui_ = false;
}

void CMap2Provider_DockTab::selected_map_connectivity_changed()
{
	updating_ui_ = true;

	label_dartNbCells->setText(QString::number(0));
	label_vertexNbCells->setText(QString::number(0));
	label_edgeNbCells->setText(QString::number(0));
	label_faceNbCells->setText(QString::number(0));
	label_volumeNbCells->setText(QString::number(0));

	if (selected_map_)
	{
		const uint32 nb_d = selected_map_->map()->nb_cells<CMap2::CDart>();
		label_dartNbCells->setText(QString::number(nb_d));
		const uint32 nb_v = selected_map_->map()->nb_cells<CMap2::Vertex>();
		label_vertexNbCells->setText(QString::number(nb_v));
		const uint32 nb_e = selected_map_->map()->nb_cells<CMap2::Edge>();
		label_edgeNbCells->setText(QString::number(nb_e));
		const uint32 nb_f = selected_map_->map()->nb_cells<CMap2::Face>();
		label_faceNbCells->setText(QString::number(nb_f));
		const uint32 nb_vol = selected_map_->map()->nb_cells<CMap2::Volume>();
		label_volumeNbCells->setText(QString::number(nb_vol));
	}

	updating_ui_ = false;
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void CMap2Provider_DockTab::add_map(CMap2Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void CMap2Provider_DockTab::remove_map(CMap2Handler* mh)
{
	QList<QListWidgetItem*> items = list_maps->findItems(mh->name(), Qt::MatchExactly);
	if (!items.empty())
	{
		updating_ui_ = true;
		delete items[0];
		updating_ui_ = false;
	}

	if (selected_map_ == mh)
	{
		selected_map_ = nullptr;
		refresh_ui();
	}
}

void CMap2Provider_DockTab::refresh_ui()
{
	updating_ui_ = true;

	combo_bbVertexAttribute->clear();
	combo_bbVertexAttribute->addItem("- select attribute -");
	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	list_dartAttributes->clear();
	list_vertexAttributes->clear();
	list_edgeAttributes->clear();
	list_faceAttributes->clear();
	list_volumeAttributes->clear();

	list_dartSelectors->clear();
	list_vertexSelectors->clear();
	list_edgeSelectors->clear();
	list_faceSelectors->clear();
	list_volumeSelectors->clear();

	label_dartNbCells->setText(QString::number(0));
	label_vertexNbCells->setText(QString::number(0));
	label_edgeNbCells->setText(QString::number(0));
	label_faceNbCells->setText(QString::number(0));
	label_volumeNbCells->setText(QString::number(0));

	if (selected_map_)
	{
		CMap2* map = selected_map_->map();

		const uint32 nb_d = map->nb_cells<CMap2::CDart>();
		label_dartNbCells->setText(QString::number(nb_d));

		if (map->is_embedded<CMap2::CDart>())
		{
			const auto& container = map->attribute_container<CMap2::CDart::ORBIT>();
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_dartAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

//		selected_map_->foreach_cells_set(Dart_Cell, [&] (CellsSetGen* cells_set)
//		{
//			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_dartSelectors);
//			item->setFlags(item->flags() | Qt::ItemIsEditable);
//			if (cells_set->is_mutually_exclusive())
//				item->setCheckState(Qt::Checked);
//			else
//				item->setCheckState(Qt::Unchecked);
//		});

		const uint32 nb_v = map->nb_cells<CMap2::Vertex>();
		label_vertexNbCells->setText(QString::number(nb_v));

		if (map->is_embedded<CMap2::Vertex>())
		{
			const auto& container = map->attribute_container<CMap2::Vertex::ORBIT>();
			const std::vector<std::string>& names = container.names();
			const std::vector<std::string>& type_names = container.type_names();
			unsigned int bb_index = 1;
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
				QString type = QString::fromStdString(type_names[i]);
				QListWidgetItem* item = new QListWidgetItem(name /*+ " (" + type + ")"*/, list_vertexAttributes);
				if (type == vec3_type_name)
				{
					combo_bbVertexAttribute->addItem(name);
					if (selected_map_->bb_vertex_attribute_name() == name)
						combo_bbVertexAttribute->setCurrentIndex(bb_index);
					++bb_index;
				}
				if (selected_map_->vbo(name))
					item->setCheckState(Qt::Checked);
				else
					item->setCheckState(Qt::Unchecked);
			}
		}

//		selected_map_->foreach_cells_set(Vertex_Cell, [&] (CellsSetGen* cells_set)
//		{
//			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_vertexSelectors);
//			item->setFlags(item->flags() | Qt::ItemIsEditable);
//			if (cells_set->is_mutually_exclusive())
//				item->setCheckState(Qt::Checked);
//			else
//				item->setCheckState(Qt::Unchecked);
//		});

		const uint32 nb_e = map->nb_cells<CMap2::Edge>();
		label_edgeNbCells->setText(QString::number(nb_e));

		if (map->is_embedded<CMap2::Edge>())
		{
			const auto& container = map->attribute_container<CMap2::Edge::ORBIT>();
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_edgeAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

//		selected_map_->foreach_cells_set(Edge_Cell, [&] (CellsSetGen* cells_set)
//		{
//			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_edgeSelectors);
//			item->setFlags(item->flags() | Qt::ItemIsEditable);
//			if (cells_set->is_mutually_exclusive())
//				item->setCheckState(Qt::Checked);
//			else
//				item->setCheckState(Qt::Unchecked);
//		});

		const uint32 nb_f = map->nb_cells<CMap2::Face>();
		label_faceNbCells->setText(QString::number(nb_f));

		if (map->is_embedded<CMap2::Face>())
		{
			const auto& container = map->attribute_container<CMap2::Face::ORBIT>();
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_faceAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

//		selected_map_->foreach_cells_set(Face_Cell, [&] (CellsSetGen* cells_set)
//		{
//			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_faceSelectors);
//			item->setFlags(item->flags() | Qt::ItemIsEditable);
//			if (cells_set->is_mutually_exclusive())
//				item->setCheckState(Qt::Checked);
//			else
//				item->setCheckState(Qt::Unchecked);
//		});

		const uint32 nb_vol = map->nb_cells<CMap2::Volume>();
		label_volumeNbCells->setText(QString::number(nb_vol));

		if (map->is_embedded<CMap2::Volume>())
		{
			const auto& container = map->attribute_container<CMap2::Volume::ORBIT>();
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_volumeAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

//		selected_map_->foreach_cells_set(Volume_Cell, [&] (CellsSetGen* cells_set)
//		{
//			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_volumeSelectors);
//			item->setFlags(item->flags() | Qt::ItemIsEditable);
//			if (cells_set->is_mutually_exclusive())
//				item->setCheckState(Qt::Checked);
//			else
//				item->setCheckState(Qt::Unchecked);
//		});
	}

	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/



} // namespace plugin_cmap2_provider

} // namespace schnapps
