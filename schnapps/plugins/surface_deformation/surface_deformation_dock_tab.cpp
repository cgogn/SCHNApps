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

#include <schnapps/plugins/surface_deformation/surface_deformation_dock_tab.h>
#include <schnapps/plugins/surface_deformation/surface_deformation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_deformation
{

SurfaceDeformation_DockTab::SurfaceDeformation_DockTab(SCHNApps* s, Plugin_SurfaceDeformation* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap_provider_(nullptr),
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(combo_positionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(position_attribute_changed(int)));
	connect(combo_freeVertexSet, SIGNAL(currentIndexChanged(int)), this, SLOT(free_vertex_set_changed(int)));
	connect(combo_handleVertexSet, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_vertex_set_changed(int)));
	connect(button_startStop, SIGNAL(clicked()), this, SLOT(start_stop_button_clicked()));

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));

	View* v = schnapps_->selected_view();
	connect(v, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(v, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	for (Object* o : v->linked_objects())
		object_linked(o);

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
}

SurfaceDeformation_DockTab::~SurfaceDeformation_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::selected_map_changed()
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(cells_set_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_removed(cgogn::Orbit, const QString&)));
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
		connect(selected_map_, SIGNAL(cells_set_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_removed(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void SurfaceDeformation_DockTab::position_attribute_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_position_attribute(selected_map_, combo_positionAttribute->currentText(), false);
}

void SurfaceDeformation_DockTab::free_vertex_set_changed(int)
{
	if (!updating_ui_ && selected_map_)
	{
		CMap2CellsSet<CMap2::Vertex>* cs = static_cast<CMap2CellsSet<CMap2::Vertex>*>(selected_map_->cells_set(CMap2::Vertex::ORBIT, combo_freeVertexSet->currentText()));
		plugin_->set_free_vertex_set(selected_map_, cs, false);
	}
}

void SurfaceDeformation_DockTab::handle_vertex_set_changed(int)
{
	if (!updating_ui_ && selected_map_)
	{
		CMap2CellsSet<CMap2::Vertex>* cs = static_cast<CMap2CellsSet<CMap2::Vertex>*>(selected_map_->cells_set(CMap2::Vertex::ORBIT, combo_handleVertexSet->currentText()));
		plugin_->set_handle_vertex_set(selected_map_, cs, false);
	}
}

void SurfaceDeformation_DockTab::start_stop_button_clicked()
{
	if (!updating_ui_ && selected_map_)
	{
		const MapParameters& p = plugin_->parameters(selected_map_);
		if (!p.initialized())
			plugin_->initialize(selected_map_, true);
		else
			plugin_->stop(selected_map_, true);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::selected_view_changed(View* old, View* cur)
{
	updating_ui_ = true;
	list_maps->clear();
	updating_ui_ = false;

	if (old)
	{
		disconnect(old, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
		disconnect(old, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	}
	if (cur)
	{
		connect(cur, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
		connect(cur, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
		for (Object* o : cur->linked_objects())
			object_linked(o);
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from View signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::object_linked(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_linked(mh);
}

void SurfaceDeformation_DockTab::map_linked(CMap2Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::object_unlinked(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_unlinked(mh);
}

void SurfaceDeformation_DockTab::map_unlinked(CMap2Handler* mh)
{
	if (selected_map_ == mh)
	{
		disconnect(selected_map_, SIGNAL(cells_set_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_cells_set_removed(cgogn::Orbit, const QString&)));
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
// slots called from MapHandlerGen signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::selected_map_cells_set_added(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		updating_ui_ = true;
		combo_freeVertexSet->addItem(name);
		combo_handleVertexSet->addItem(name);
		updating_ui_ = false;
	}
}

void SurfaceDeformation_DockTab::selected_map_cells_set_removed(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		updating_ui_ = true;
		int index = combo_freeVertexSet->findText(name);
		if (index > 0)
			combo_freeVertexSet->removeItem(index);
		index = combo_handleVertexSet->findText(name);
		if (index > 0)
			combo_handleVertexSet->removeItem(index);
		updating_ui_ = false;
	}
}

void SurfaceDeformation_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map = selected_map_->map();
		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			updating_ui_ = true;
			combo_positionAttribute->addItem(name);
			updating_ui_ = false;
		}
	}
}

void SurfaceDeformation_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		int index = combo_positionAttribute->findText(name, Qt::MatchExactly);
		if (index > 0)
		{
			updating_ui_ = true;
			combo_positionAttribute->removeItem(index);
			updating_ui_ = false;
		}
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void SurfaceDeformation_DockTab::set_position_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_positionAttribute->findText(name);
	if (index > 0)
		combo_positionAttribute->setCurrentIndex(index);
	else
		combo_positionAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::set_free_vertex_set(CMap2CellsSet<CMap2::Vertex>* cs)
{
	updating_ui_ = true;
	if (cs)
	{
		int index = combo_freeVertexSet->findText(cs->name());
		if (index > 0)
			combo_freeVertexSet->setCurrentIndex(index);
		else
			combo_freeVertexSet->setCurrentIndex(0);
	}
	else
		combo_freeVertexSet->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::set_handle_vertex_set(CMap2CellsSet<CMap2::Vertex>* cs)
{
	updating_ui_ = true;
	if (cs)
	{
		int index = combo_handleVertexSet->findText(cs->name());
		if (index > 0)
			combo_handleVertexSet->setCurrentIndex(index);
		else
			combo_handleVertexSet->setCurrentIndex(0);
	}
	else
		combo_handleVertexSet->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::set_deformation_initialized(bool b)
{
	updating_ui_ = true;
	button_startStop->setText(b ? "Stop" : "Start");
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::refresh_ui()
{
	CMap2Handler* mh = selected_map_;

	if (!mh)
		return;

	const MapParameters& p = plugin_->parameters(mh);

	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_positionAttribute->addItem("- select attribute -");

	combo_freeVertexSet->clear();
	combo_freeVertexSet->addItem("- select set -");

	combo_handleVertexSet->clear();
	combo_handleVertexSet->addItem("- select set -");

	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	const CMap2::ChunkArrayContainer<uint32>& container = mh->map()->attribute_container<CMap2::Vertex::ORBIT>();
	const std::vector<std::string>& names = container.names();
	const std::vector<std::string>& type_names = container.type_names();

	unsigned int i = 1;
	for (std::size_t j = 0u; j < names.size(); ++j)
	{
		QString name = QString::fromStdString(names[j]);
		QString type = QString::fromStdString(type_names[j]);
		if (type == vec3_type_name)
		{
			combo_positionAttribute->addItem(name);
			if (p.position_attribute().is_valid() && QString::fromStdString(p.position_attribute().name()) == name)
				combo_positionAttribute->setCurrentIndex(int(i));
			++i;
		}
	}

	CMap2CellsSet<CMap2::Vertex>* fvs = p.free_vertex_set();
	CMap2CellsSet<CMap2::Vertex>* hvs = p.handle_vertex_set();

	i = 1;
	mh->foreach_cells_set(CMap2::Vertex::ORBIT, [&] (CMapCellsSetGen* cells_set)
	{
		combo_freeVertexSet->addItem(cells_set->name());
		if (cells_set == fvs)
			combo_freeVertexSet->setCurrentIndex(int(i));

		combo_handleVertexSet->addItem(cells_set->name());
		if (cells_set == hvs)
			combo_handleVertexSet->setCurrentIndex(int(i));

		++i;
	});

	button_startStop->setText(p.initialized() ? "Stop" : "Start");

	updating_ui_ = false;
}

} // namespace plugin_surface_deformation

} // namespace schnapps
