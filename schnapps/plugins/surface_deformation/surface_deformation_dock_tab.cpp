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
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	connect(combo_positionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(position_attribute_changed(int)));
	connect(combo_freeVertexSet, SIGNAL(currentIndexChanged(int)), this, SLOT(free_vertex_set_changed(int)));
	connect(combo_handleVertexSet, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_vertex_set_changed(int)));
	connect(button_startStop, SIGNAL(clicked()), this, SLOT(start_stop_button_clicked()));

	MapHandlerGen* smap = schnapps_->get_selected_map();
	if (smap && smap->dimension() == 2)
	{
		selected_map_ = static_cast<CMap2Handler*>(smap);
		connect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		connect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

SurfaceDeformation_DockTab::~SurfaceDeformation_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::position_attribute_changed(int index)
{
	if (!updating_ui_)
		plugin_->set_position_attribute(schnapps_->get_selected_map(), combo_positionAttribute->currentText(), false);
}

void SurfaceDeformation_DockTab::free_vertex_set_changed(int index)
{
	if (!updating_ui_)
	{
		CellsSetGen* cs = selected_map_->get_cells_set(Vertex_Cell, combo_freeVertexSet->currentText());
		plugin_->set_free_vertex_set(schnapps_->get_selected_map(), cs, false);
	}
}

void SurfaceDeformation_DockTab::handle_vertex_set_changed(int index)
{
	if (!updating_ui_)
	{
		CellsSetGen* cs = selected_map_->get_cells_set(Vertex_Cell, combo_handleVertexSet->currentText());
		plugin_->set_handle_vertex_set(schnapps_->get_selected_map(), cs, false);
	}
}

void SurfaceDeformation_DockTab::start_stop_button_clicked()
{
	if (!updating_ui_)
		plugin_->start_stop(schnapps_->get_selected_map(), true);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::selected_view_changed(View* old, View* cur)
{
	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void SurfaceDeformation_DockTab::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	if (cur->dimension() == 2)
	{
		selected_map_ = static_cast<CMap2Handler*>(cur);
		connect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		connect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}
	else
		selected_map_ = nullptr;

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from MapHandlerGen signals
/*****************************************************************************/

void SurfaceDeformation_DockTab::selected_map_cells_set_added(CellType ct, const QString& name)
{
	updating_ui_ = true;
	if (ct == Vertex_Cell)
	{
		combo_freeVertexSet->addItem(name);
		combo_handleVertexSet->addItem(name);
	}
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::selected_map_cells_set_removed(CellType ct, const QString& name)
{
	if (ct == Vertex_Cell)
	{
		int index = combo_freeVertexSet->findText(name);
		if (index > 0)
			combo_freeVertexSet->removeItem(index);
		index = combo_handleVertexSet->findText(name);
		if (index > 0)
			combo_handleVertexSet->removeItem(index);
	}
}

void SurfaceDeformation_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	if (orbit == CMap2::Vertex::ORBIT)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const CMap2* map2 = selected_map_->get_map();
		const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
			combo_positionAttribute->addItem(name);
	}
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		int index = combo_positionAttribute->findText(name, Qt::MatchExactly);
		if (index > 0)
			combo_positionAttribute->removeItem(index);
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

void SurfaceDeformation_DockTab::set_free_vertex_set(CellsSetGen* cs)
{
	updating_ui_ = true;
	if (cs && cs->get_cell_type() == Vertex_Cell)
	{
		int index = combo_freeVertexSet->findText(cs->get_name());
		if (index > 0)
			combo_freeVertexSet->setCurrentIndex(index);
		else
			combo_freeVertexSet->setCurrentIndex(0);
	}
	else
		combo_freeVertexSet->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceDeformation_DockTab::set_handle_vertex_set(CellsSetGen* cs)
{
	updating_ui_ = true;
	if (cs && cs->get_cell_type() == Vertex_Cell)
	{
		int index = combo_handleVertexSet->findText(cs->get_name());
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
	MapHandlerGen* map = schnapps_->get_selected_map();

	if (!map)
		return;

	const MapParameters& p = plugin_->get_parameters(map);

	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_positionAttribute->addItem("- select attribute -");

	combo_freeVertexSet->clear();
	combo_freeVertexSet->addItem("- select set -");

	combo_handleVertexSet->clear();
	combo_handleVertexSet->addItem("- select set -");

	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	const MapHandlerGen::ChunkArrayContainer<uint32>* container = map->attribute_container(CellType::Vertex_Cell);
	const std::vector<std::string>& names = container->names();
	const std::vector<std::string>& type_names = container->type_names();

	unsigned int i = 1;
	for (std::size_t j = 0u; j < names.size(); ++j)
	{
		QString name = QString::fromStdString(names[j]);
		QString type = QString::fromStdString(type_names[j]);
		if (type == vec3_type_name)
		{
			combo_positionAttribute->addItem(name);
			if (p.get_position_attribute().is_valid() && p.get_position_attribute_name() == name)
				combo_positionAttribute->setCurrentIndex(i);
			++i;
		}
	}

	CellsSetGen* fvs = p.get_free_vertex_set();
	CellsSetGen* hvs = p.get_handle_vertex_set();

	i = 1;
	map->foreach_cells_set(Vertex_Cell, [&] (CellsSetGen* cells_set)
	{
		combo_freeVertexSet->addItem(cells_set->get_name());
		if (cells_set == fvs)
			combo_freeVertexSet->setCurrentIndex(i);

		combo_handleVertexSet->addItem(cells_set->get_name());
		if (cells_set == hvs)
			combo_handleVertexSet->setCurrentIndex(i);

		++i;
	});

	button_startStop->setText(p.get_initialized() ? "Stop" : "Start");

	updating_ui_ = false;
}

} // namespace plugin_surface_deformation

} // namespace schnapps
