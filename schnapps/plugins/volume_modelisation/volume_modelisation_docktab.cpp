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

#include <volume_modelisation_docktab.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <volume_modelisation.h>

namespace schnapps
{

namespace plugin_volume_modelisation
{

VolumeModelisation_DockTab::VolumeModelisation_DockTab(SCHNApps* s, VolumeModelisationPlugin* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	this->setupUi(this);

	connect(process_pushButton, SIGNAL(pressed()), plugin_, SLOT(process_operation()));
}

VolumeModelisation_DockTab::~VolumeModelisation_DockTab()
{
	disconnect(process_pushButton, SIGNAL(pressed()), plugin_, SLOT(process_operation()));
}

void VolumeModelisation_DockTab::update(MapHandlerGen* map)
{
	position_comboBox->clear();
	position_comboBox->addItem("-select attribute-");
	if (map)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));
		const MapHandlerGen::ChunkArrayContainer<cgogn::numerics::uint32>& container = map->const_attribute_container(CellType::Vertex_Cell);
		const std::vector<std::string>& names = container.names();
		const std::vector<std::string>& type_names = container.type_names();

		for (std::size_t j = 0u; j < names.size(); ++j)
		{
			QString name = QString::fromStdString(names[j]);
			QString type = QString::fromStdString(type_names[j]);
			if (type == vec3_type_name)
				position_comboBox->addItem(name);
		}
	}

	dartSet_comboBox->clear();
	dartSet_comboBox->addItem("-select set-");
	vertexSet_comboBox->clear();
	vertexSet_comboBox->addItem("-select set-");
	edgeSet_comboBox->clear();
	edgeSet_comboBox->addItem("-select set-");
	faceSet_comboBox->clear();
	faceSet_comboBox->addItem("-select set-");
	volumeSet_comboBox->clear();
	volumeSet_comboBox->addItem("-select set-");

	operations_combobox->clear();
	operations_combobox->addItem("-select set-");
	auto&& op_names = plugin_->operations_->get_operations();
	for (const std::string& name : op_names)
		operations_combobox->addItem(QString::fromStdString(name));

	if (map && map->dimension() == 3u)
	{
		map->foreach_cells_set(CellType::Dart_Cell, [&](CellsSetGen* csg)
		{
			dartSet_comboBox->addItem(csg->get_name());
		});
		map->foreach_cells_set(CellType::Vertex_Cell, [&](CellsSetGen* csg)
		{
			vertexSet_comboBox->addItem(csg->get_name());
		});
		map->foreach_cells_set(CellType::Edge_Cell, [&](CellsSetGen* csg)
		{
			edgeSet_comboBox->addItem(csg->get_name());
		});
		map->foreach_cells_set(CellType::Face_Cell, [&](CellsSetGen* csg)
		{
			faceSet_comboBox->addItem(csg->get_name());
		});
		map->foreach_cells_set(CellType::Volume_Cell, [&](CellsSetGen* csg)
		{
			volumeSet_comboBox->addItem(csg->get_name());
		});
	}
}

QComboBox*VolumeModelisation_DockTab::get_cell_set_combo_box(CellType ct)
{
	switch (ct) {
		case CellType::Dart_Cell: return dartSet_comboBox;
		case CellType::Vertex_Cell: return vertexSet_comboBox;
		case CellType::Edge_Cell: return edgeSet_comboBox;
		case CellType::Face_Cell: return faceSet_comboBox;
		case CellType::Volume_Cell: return volumeSet_comboBox;
		default:
			return nullptr;
	}
}

} // namespace plugin_volume_modelisation
} // namespace schnapps
