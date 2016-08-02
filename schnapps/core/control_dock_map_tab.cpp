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

#include <schnapps/core/control_dock_map_tab.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

ControlDock_MapTab::ControlDock_MapTab(SCHNApps* s) :
	schnapps_(s),
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	// connect UI signals
	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(button_duplicate, SIGNAL(clicked()), this, SLOT(duplicate_current_map_clicked()));
	connect(button_remove, SIGNAL(clicked()), this, SLOT(remove_current_map_clicked()));

	connect(check_drawBB, SIGNAL(toggled(bool)), this, SLOT(show_bb_changed(bool)));
	connect(combo_bbVertexAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(bb_vertex_attribute_changed(int)));
	connect(list_vertexAttributes, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vertex_attribute_check_state_changed(QListWidgetItem*)));

	connect(list_dartSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
	connect(button_dartAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
	connect(button_dartRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));
	connect(button_dartRemoveAttribute, SIGNAL(clicked()), this, SLOT(remove_attribute()));

	connect(list_vertexSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
	connect(button_vertexAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
	connect(button_vertexRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));
	connect(button_vertexRemoveAttribute, SIGNAL(clicked()), this, SLOT(remove_attribute()));

	connect(list_edgeSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
	connect(button_edgeAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
	connect(button_edgeRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));
	connect(button_edgeRemoveAttribute, SIGNAL(clicked()), this, SLOT(remove_attribute()));

	connect(list_faceSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
	connect(button_faceAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
	connect(button_faceRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));
	connect(button_faceRemoveAttribute, SIGNAL(clicked()), this, SLOT(remove_attribute()));

	connect(list_volumeSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
	connect(button_volumeAddSelector, SIGNAL(clicked()), this, SLOT(add_cells_set()));
	connect(button_volumeRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_cells_set()));
	connect(button_volumeRemoveAttribute, SIGNAL(clicked()), this, SLOT(remove_attribute()));

	// connect SCHNApps signals
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
}

CellType ControlDock_MapTab::get_current_cell_type()
{
	int current = tabWidget_mapInfo->currentIndex();
	switch (current)
	{
		case 0 : return Dart_Cell;
		case 1 : return Vertex_Cell;
		case 2 : return Edge_Cell;
		case 3 : return Face_Cell;
		case 4 : return Volume_Cell;
		default: return Dart_Cell;
	}
}

void ControlDock_MapTab::set_selected_map(const QString& map_name)
{
	QList<QListWidgetItem*> items = list_maps->findItems(map_name, Qt::MatchExactly);
	if (!items.empty())
	{
		items[0]->setSelected(true);
		update_selected_map_info();
	}
}





void ControlDock_MapTab::selected_map_changed()
{
	if (!updating_ui_)
	{
		MapHandlerGen* old = selected_map_;

		if (old)
		{
			disconnect(old, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
			disconnect(old, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
			disconnect(old, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
			disconnect(old, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
			disconnect(old, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
			disconnect(old, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
			disconnect(old, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
			disconnect(old, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		}

		QList<QListWidgetItem*> items = list_maps->selectedItems();
		if (!items.empty())
		{
			QString selected_map_name = items[0]->text();
			selected_map_ = schnapps_->get_map(selected_map_name);

			connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
			connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
			connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
			connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
			connect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
			connect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
			connect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
			connect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		}
		else
			selected_map_ = nullptr;

		update_selected_map_info();
		schnapps_->notify_selected_map_changed(old, selected_map_);
	}
}

void ControlDock_MapTab::duplicate_current_map_clicked()
{
	if (!updating_ui_)
	{
//		if (selected_map_)
//			schnapps_->duplicate_map(selected_map_->get_name(), true);
	}
}

void ControlDock_MapTab::remove_current_map_clicked()
{
	if (!updating_ui_)
	{
		if (selected_map_)
			schnapps_->remove_map(selected_map_->get_name());
	}
}

void ControlDock_MapTab::show_bb_changed(bool b)
{
	if (!updating_ui_)
	{
		if (selected_map_)
			selected_map_->set_show_bb(b);
	}
}

void ControlDock_MapTab::bb_vertex_attribute_changed(int index)
{
	if (!updating_ui_)
		selected_map_->set_bb_vertex_attribute(combo_bbVertexAttribute->currentText());
}

void ControlDock_MapTab::vertex_attribute_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_)
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

void ControlDock_MapTab::cells_set_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_)
	{
		if (selected_map_)
		{
			CellType ct = get_current_cell_type();
			CellsSetGen* cs = selected_map_->get_cells_set(ct, item->text());
			cs->set_mutually_exclusive(item->checkState() == Qt::Checked);
			selected_map_->update_mutually_exclusive_cells_sets(ct);
		}
	}
}

void ControlDock_MapTab::add_cells_set()
{
	if (!updating_ui_)
	{
		if (selected_map_)
		{
			CellType ct = get_current_cell_type();
			QString set_name = QString::fromStdString(cell_type_name(ct)) + QString("_set_") + QString::number(CellsSetGen::cells_set_count_);
			selected_map_->add_cells_set(ct, set_name);
		}
	}
}

void ControlDock_MapTab::remove_cells_set()
{
	if (!updating_ui_)
	{
		if (selected_map_)
		{
			CellType ct = get_current_cell_type();
			QListWidget* list_selectors = nullptr;
			switch (ct) {
				case CellType::Dart_Cell:
					list_selectors = list_dartSelectors; break;
				case CellType::Vertex_Cell:
					list_selectors = list_vertexSelectors; break;
				case CellType::Edge_Cell:
					list_selectors = list_edgeSelectors; break;
				case CellType::Face_Cell:
					list_selectors = list_faceSelectors; break;
				case CellType::Volume_Cell:
					list_selectors = list_volumeSelectors; break;
				default:
					list_selectors = nullptr; break;
			}
			if (!list_selectors)
				return;

			for (const QListWidgetItem* item : list_selectors->selectedItems())
				selected_map_->remove_cells_set(ct, item->text());
		}
	}
}

void ControlDock_MapTab::remove_attribute()
{
	if (!updating_ui_)
	{
		if (selected_map_)
		{
			CellType ct = get_current_cell_type();
			QListWidget* list_attributes = nullptr;
			switch (ct) {
				case CellType::Dart_Cell:
					list_attributes = list_dartAttributes; break;
				case CellType::Vertex_Cell:
					list_attributes = list_vertexAttributes; break;
				case CellType::Edge_Cell:
					list_attributes = list_edgeAttributes; break;
				case CellType::Face_Cell:
					list_attributes = list_faceAttributes; break;
				case CellType::Volume_Cell:
					list_attributes = list_volumeAttributes; break;
				default:
					list_attributes = nullptr; break;
			}
			if (!list_attributes)
				return;

			for (const QListWidgetItem* item : list_attributes->selectedItems())
				selected_map_->remove_attribute(ct, item->text());
		}
	}
}





void ControlDock_MapTab::map_added(MapHandlerGen* m)
{
	updating_ui_ = true;
	list_maps->addItem(m->get_name());
	updating_ui_ = false;
}

void ControlDock_MapTab::map_removed(MapHandlerGen* m)
{
	if (selected_map_ == m)
	{
//		selected_selector_[DART] = nullptr;
//		foreach(QListWidgetItem* item, list_dartSelectors->selectedItems())
//			item->setSelected(false);
//		selected_selector_[VERTEX] = nullptr;
//		foreach(QListWidgetItem* item, list_vertexSelectors->selectedItems())
//			item->setSelected(false);
//		selected_selector_[EDGE] = nullptr;
//		foreach(QListWidgetItem* item, list_edgeSelectors->selectedItems())
//			item->setSelected(false);
//		selected_selector_[FACE] = nullptr;
//		foreach(QListWidgetItem* item, list_faceSelectors->selectedItems())
//			item->setSelected(false);
//		selected_selector_[VOLUME] = nullptr;
//		foreach(QListWidgetItem* item, list_volumeSelectors->selectedItems())
//			item->setSelected(false);

		selected_map_ = nullptr;
	}

	QList<QListWidgetItem*> items = list_maps->findItems(m->get_name(), Qt::MatchExactly);
	if (!items.empty())
	{
		updating_ui_ = true;
		delete items[0];
		updating_ui_ = false;
	}
}





void ControlDock_MapTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_bb_vertex_attribute_changed(const QString& name)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_connectivity_changed()
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_cells_set_added(CellType ct, const QString& name)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_cells_set_removed(CellType ct, const QString& name)
{
	update_selected_map_info();
}


void ControlDock_MapTab::update_selected_map_info()
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
		check_drawBB->setChecked(selected_map_->get_show_bb());

		const uint32 nb_d = selected_map_->nb_cells(Dart_Cell);
		label_dartNbCells->setText(QString::number(nb_d));

		if (selected_map_->is_embedded(Dart_Cell))
		{
			const auto& container = selected_map_->const_attribute_container(Dart_Cell);
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_dartAttributes->addItem(name);
			}
		}

		selected_map_->foreach_cells_set(Dart_Cell, [&] (CellsSetGen* cells_set)
		{
			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_dartSelectors);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			if (cells_set->is_mutually_exclusive())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
		});

		const uint32 nb_v = selected_map_->nb_cells(Vertex_Cell);
		label_vertexNbCells->setText(QString::number(nb_v));

		if (selected_map_->is_embedded(Vertex_Cell))
		{
			const auto& container = selected_map_->const_attribute_container(Vertex_Cell);
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
					if (selected_map_->get_bb_vertex_attribute_name() == name)
						combo_bbVertexAttribute->setCurrentIndex(bb_index);
					++bb_index;
				}
				if (selected_map_->get_vbo(name))
					item->setCheckState(Qt::Checked);
				else
					item->setCheckState(Qt::Unchecked);
			}
		}

		selected_map_->foreach_cells_set(Vertex_Cell, [&] (CellsSetGen* cells_set)
		{
			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_vertexSelectors);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			if (cells_set->is_mutually_exclusive())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
		});

		const uint32 nb_e = selected_map_->nb_cells(Edge_Cell);
		label_edgeNbCells->setText(QString::number(nb_e));

		if (selected_map_->is_embedded(Edge_Cell))
		{
			const auto& container = selected_map_->const_attribute_container(Edge_Cell);
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_edgeAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

		selected_map_->foreach_cells_set(Edge_Cell, [&] (CellsSetGen* cells_set)
		{
			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_edgeSelectors);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			if (cells_set->is_mutually_exclusive())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
		});

		const uint32 nb_f = selected_map_->nb_cells(Face_Cell);
		label_faceNbCells->setText(QString::number(nb_f));

		if (selected_map_->is_embedded(Face_Cell))
		{
			const auto& container = selected_map_->const_attribute_container(Face_Cell);
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_faceAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

		selected_map_->foreach_cells_set(Face_Cell, [&] (CellsSetGen* cells_set)
		{
			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_faceSelectors);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			if (cells_set->is_mutually_exclusive())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
		});

		const uint32 nb_vol = selected_map_->nb_cells(Volume_Cell);
		label_volumeNbCells->setText(QString::number(nb_vol));

		if (selected_map_->is_embedded(Volume_Cell))
		{
			const auto& container = selected_map_->const_attribute_container(Volume_Cell);
			const std::vector<std::string>& names = container.names();
//			const std::vector<std::string>& type_names = container.type_names();
			for (std::size_t i = 0u; i < names.size(); ++i)
			{
				QString name = QString::fromStdString(names[i]);
//				QString type = QString::fromStdString(type_names[i]);
				list_volumeAttributes->addItem(name /*+ " (" + type + ")"*/);
			}
		}

		selected_map_->foreach_cells_set(Volume_Cell, [&] (CellsSetGen* cells_set)
		{
			QListWidgetItem* item = new QListWidgetItem(cells_set->get_name(), list_volumeSelectors);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			if (cells_set->is_mutually_exclusive())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
		});
	}

	updating_ui_ = false;
}

} // namespace schnapps
