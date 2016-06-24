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

#include <selection_dock_tab.h>
#include <selection.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

Selection_DockTab::Selection_DockTab(SCHNApps* s, Plugin_Selection* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false),
	current_cells_set_(nullptr)
{
	setupUi(this);

	combo_color->setEnabled(true);

	connect(combo_positionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(position_attribute_changed(int)));
	connect(combo_normalAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_attribute_changed(int)));
	connect(combo_selectionMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(selection_method_changed(int)));
	connect(combo_cellType, SIGNAL(currentIndexChanged(int)), this, SLOT(cell_type_changed(int)));
	connect(combo_cellsSet, SIGNAL(currentIndexChanged(int)), this, SLOT(cells_set_changed(int)));
	connect(slider_verticesScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertices_scale_factor_changed(int)));
	connect(slider_verticesScaleFactor, SIGNAL(sliderPressed()), this, SLOT(vertices_scale_factor_pressed()));
	connect(combo_color, SIGNAL(currentIndexChanged(int)), this, SLOT(color_changed(int)));
	connect(button_clear, SIGNAL(clicked()), this, SLOT(clear_clicked()));
}





void Selection_DockTab::position_attribute_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_position_attribute(combo_positionAttribute->currentText());
			p.update_selected_cells_rendering();
		}
	}
}

void Selection_DockTab::normal_attribute_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_position_attribute(combo_normalAttribute->currentText());
			p.update_selected_cells_rendering();
		}
	}
}

void Selection_DockTab::selection_method_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.selection_method_ = MapParameters::SelectionMethod(index);

			switch (index)
			{
				case MapParameters::SingleCell:
					spin_angle_radius->setVisible(false);
					label_angle_radius->setText(QString());
					break;
				case MapParameters::WithinSphere:
					spin_angle_radius->setVisible(true);
	//				spin_angle_radius->setValue(plugin_->m_selectionRadiusBase * plugin_->m_selectionRadiusCoeff);
					label_angle_radius->setText(QString("Radius:"));
					break;
				case MapParameters::NormalAngle:
					spin_angle_radius->setVisible(true);
	//				spin_angle_radius->setValue(plugin_->m_normalAngleThreshold / M_PI * 180);
					label_angle_radius->setText(QString("Angle:"));
					break;
				default:
					break;
			}
		}
	}
}

void Selection_DockTab::cell_type_changed(int index)
{
	updating_ui_ = true;

	combo_cellsSet->clear();
	combo_cellsSet->addItem("- select set -");

	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (view && map)
	{
		MapParameters& p = plugin_->get_parameters(view, map);
		uint32 i = 1;
		map->foreach_cells_set(static_cast<CellType>(combo_cellType->currentIndex()), [&] (CellsSetGen* cells_set)
		{
			combo_cellsSet->addItem(cells_set->get_name());
			if (p.get_cells_set() == cells_set)
				combo_cellsSet->setCurrentIndex(i);
			++i;
		});
	}

	updating_ui_ = false;
}

void Selection_DockTab::cells_set_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			CellsSetGen* cs = map->get_cells_set(CellType(combo_cellType->currentIndex()), combo_cellsSet->currentText());
			p.set_cells_set(dynamic_cast<CellsSet<CMap2, MapHandler<CMap2>::Vertex>*>(cs));
			p.update_selected_cells_rendering();
		}
	}
}

void Selection_DockTab::selected_map_cells_set_added(CellType ct, const QString& name)
{
	updating_ui_ = true;
	if (ct == CellType(combo_cellType->currentIndex()))
		combo_cellsSet->addItem(name);
	updating_ui_ = false;
}

void Selection_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map->cell_type(orbit) == Vertex_Cell)
	{
		updating_ui_ = true;

		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const MapHandlerGen::ChunkArrayContainer<cgogn::numerics::uint32>& container = map->const_attribute_container(Vertex_Cell);
		QString attribute_type_name = QString::fromStdString(container.get_chunk_array(name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			combo_positionAttribute->addItem(name);
			combo_normalAttribute->addItem(name);
		}

		updating_ui_ = false;
	}
}

void Selection_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	if (map->cell_type(orbit) == Vertex_Cell)
	{
		updating_ui_ = true;

		int curIndex = combo_positionAttribute->currentIndex();
		int index = combo_positionAttribute->findText(name, Qt::MatchExactly);
		if (curIndex == index)
			combo_positionAttribute->setCurrentIndex(0);
		combo_positionAttribute->removeItem(index);

		curIndex = combo_normalAttribute->currentIndex();
		index = combo_normalAttribute->findText(name, Qt::MatchExactly);
		if (curIndex == index)
			combo_normalAttribute->setCurrentIndex(0);
		combo_normalAttribute->removeItem(index);

		updating_ui_ = false;
	}
}

void Selection_DockTab::vertices_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_vertex_scale_factor(i / 50.0);
			for (View* view : map->get_linked_views())
				view->update();
		}
	}
}

void Selection_DockTab::vertices_scale_factor_pressed()
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_cells(Edge_Cell))));
			for (View* view : map->get_linked_views())
				view->update();
		}
	}
}

void Selection_DockTab::color_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_color(combo_color->color());
			for (View* view : map->get_linked_views())
				view->update();
		}
	}
}

void Selection_DockTab::clear_clicked()
{
	if (!updating_ui_)
	{
//		MapHandlerGen* map = schnapps_->get_selected_map();
//		cgogn::Orbit orbit = schnapps_->get_current_orbit();
//		CellSelectorGen* sel = schnapps_->getSelectedSelector(orbit);
//		if (map && sel)
//			plugin_->clearSelection(map->get_name(), orbit, sel->get_name());
	}
}

void Selection_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_positionAttribute->addItem("- select attribute -");

	combo_normalAttribute->clear();
	combo_normalAttribute->addItem("- select attribute -");

	combo_cellsSet->clear();
	combo_cellsSet->addItem("- select set -");

	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(map);
	if (!mh)
		return;

	const CMap2* map2 = mh->get_map();
	const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
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
			if (p.get_position_attribute().is_valid() && p.get_position_attribute_name() == name)
				combo_positionAttribute->setCurrentIndex(i);

			combo_normalAttribute->addItem(name);
			if (p.get_normal_attribute().is_valid() && p.get_normal_attribute_name() == name)
				combo_normalAttribute->setCurrentIndex(i);

			++i;
		}
	}

	i = 1;
	map->foreach_cells_set(CellType(combo_cellType->currentIndex()), [&] (CellsSetGen* cells_set)
	{
		combo_cellsSet->addItem(cells_set->get_name());
		if (p.get_cells_set() == cells_set)
			combo_cellsSet->setCurrentIndex(i);
		++i;
	});

	combo_selectionMethod->setCurrentIndex(p.selection_method_);
	combo_color->setColor(p.get_color());

	slider_verticesScaleFactor->setSliderPosition(p.get_vertex_scale_factor() * 50.0);

	switch (p.selection_method_)
	{
		case MapParameters::SingleCell:
			spin_angle_radius->setVisible(false);
			label_angle_radius->setText(QString());
			break;
		case MapParameters::WithinSphere:
			spin_angle_radius->setVisible(true);
//			spin_angle_radius->setValue(plugin_->m_selectionRadiusBase * plugin_->m_selectionRadiusCoeff);
			label_angle_radius->setText(QString("Radius:"));
			break;
		case MapParameters::NormalAngle:
			spin_angle_radius->setVisible(true);
//			spin_angle_radius->setValue(plugin_->m_normalAngleThreshold / M_PI * 180);
			label_angle_radius->setText(QString("Angle:"));
			break;
		default:
			break;
	}

	updating_ui_ = false;
}

} // namespace schnapps
