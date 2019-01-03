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

#include <schnapps/plugins/cmap1_provider/cmap1_provider_dock_tab.h>
#include <schnapps/plugins/cmap1_provider/cmap1_provider.h>
#include <schnapps/plugins/cmap1_provider/cmap1_cells_set.h>

namespace schnapps
{

namespace plugin_cmap1_provider
{

CMap1Provider_DockTab::CMap1Provider_DockTab(SCHNApps* s, Plugin_CMap1Provider* p) :
	schnapps_(s),
	plugin_(p),
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

//	connect(button_duplicate, SIGNAL(clicked()), this, SLOT(duplicate_current_map_clicked()));
	connect(button_remove, SIGNAL(clicked()), this, SLOT(remove_current_map_clicked()));

	connect(combo_bbVertexAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(bb_vertex_attribute_changed(int)));
//	connect(combo_obbVertexAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(obb_vertex_attribute_changed(int)));
	connect(list_vertexAttributes, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vertex_attribute_check_state_changed(QListWidgetItem*)));

	connect(list_vertexCellsSets, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cells_set_check_state_changed(QListWidgetItem*)));
	connect(button_vertexAddCellsSet, SIGNAL(clicked()), this, SLOT(add_cells_set()));
	connect(button_vertexRemoveCellsSet, SIGNAL(clicked()), this, SLOT(remove_cells_set()));

	connect(spin_scaling_x, SIGNAL(valueChanged(double)), this, SLOT(scale_object()));
	connect(spin_scaling_y, SIGNAL(valueChanged(double)), this, SLOT(scale_object()));
	connect(spin_scaling_z, SIGNAL(valueChanged(double)), this, SLOT(scale_object()));
}

CMap1Provider_DockTab::~CMap1Provider_DockTab()
{

}

cgogn::Orbit CMap1Provider_DockTab::current_orbit()
{
	return CMap1::Vertex::ORBIT;
}

QString CMap1Provider_DockTab::orbit_name(cgogn::Orbit orbit)
{
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT: return QString("Vertex");
	}
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void CMap1Provider_DockTab::selected_map_changed()
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
//		disconnect(selected_map_, SIGNAL(obb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_obb_vertex_attribute_changed(const QString&)));
		disconnect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		disconnect(selected_map_, SIGNAL(cells_set_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_removed(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_mutually_exclusive_changed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_mutually_exclusive_changed(cgogn::Orbit, const QString&)));
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
//		connect(selected_map_, SIGNAL(obb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_obb_vertex_attribute_changed(const QString&)));
		connect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		connect(selected_map_, SIGNAL(cells_set_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_removed(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(cells_set_mutually_exclusive_changed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_mutually_exclusive_changed(cgogn::Orbit, const QString&)));
	}

	refresh_ui();
}

//void CMap1Provider_DockTab::duplicate_current_map_clicked()
//{
//	if (!updating_ui_ && selected_map_)
//		plugin_->duplicate_map(selected_map_->name());
//}

void CMap1Provider_DockTab::remove_current_map_clicked()
{
	if (!updating_ui_ && selected_map_)
		plugin_->remove_map(selected_map_->name());
}

void CMap1Provider_DockTab::bb_vertex_attribute_changed(int)
{
	if (!updating_ui_ && selected_map_)
		selected_map_->set_bb_vertex_attribute(combo_bbVertexAttribute->currentText());
}

//void CMap1Provider_DockTab::obb_vertex_attribute_changed(int)
//{
//	if (!updating_ui_ && selected_map_)
//		selected_map_->set_obb_vertex_attribute(combo_obbVertexAttribute->currentText());
//}

void CMap1Provider_DockTab::vertex_attribute_check_state_changed(QListWidgetItem* item)
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

void CMap1Provider_DockTab::cells_set_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_ && selected_map_)
	{
		cgogn::Orbit orbit = current_orbit();
		CMap1CellsSetGen* cs = selected_map_->cells_set(orbit, item->text());
		cs->set_mutually_exclusive(item->checkState() == Qt::Checked);
	}
}

void CMap1Provider_DockTab::add_cells_set()
{
	if (!updating_ui_ && selected_map_)
	{
		cgogn::Orbit orbit = current_orbit();
		QString set_name = orbit_name(orbit) + QString("_set_") + QString::number(CMap1CellsSetGen::cells_set_count_);
		selected_map_->add_cells_set(orbit, set_name);
	}
}

void CMap1Provider_DockTab::remove_cells_set()
{
	if (!updating_ui_ && selected_map_)
	{
		cgogn::Orbit orbit = current_orbit();
		QListWidget* list_cells_set = nullptr;
		switch (orbit)
		{
			case CMap1::Vertex::ORBIT: list_cells_set = list_vertexCellsSets; break;
			default: break;
		}
		if (!list_cells_set)
			return;

		for (const QListWidgetItem* item : list_cells_set->selectedItems())
			selected_map_->remove_cells_set(orbit, item->text());
	}
}


/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/



/*****************************************************************************/
// slots called from CMap1Handler signals
/*****************************************************************************/

void CMap1Provider_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT: {
			QListWidgetItem* item = new QListWidgetItem(name, list_vertexAttributes);
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
			const auto* cag = selected_map_->map()->attribute_container<CMap1::Vertex::ORBIT>().get_chunk_array(name.toStdString());
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
		default: break;
	}
	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT: {
			QList<QListWidgetItem*> items = list_vertexAttributes->findItems(name, Qt::MatchExactly);
			if (!items.empty()) delete items[0];
			QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
			const auto* cag = selected_map_->map()->attribute_container<CMap1::Vertex::ORBIT>().get_chunk_array(name.toStdString());
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
		default: break;
	}
	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_bb_vertex_attribute_changed(const QString& name)
{
	updating_ui_ = true;
	int index = combo_bbVertexAttribute->findText(name, Qt::MatchExactly);
	if (index > 0)
		combo_bbVertexAttribute->setCurrentIndex(index);
	updating_ui_ = false;
}

//void CMap1Provider_DockTab::selected_map_obb_vertex_attribute_changed(const QString& name)
//{
//	updating_ui_ = true;
//	int index = combo_obbVertexAttribute->findText(name, Qt::MatchExactly);
//	if (index > 0)
//		combo_obbVertexAttribute->setCurrentIndex(index);
//	updating_ui_ = false;
//}

void CMap1Provider_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	QList<QListWidgetItem*> items = list_vertexAttributes->findItems(QString::fromStdString(vbo->name()), Qt::MatchExactly);
	if (!items.empty())
		items[0]->setCheckState(Qt::Checked);
	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	QList<QListWidgetItem*> items = list_vertexAttributes->findItems(QString::fromStdString(vbo->name()), Qt::MatchExactly);
	if (!items.empty())
		items[0]->setCheckState(Qt::Unchecked);
	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_connectivity_changed()
{
	updating_ui_ = true;

	label_vertexNbCells->setText(QString::number(0));

	if (selected_map_)
	{
		const uint32 nb_v = selected_map_->map()->nb_cells<CMap1::Vertex>();
		label_vertexNbCells->setText(QString::number(nb_v));
	}

	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_cells_set_added(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;

	QListWidgetItem* item = nullptr;
	CMap1CellsSetGen* cs = nullptr;
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT:
			cs = selected_map_->cells_set<CMap1::Vertex>(name);
			if (cs)
				item = new QListWidgetItem(name, list_vertexCellsSets);
			break;
		default:
			break;
	}
	if (item)
	{
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		if (cs->is_mutually_exclusive())
			item->setCheckState(Qt::Checked);
		else
			item->setCheckState(Qt::Unchecked);
	}

	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_cells_set_removed(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;

	QList<QListWidgetItem*> items;
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT:
			items = list_vertexCellsSets->findItems(name, Qt::MatchExactly);
			break;
		default:
			break;
	}
	if (!items.empty())
		delete items[0];

	updating_ui_ = false;
}

void CMap1Provider_DockTab::selected_map_cells_set_mutually_exclusive_changed(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;

	CMap1CellsSetGen* cs = nullptr;
	QList<QListWidgetItem*> items;
	switch (orbit)
	{
		case CMap1::Vertex::ORBIT:
			cs = selected_map_->cells_set<CMap1::Vertex>(name);
			items = list_vertexCellsSets->findItems(name, Qt::MatchExactly);
			break;
		default:
			break;
	}
	if (cs && !items.empty())
		items[0]->setCheckState(cs->is_mutually_exclusive() ? Qt::Checked : Qt::Unchecked);

	updating_ui_ = false;
}

void CMap1Provider_DockTab::scale_object()
{
	if(selected_map_)
		selected_map_->rescale(spin_scaling_x->value(),spin_scaling_y->value(),spin_scaling_z->value());
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void CMap1Provider_DockTab::add_map(CMap1Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void CMap1Provider_DockTab::remove_map(CMap1Handler* mh)
{
	if (selected_map_ == mh)
	{
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(bb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_bb_vertex_attribute_changed(const QString&)));
//		disconnect(selected_map_, SIGNAL(obb_vertex_attribute_changed(const QString&)), this, SLOT(selected_map_obb_vertex_attribute_changed(const QString&)));
		disconnect(selected_map_, SIGNAL(connectivity_changed()), this, SLOT(selected_map_connectivity_changed()));
		disconnect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_mutually_exclusive_changed(CellType, const QString&)), this, SLOT(selected_map_cells_set_mutually_exclusive_changed(CellType, const QString&)));
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

void CMap1Provider_DockTab::refresh_ui()
{
	updating_ui_ = true;

	combo_bbVertexAttribute->clear();
	combo_bbVertexAttribute->addItem("- select attribute -");
//	combo_obbVertexAttribute->clear();
//	combo_obbVertexAttribute->addItem("- select attribute -");
	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	list_vertexAttributes->clear();

	list_vertexCellsSets->clear();

	label_vertexNbCells->setText(QString::number(0));


	if (selected_map_)
	{
		CMap1* map = selected_map_->map();

		const uint32 nb_v = map->nb_cells<CMap1::Vertex>();
		label_vertexNbCells->setText(QString::number(nb_v));

		if (map->is_embedded<CMap1::Vertex>())
		{
			const auto& container = map->attribute_container<CMap1::Vertex::ORBIT>();
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

//					combo_obbVertexAttribute->addItem(name);
//					if (selected_map_->obb_vertex_attribute_name() == name)
//						combo_obbVertexAttribute->setCurrentIndex(bb_index);

					++bb_index;
				}
				if (selected_map_->vbo(name))
					item->setCheckState(Qt::Checked);
				else
					item->setCheckState(Qt::Unchecked);
			}
		}

		selected_map_->foreach_cells_set<CMap1::Vertex>([&] (CMap1CellsSet<CMap1::Vertex>* cells_set)
		{
			QListWidgetItem* item = new QListWidgetItem(cells_set->name(), list_vertexCellsSets);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			if (cells_set->is_mutually_exclusive())
				item->setCheckState(Qt::Checked);
			else
				item->setCheckState(Qt::Unchecked);
		});

		QVector3D s = selected_map_->scale();
		spin_scaling_x->setValue(s[0]);
		spin_scaling_y->setValue(s[1]);
		spin_scaling_z->setValue(s[2]);
	}

	updating_ui_ = false;
}

/*******************************Ò**********************************************/
// internal UI cascading updates
/*****************************************************************************/



} // namespace plugin_cmap1_provider

} // namespace schnapps
