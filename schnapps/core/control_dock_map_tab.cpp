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
#include <schnapps/core/map_handler.h>

namespace schnapps
{

ControlDock_MapTab::ControlDock_MapTab(SCHNApps* s) :
	schnapps_(s),
	selected_map_(NULL),
	updating_ui_(false)
{
//	for(unsigned int i = 0; i < NB_ORBITS; ++i)
//		selected_selector_[i] = NULL;

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
	if (map_name == QString("NONE"))
	{
		QList<QListWidgetItem*> items = list_maps->selectedItems();
		if (!items.empty())
			items[0]->setSelected(false);
		selected_map_ = NULL;
		update_selected_map_info();
		return;
	}

	QList<QListWidgetItem *> items = list_maps->findItems(map_name, Qt::MatchExactly);
	if (!items.empty())
		items[0]->setSelected(true);
}





void ControlDock_MapTab::selected_map_changed()
{
	if (!updating_ui_)
	{
		MapHandlerGen* old = selected_map_;

		if (old)
		{
			disconnect(old, SIGNAL(attribute_added(unsigned int, const QString&)), this, SLOT(selected_map_attribute_added(unsigned int, const QString&)));
			disconnect(old, SIGNAL(VBO_added(Utils::VBO*)), this, SLOT(selected_map_VBO_added(Utils::VBO*)));
			disconnect(old, SIGNAL(VBO_removed(Utils::VBO*)), this, SLOT(selected_map_VBO_removed(Utils::VBO*)));
			disconnect(old, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
//			disconnect(old, SIGNAL(cell_selector_added(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_added(unsigned int, const QString&)));
//			disconnect(old, SIGNAL(cell_selector_removed(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_removed(unsigned int, const QString&)));
		}

		QList<QListWidgetItem*> items = list_maps->selectedItems();
		if (!items.empty())
		{
			QString selected_map_name = items[0]->text();
			selected_map_ = schnapps_->get_map(selected_map_name);

			connect(selected_map_, SIGNAL(attribute_added(unsigned int, const QString&)), this, SLOT(selected_map_attribute_added(unsigned int, const QString&)));
			connect(selected_map_, SIGNAL(VBO_added(Utils::VBO*)), this, SLOT(selected_map_VBO_added(Utils::VBO*)));
			connect(selected_map_, SIGNAL(VBO_removed(Utils::VBO*)), this, SLOT(selected_map_VBO_removed(Utils::VBO*)));
			connect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
//			connect(selected_map_, SIGNAL(cell_selector_added(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_added(unsigned int, const QString&)));
//			connect(selected_map_, SIGNAL(cell_selector_removed(unsigned int, const QString&)), this, SLOT(selected_map_cell_selector_removed(unsigned int, const QString&)));

//			for(unsigned int i = 0; i < NB_ORBITS; ++i)
//				selected_selector_[i] = NULL;
		}
		else
			selected_map_ = NULL;

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
//	if (!updating_ui_)
//	{
//		if (item->checkState() == Qt::Checked)
//		{
//			Utils::VBO* vbo = selected_map_->create_VBO(item->text());
//			if (!vbo)
//			{
//				updating_ui_ = true;
//				item->setCheckState(Qt::Unchecked);
//				updating_ui_ = false;
//			}
//		}
//		else
//			selected_map_->delete_VBO(item->text());
//	}
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
//					selected_selector_[orbit] = NULL;

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
	QList<QListWidgetItem*> items = list_maps->findItems(m->get_name(), Qt::MatchExactly);
	if(!items.empty())
	{
		updating_ui_ = true;

		delete items[0];
		if (schnapps_->get_selected_map() == m)
		{
//			selected_selector_[DART] = NULL;
//			foreach(QListWidgetItem* item, list_dartSelectors->selectedItems())
//				item->setSelected(false);
//			selected_selector_[VERTEX] = NULL;
//			foreach(QListWidgetItem* item, list_vertexSelectors->selectedItems())
//				item->setSelected(false);
//			selected_selector_[EDGE] = NULL;
//			foreach(QListWidgetItem* item, list_edgeSelectors->selectedItems())
//				item->setSelected(false);
//			selected_selector_[FACE] = NULL;
//			foreach(QListWidgetItem* item, list_faceSelectors->selectedItems())
//				item->setSelected(false);
//			selected_selector_[VOLUME] = NULL;
//			foreach(QListWidgetItem* item, list_volumeSelectors->selectedItems())
//				item->setSelected(false);
		}

		updating_ui_ = false;
	}
}





void ControlDock_MapTab::selected_map_attribute_added(unsigned int orbit, const QString& name)
{
	update_selected_map_info();
}

void ControlDock_MapTab::selected_map_bb_vertex_attribute_changed(const QString& name)
{
	update_selected_map_info();
}

//void ControlDock_MapTab::selected_map_VBO_added(Utils::VBO* vbo)
//{
//	update_selected_map_info();
//}

//void ControlDock_MapTab::selected_map_VBO_removed(Utils::VBO* vbo)
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
			unsigned int nb_d = map2->nb_darts();
			label_dartNbCells->setText(QString::number(nb_d));

			unsigned int nb_v = map2->nb_cells<CMap2::Vertex::ORBIT>();
			label_vertexNbCells->setText(QString::number(nb_v));

			unsigned int nb_e = map2->nb_cells<CMap2::Edge::ORBIT>();
			label_edgeNbCells->setText(QString::number(nb_e));

			unsigned int nb_f = map2->nb_cells<CMap2::Face::ORBIT>();
			label_faceNbCells->setText(QString::number(nb_f));

			unsigned int nb_vol = map2->nb_cells<CMap2::Volume::ORBIT>();
			label_volumeNbCells->setText(QString::number(nb_vol));
		}

//		for (unsigned int orbit = DART; orbit <= VOLUME; ++orbit)
//		{
//			check_drawBB->setChecked(selected_map_->isBBshown());

//			unsigned int nbc = m->getNbCells(orbit);

//			QListWidget* selectorList = NULL;

//			switch (orbit)
//			{
//				case DART : {
//					label_dartNbCells->setText(QString::number(nbc));
//					selectorList = list_dartSelectors;
//					break;
//				}
//				case VERTEX : {
//					label_vertexNbCells->setText(QString::number(nbc));
//					selectorList = list_vertexSelectors;
//					break;
//				}
//				case EDGE : {
//					label_edgeNbCells->setText(QString::number(nbc));
//					selectorList = list_edgeSelectors;
//					break;
//				}
//				case FACE : {
//					label_faceNbCells->setText(QString::number(nbc));
//					selectorList = list_faceSelectors;
//					break;
//				}
//				case VOLUME : {
//					label_volumeNbCells->setText(QString::number(nbc));
//					selectorList = list_volumeSelectors;
//					break;
//				}
//			}

//			foreach (CellSelectorGen* cs, selected_map_->get_cell_selector_set(orbit).values())
//			{
//				QListWidgetItem* item = new QListWidgetItem(cs->get_name(), selectorList);
//				item->setFlags(item->flags() | Qt::ItemIsEditable);
//				if (selected_selector_[orbit] == cs)
//					item->setSelected(true);
//				if (cs->is_mutually_exclusive())
//					item->setCheckState(Qt::Checked);
//				else
//					item->setCheckState(Qt::Unchecked);
//			}

//			if (m->is_embedded(orbit))
//			{
//				AttributeContainer& cont = m->get_attribute_container(orbit);
//				std::vector<std::string> names;
//				std::vector<std::string> types;
//				cont.getAttributesNames(names);
//				cont.getAttributesTypes(types);
//				unsigned int idx = 1;
//				for(unsigned int i = 0; i < names.size(); ++i)
//				{
//					QString name = QString::fromStdString(names[i]);
//					QString type = QString::fromStdString(types[i]);
//					switch (orbit)
//					{
//						case DART : {
//							list_dartAttributes->addItem(name + " (" + type + ")");
//						} break;
//						case VERTEX : {
//							QListWidgetItem* item = new QListWidgetItem(name /*+ " (" + type + ")"*/, list_vertexAttributes);
//							if (type == vec3_type_name)
//							{
//								combo_bbVertexAttribute->addItem(name);
//								if (selected_map_->getBBVertexAttributeName() == name)
//									combo_bbVertexAttribute->setCurrentIndex(idx);
//								++idx;
//							}
//							if (selected_map_->getVBO(name))
//								item->setCheckState(Qt::Checked);
//							else
//								item->setCheckState(Qt::Unchecked);
////							item->setToolTip(QString("Check for VBO"));
//						} break;
//						case EDGE : {
//							list_edgeAttributes->addItem(name + " (" + type + ")");
//						} break;
//						case FACE : {
//							list_faceAttributes->addItem(name + " (" + type + ")");
//						} break;
//						case VOLUME : {
//							list_volumeAttributes->addItem(name + " (" + type + ")");
//						} break;
//					}
//				}
//			}
//		}
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
