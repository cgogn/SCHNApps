/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Modelisation                                                   *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#include <volume_modelisation.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/core/cmap/cmap3.h>
#include <cgogn/geometry/algos/centroid.h>
#include <cgogn/modeling/algos/tetrahedralization.h>

namespace schnapps
{

namespace plugin_volume_modelisation
{

VolumeModelisationPlugin::VolumeModelisationPlugin()
{
	operations_ = cgogn::make_unique<VolumeOperation>();

	operations_->add_operation("Split 1 to 4",CellType::Volume_Cell, [=](MapHandlerGen* mhg, MapHandlerGen::Attribute_T<VEC3>& pos_attr, const std::vector<cgogn::Dart>& darts) -> std::vector<cgogn::Dart>
	{
		std::vector<cgogn::Dart> res;
		if (mhg && mhg->dimension() == 3)
		{
			res.reserve(darts.size());
			CMap3* map3 = static_cast<CMap3Handler*>(mhg)->get_map();
			CMap3::VertexAttribute<VEC3>& pos3 = static_cast<CMap3::VertexAttribute<VEC3>&>(pos_attr);
			for (auto d : darts)
			{
				const CMap3::Volume w(d);
				auto inserted_vertex_pos = cgogn::geometry::centroid<VEC3>(*map3, w,pos3);
				res.push_back(cgogn::modeling::flip_14(*map3, w).dart);
				cgogn::Dart v(res.back());
				if (!v.is_nil())
					pos_attr[v] = std::move(inserted_vertex_pos);
			}
		}
		return res;
	});

	operations_->add_operation("Delete edge",CellType::Edge_Cell, [=](MapHandlerGen* mhg, MapHandlerGen::Attribute_T<VEC3>& pos_attr, const std::vector<cgogn::Dart>& darts) -> std::vector<cgogn::Dart>
	{
		std::vector<cgogn::Dart> res;
		if (mhg && mhg->dimension() == 3)
		{
			res.reserve(darts.size());
			CMap3* map3 = static_cast<CMap3Handler*>(mhg)->get_map();
			for (auto d : darts)
				res.push_back(map3->delete_edge(CMap3::Edge(d)));
		}
		return res;
	});

	operations_->add_operation("Delete volume",CellType::Volume_Cell, [=](MapHandlerGen* mhg, MapHandlerGen::Attribute_T<VEC3>& pos_attr, const std::vector<cgogn::Dart>& darts) -> std::vector<cgogn::Dart>
	{
		if (mhg && mhg->dimension() == 3)
		{
			CMap3* map3 = static_cast<CMap3Handler*>(mhg)->get_map();
			for (auto d : darts)
				map3->delete_volume(CMap3::Volume(d));
		}
		return std::vector<cgogn::Dart>();
	});


	operations_->add_operation("Unsew volumes",CellType::Face_Cell, [=](MapHandlerGen* mhg, MapHandlerGen::Attribute_T<VEC3>& pos_attr, const std::vector<cgogn::Dart>& darts) -> std::vector<cgogn::Dart>
	{
		if (mhg && mhg->dimension() == 3)
		{
			CMap3* map3 = static_cast<CMap3Handler*>(mhg)->get_map();
			for (auto d : darts)
				map3->unsew_volumes(CMap3::Face(d));
		}
		return std::vector<cgogn::Dart>();
	});

	operations_->add_operation("Merge volumes",CellType::Face_Cell, [=](MapHandlerGen* mhg, MapHandlerGen::Attribute_T<VEC3>& pos_attr, const std::vector<cgogn::Dart>& darts) -> std::vector<cgogn::Dart>
	{
		if (mhg && mhg->dimension() == 3)
		{
			CMap3* map3 = static_cast<CMap3Handler*>(mhg)->get_map();
			for (auto d : darts)
				map3->merge_incident_volumes(CMap3::Face(d));
		}
		return std::vector<cgogn::Dart>();
	});

}

VolumeModelisationPlugin::~VolumeModelisationPlugin()
{}

bool VolumeModelisationPlugin::enable()
{
	docktab_ = cgogn::make_unique<VolumeModelisation_DockTab>(schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, docktab_.get(), "Modelisation3");

	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(current_map_changed(MapHandlerGen*,MapHandlerGen*)));
	update_dock_tab();
	return true;
}

void VolumeModelisationPlugin::disable()
{
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(current_map_changed(MapHandlerGen*,MapHandlerGen*)));
	schnapps_->remove_plugin_dock_tab(this, docktab_.get());
}

void VolumeModelisationPlugin::process_operation()
{
	static uint32 counter = 0u;
	MapHandlerGen* mhg = schnapps_->get_selected_map();
	if (mhg && mhg->dimension() == 3)
	{
		CMap3Handler* mh3 = static_cast<CMap3Handler*>(mhg);
		const MapOperator* op = operations_->get_operator(docktab_->operations_combobox->currentText().toStdString());
		if (!op)
			return;

		QComboBox* cbox = docktab_->get_cell_set_combo_box(op->cell_type_);
		CellsSetGen* csg = mhg->get_cells_set(op->cell_type_, cbox->currentText());
		if (!csg)
			return;

		std::vector<cgogn::Dart> arg;
		arg.reserve(csg->get_nb_cells());
		csg->foreach_cell([&arg](cgogn::Dart d) { arg.push_back(d);});

		auto pos_attribute = mh3->get_attribute<VEC3, CMap3::Vertex::ORBIT>(docktab_->position_comboBox->currentText());
		std::vector<cgogn::Dart> res = op->func_(mhg,pos_attribute, arg);
		res.erase(std::remove(res.begin(), res.end(), cgogn::Dart()), res.end()); // clean NIL darts
		if (!res.empty())
		{
			CellsSetGen* res_csg = mhg->add_cells_set(CellType::Dart_Cell, docktab_->operations_combobox->currentText() + QString("__" + QString::number(counter++)));
			res_csg->select(res);
		}

		mhg->notify_connectivity_change();
	}
}

void VolumeModelisationPlugin::current_map_changed(MapHandlerGen* prev, MapHandlerGen* next)
{
	VolumeModelisation_DockTab* dt = docktab_.get();
	if (prev)
	{
		disconnect(prev, SIGNAL(cells_set_added(CellType,QString)), this, SLOT(current_cells_set_added(CellType,QString)));
		disconnect(prev, SIGNAL(cells_set_removed(CellType,QString)), this, SLOT(current_cells_set_removed(CellType,QString)));
		disconnect(next, SIGNAL(attribute_added(cgogn::Orbit,QString)), this, SLOT(current_map_attribute_added(cgogn::Orbit,QString)));
		disconnect(next, SIGNAL(attribute_removed(cgogn::Orbit,QString)), this, SLOT(current_map_attribute_removed(cgogn::Orbit,QString)));
	}
	if (next)
	{
		connect(next, SIGNAL(cells_set_added(CellType,QString)), this, SLOT(current_cells_set_added(CellType,QString)));
		connect(next, SIGNAL(cells_set_removed(CellType,QString)), this, SLOT(current_cells_set_removed(CellType,QString)));
		connect(next, SIGNAL(attribute_added(cgogn::Orbit,QString)), this, SLOT(current_map_attribute_added(cgogn::Orbit,QString)));
		connect(next, SIGNAL(attribute_removed(cgogn::Orbit,QString)), this, SLOT(current_map_attribute_removed(cgogn::Orbit,QString)));
	}
	update_dock_tab();
}

void VolumeModelisationPlugin::current_cells_set_added(CellType ct, const QString& name)
{
	QComboBox* cb = docktab_->get_cell_set_combo_box(ct);
	cb->addItem(name);
}

void VolumeModelisationPlugin::current_cells_set_removed(CellType ct, const QString& name)
{
	QComboBox* cb = docktab_->get_cell_set_combo_box(ct);
	cb->removeItem(cb->findText(name));
}

void VolumeModelisationPlugin::current_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap3::Vertex::ORBIT)
		docktab_->position_comboBox->addItem(name);
}

void VolumeModelisationPlugin::current_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	if (orbit == CMap3::Vertex::ORBIT)
		docktab_->position_comboBox->removeItem(docktab_->position_comboBox->findText(name));
}

void VolumeModelisationPlugin::update_dock_tab()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map && map->dimension() == 3u)
		schnapps_->enable_plugin_tab_widgets(this);
	else
		schnapps_->disable_plugin_tab_widgets(this);
	docktab_->update(map);
}

void VolumeModelisationPlugin::draw(View* view, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
}

void VolumeModelisationPlugin::draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
}

void VolumeModelisationPlugin::keyPress(View* view, QKeyEvent* event)
{
}

void VolumeModelisationPlugin::keyRelease(View* view, QKeyEvent* event)
{
}

void VolumeModelisationPlugin::mousePress(View* view, QMouseEvent* event)
{
}

void VolumeModelisationPlugin::mouseRelease(View* view, QMouseEvent* event)
{
}

void VolumeModelisationPlugin::mouseMove(View* view, QMouseEvent* event)
{
}

void VolumeModelisationPlugin::wheelEvent(View* view, QWheelEvent* event)
{
}

void VolumeModelisationPlugin::view_linked(View* view)
{
}

void VolumeModelisationPlugin::view_unlinked(View* view)
{
}

} // namespace plugin_volume_modelisation
} // namespace schnapps
