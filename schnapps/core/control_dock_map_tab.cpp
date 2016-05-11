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
//	for(unsigned int i = 0; i < NB_ORBITS; ++i)
//		selected_selector_[i] = nullptr;

	setupUi(this);

	// connect UI signals
	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(button_duplicate, SIGNAL(clicked()), this, SLOT(duplicate_current_map_clicked()));
	connect(button_remove, SIGNAL(clicked()), this, SLOT(remove_current_map_clicked()));

	connect(check_drawBB, SIGNAL(toggled(bool)), this, SLOT(show_bb_changed(bool)));
	connect(combo_bbVertexAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(bb_vertex_attribute_changed(int)));
	connect(list_vertexAttributes, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vertex_attribute_check_state_changed(QListWidgetItem*)));

//	connect(tabWidget_mapInfo, SIGNAL(currentChanged(int)), this, SLOT(selected_selector_changed()));

//	connect(list_dartSelectors, SIGNAL(itemSelectionChanged()), this, SLOT(selected_selector_changed()));
//	connect(list_dartSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selector_check_state_changed(QListWidgetItem*)));
//	connect(button_dartAddSelector, SIGNAL(clicked()), this, SLOT(add_selector()));
//	connect(button_dartRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_selector()));

//	connect(list_vertexSelectors, SIGNAL(itemSelectionChanged()), this, SLOT(selected_selector_changed()));
//	connect(list_vertexSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selector_check_state_changed(QListWidgetItem*)));
//	connect(button_vertexAddSelector, SIGNAL(clicked()), this, SLOT(add_selector()));
//	connect(button_vertexRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_selector()));

//	connect(list_edgeSelectors, SIGNAL(itemSelectionChanged()), this, SLOT(selected_selector_changed()));
//	connect(list_edgeSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selector_check_state_changed(QListWidgetItem*)));
//	connect(button_edgeAddSelector, SIGNAL(clicked()), this, SLOT(add_selector()));
//	connect(button_edgeRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_selector()));

//	connect(list_faceSelectors, SIGNAL(itemSelectionChanged()), this, SLOT(selected_selector_changed()));
//	connect(list_faceSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selector_check_state_changed(QListWidgetItem*)));
//	connect(button_faceAddSelector, SIGNAL(clicked()), this, SLOT(add_selector()));
//	connect(button_faceRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_selector()));

//	connect(list_volumeSelectors, SIGNAL(itemSelectionChanged()), this, SLOT(selected_selector_changed()));
//	connect(list_volumeSelectors, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(selector_check_state_changed(QListWidgetItem*)));
//	connect(button_volumeAddSelector, SIGNAL(clicked()), this, SLOT(add_selector()));
//	connect(button_volumeRemoveSelector, SIGNAL(clicked()), this, SLOT(remove_selector()));

	// connect SCHNApps signals
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
}

//unsigned int ControlDock_MapTab::get_current_orbit()
//{
//	int current = tabWidget_mapInfo->currentIndex();
//	switch (current)
//	{
//		case 0 : return DART; break;
//		case 1 : return VERTEX; break;
//		case 2 : return EDGE; break;
//		case 3 : return FACE; break;
//		case 4 : return VOLUME; break;
//	}
//	return DART;
//}

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
			disconnect(old, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
			disconnect(old, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
			disconnect(old, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
//			disconnect(old, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
//			disconnect(old, SIGNAL(cell_selector_added(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_added(unsigned int, const QString&)));
//			disconnect(old, SIGNAL(cell_selector_removed(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_removed(unsigned int, const QString&)));
		}

		QList<QListWidgetItem*> items = list_maps->selectedItems();
		if (!items.empty())
		{
			QString selected_map_name = items[0]->text();
			selected_map_ = schnapps_->get_map(selected_map_name);

			connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
			connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
			connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
			connect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
//			connect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
//			connect(selected_map_, SIGNAL(cell_selector_added(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_added(unsigned int, const QString&)));
//			connect(selected_map_, SIGNAL(cell_selector_removed(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_removed(unsigned int, const QString&)));

//			for(unsigned int i = 0; i < NB_ORBITS; ++i)
//				selected_selector_[i] = nullptr;
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
//		if (selected_map_)
//			schnapps_->remove_map(selected_map_->get_name());
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

//void ControlDock_MapTab::selected_selector_changed()
//{
//	if (!updating_ui_)
//	{
//		if (selected_map_)
//		{
//			QList<QListWidgetItem*> items;
//			unsigned int orbit = get_current_orbit();
//			switch (orbit)
//			{
//				case DART: items = list_dartSelectors->selectedItems(); break;
//				case VERTEX: items = list_vertexSelectors->selectedItems(); break;
//				case EDGE: items = list_edgeSelectors->selectedItems(); break;
//				case FACE: items = list_faceSelectors->selectedItems(); break;
//				case VOLUME: items = list_volumeSelectors->selectedItems(); break;
//			}
//
//			if (!items.empty())
//				selected_selector_[orbit] = selected_map_->get_cell_selector(orbit, items[0]->text());

//			schnapps_->notify_selected_cell_selector_changed(selected_selector_[orbit]);
//		}
//	}
//}

//void ControlDock_MapTab::selector_check_state_changed(QListWidgetItem* item)
//{
//	if (!updating_ui_)
//	{
//		if (selected_map_)
//		{
//			unsigned int orbit = get_current_orbit();
//			CellSelectorGen* cs = selected_map_->get_cell_selector(orbit, item->text());
//			cs->setMutuallyExclusive(item->checkState() == Qt::Checked);
//			selected_map_->update_mutually_exclusive_selectors(orbit);
//		}
//	}
//}

//void ControlDock_MapTab::add_selector()
//{
//	if (!updating_ui_)
//	{
//		if (selected_map_)
//			selected_map_->add_cell_selector(get_current_orbit());
//	}
//}

//void ControlDock_MapTab::remove_selector()
//{
//	if (!updating_ui_)
//	{
//		if (selected_map_)
//		{
//			QList<QListWidgetItem*> items;
//			unsigned int orbit = get_current_orbit();
//			switch (orbit)
//			{
//				case DART: items = list_dartSelectors->selectedItems(); break;
//				case VERTEX: items = list_vertexSelectors->selectedItems(); break;
//				case EDGE: items = list_edgeSelectors->selectedItems(); break;
//				case FACE: items = list_faceSelectors->selectedItems(); break;
//				case VOLUME: items = list_volumeSelectors->selectedItems(); break;
//			}

//			if (!items.empty())
//			{
//				if (selected_selector_[orbit]->get_name() == items[0]->text())
//					selected_selector_[orbit] = nullptr;

//				selected_map_->remove_cell_selector(orbit, items[0]->text());

//				schnapps_->notify_selected_cell_selector_changed(selected_selector_[orbit]);
//			}
//		}
//	}
//}





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

//void ControlDock_MapTab::selected_map_connectivity_changed()
//{
//	update_selected_map_info();
//}

//void ControlDock_MapTab::selected_map_cell_selector_added(unsigned int orbit, const QString& name)
//{
//	update_selected_map_info();
//}

//void ControlDock_MapTab::selected_map_cell_selector_removed(unsigned int orbit, const QString& name)
//{
//	update_selected_map_info();
//}





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

	if (selected_map_)
	{
		check_drawBB->setChecked(selected_map_->get_show_bb());

		const MapBaseData* map = selected_map_->get_map();
		const CMap2* map2 = dynamic_cast<const CMap2*>(map);
		const CMap3* map3 = dynamic_cast<const CMap3*>(map);

		if (map2)
		{
			const MapHandler<CMap2>* map2h = dynamic_cast<const MapHandler<CMap2>*>(selected_map_);

			unsigned int nb_d = map2->nb_darts();
			label_dartNbCells->setText(QString::number(nb_d));
//			foreach (CellSelectorGen* cs, selected_map_->get_cell_selector_set(CMap2::CDart::ORBIT).values())
//			{
//				QListWidgetItem* item = new QListWidgetItem(cs->get_name(), list_dartSelectors);
//				item->setFlags(item->flags() | Qt::ItemIsEditable);
//				if (selected_selector_[orbit] == cs)
//					item->setSelected(true);
//				if (cs->is_mutually_exclusive())
//					item->setCheckState(Qt::Checked);
//				else
//					item->setCheckState(Qt::Unchecked);
//			}
			if (map2->is_embedded<CMap2::CDart::ORBIT>())
			{
				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->get_attribute_container<CMap2::CDart::ORBIT>();
				const std::vector<std::string>& names = container.get_names();
				const std::vector<std::string>& type_names = container.get_type_names();
				for(std::size_t i = 0u; i < names.size(); ++i)
				{
					QString name = QString::fromStdString(names[i]);
					QString type = QString::fromStdString(type_names[i]);
					list_dartAttributes->addItem(name + " (" + type + ")");
				}
			}

			unsigned int nb_v = map2->nb_cells<CMap2::Vertex::ORBIT>();
			label_vertexNbCells->setText(QString::number(nb_v));
//			foreach (CellSelectorGen* cs, selected_map_->get_cell_selector_set(CMap2::Vertex::ORBIT).values())
//			{
//				QListWidgetItem* item = new QListWidgetItem(cs->get_name(), list_vertexSelectors);
//				item->setFlags(item->flags() | Qt::ItemIsEditable);
//				if (selected_selector_[orbit] == cs)
//					item->setSelected(true);
//				if (cs->is_mutually_exclusive())
//					item->setCheckState(Qt::Checked);
//				else
//					item->setCheckState(Qt::Unchecked);
//			}
			if (map2->is_embedded<CMap2::Vertex::ORBIT>())
			{
				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->get_attribute_container<CMap2::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.get_names();
				const std::vector<std::string>& type_names = container.get_type_names();
				unsigned int bb_index = 1;
				for(std::size_t i = 0u; i < names.size(); ++i)
				{
					QString name = QString::fromStdString(names[i]);
					QString type = QString::fromStdString(type_names[i]);
					QListWidgetItem* item = new QListWidgetItem(name /*+ " (" + type + ")"*/, list_vertexAttributes);
					if (type == vec3_type_name)
					{
						combo_bbVertexAttribute->addItem(name);
						if (map2h->get_bb_vertex_attribute_name() == name)
							combo_bbVertexAttribute->setCurrentIndex(bb_index);
						++bb_index;
					}
					if (selected_map_->get_vbo(name))
						item->setCheckState(Qt::Checked);
					else
						item->setCheckState(Qt::Unchecked);
				}
			}

			unsigned int nb_e = map2->nb_cells<CMap2::Edge::ORBIT>();
			label_edgeNbCells->setText(QString::number(nb_e));
//			foreach (CellSelectorGen* cs, selected_map_->get_cell_selector_set(CMap2::Edge::ORBIT).values())
//			{
//				QListWidgetItem* item = new QListWidgetItem(cs->get_name(), list_edgeSelectors);
//				item->setFlags(item->flags() | Qt::ItemIsEditable);
//				if (selected_selector_[orbit] == cs)
//					item->setSelected(true);
//				if (cs->is_mutually_exclusive())
//					item->setCheckState(Qt::Checked);
//				else
//					item->setCheckState(Qt::Unchecked);
//			}
			if (map2->is_embedded<CMap2::Edge::ORBIT>())
			{
				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->get_attribute_container<CMap2::Edge::ORBIT>();
				const std::vector<std::string>& names = container.get_names();
				const std::vector<std::string>& type_names = container.get_type_names();
				for(std::size_t i = 0u; i < names.size(); ++i)
				{
					QString name = QString::fromStdString(names[i]);
					QString type = QString::fromStdString(type_names[i]);
					list_edgeAttributes->addItem(name + " (" + type + ")");
				}
			}

			unsigned int nb_f = map2->nb_cells<CMap2::Face::ORBIT>();
			label_faceNbCells->setText(QString::number(nb_f));
//			foreach (CellSelectorGen* cs, selected_map_->get_cell_selector_set(CMap2::Face::ORBIT).values())
//			{
//				QListWidgetItem* item = new QListWidgetItem(cs->get_name(), list_faceSelectors);
//				item->setFlags(item->flags() | Qt::ItemIsEditable);
//				if (selected_selector_[orbit] == cs)
//					item->setSelected(true);
//				if (cs->is_mutually_exclusive())
//					item->setCheckState(Qt::Checked);
//				else
//					item->setCheckState(Qt::Unchecked);
//			}
			if (map2->is_embedded<CMap2::Face::ORBIT>())
			{
				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->get_attribute_container<CMap2::Face::ORBIT>();
				const std::vector<std::string>& names = container.get_names();
				const std::vector<std::string>& type_names = container.get_type_names();
				for(std::size_t i = 0u; i < names.size(); ++i)
				{
					QString name = QString::fromStdString(names[i]);
					QString type = QString::fromStdString(type_names[i]);
					list_faceAttributes->addItem(name + " (" + type + ")");
				}
			}

			unsigned int nb_vol = map2->nb_cells<CMap2::Volume::ORBIT>();
			label_volumeNbCells->setText(QString::number(nb_vol));
//			foreach (CellSelectorGen* cs, selected_map_->get_cell_selector_set(CMap2::Volume::ORBIT).values())
//			{
//				QListWidgetItem* item = new QListWidgetItem(cs->get_name(), list_volumeSelectors);
//				item->setFlags(item->flags() | Qt::ItemIsEditable);
//				if (selected_selector_[orbit] == cs)
//					item->setSelected(true);
//				if (cs->is_mutually_exclusive())
//					item->setCheckState(Qt::Checked);
//				else
//					item->setCheckState(Qt::Unchecked);
//			}
			if (map2->is_embedded<CMap2::Volume::ORBIT>())
			{
				const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->get_attribute_container<CMap2::Volume::ORBIT>();
				const std::vector<std::string>& names = container.get_names();
				const std::vector<std::string>& type_names = container.get_type_names();
				for(std::size_t i = 0u; i < names.size(); ++i)
				{
					QString name = QString::fromStdString(names[i]);
					QString type = QString::fromStdString(type_names[i]);
					list_volumeAttributes->addItem(name + " (" + type + ")");
				}
			}
		}
	}
	else
	{
		label_dartNbCells->setText(QString::number(0));
		label_vertexNbCells->setText(QString::number(0));
		label_edgeNbCells->setText(QString::number(0));
		label_faceNbCells->setText(QString::number(0));
		label_volumeNbCells->setText(QString::number(0));
	}

	updating_ui_ = false;
}

} // namespace schnapps
