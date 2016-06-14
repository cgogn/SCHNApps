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
	updating_ui_(false)
{
	setupUi(this);

	combo_color->setEnabled(true);

	connect(combo_positionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(position_attribute_changed(int)));
	connect(combo_normalAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_attribute_changed(int)));
	connect(combo_selectionMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(selection_method_changed(int)));
	connect(slider_verticesScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertices_scale_factor_changed(int)));
	connect(slider_verticesScaleFactor, SIGNAL(sliderPressed()), this, SLOT(vertices_scale_factor_pressed()));
	connect(combo_color, SIGNAL(currentIndexChanged(int)), this, SLOT(color_changed(int)));
	connect(button_clear, SIGNAL(clicked()), this, SLOT(clear_clicked()));
}





void Selection_DockTab::position_attribute_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			MapParameters& p = plugin_->get_parameters(map);
			p.set_position_attribute(combo_positionAttribute->currentText());
//			plugin_->update_selected_cells_rendering();
		}
	}
}

void Selection_DockTab::normal_attribute_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			MapParameters& p = plugin_->get_parameters(map);
			p.set_position_attribute(combo_normalAttribute->currentText());
//			plugin_->update_selected_cells_rendering();
		}
	}
}

void Selection_DockTab::selection_method_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			MapParameters& p = plugin_->get_parameters(map);
			p.selection_method_ = MapParameters::SelectionMethod(index);
		}
		
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

void Selection_DockTab::vertices_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			MapParameters& p = plugin_->get_parameters(map);
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
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			MapParameters& p = plugin_->get_parameters(map);
			p.set_vertex_base_size(map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges())));
			for (View* view : map->get_linked_views())
				view->update();
		}
	}
}

void Selection_DockTab::color_changed(int i)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			MapParameters& p = plugin_->get_parameters(map);
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

void Selection_DockTab::add_vertex_attribute(const QString& attribute_name)
{
	updating_ui_ = true;

	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(schnapps_->get_selected_map());
	if (!mh)
		return;

	const CMap2* map2 = mh->get_map();
	const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
	QString attribute_type_name = QString::fromStdString(container.get_chunk_array(attribute_name.toStdString())->type_name());

	if (attribute_type_name == vec3_type_name)
	{
		combo_positionAttribute->addItem(attribute_name);
		combo_normalAttribute->addItem(attribute_name);
	}

	updating_ui_ = false;
}

void Selection_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_positionAttribute->addItem("- select attribute -");

	combo_normalAttribute->clear();
	combo_normalAttribute->addItem("- select attribute -");

	QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(map);
	if (!mh)
		return;

	const CMap2* map2 = mh->get_map();
	const CMap2::ChunkArrayContainer<cgogn::numerics::uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
	const std::vector<std::string>& names = container.names();
	const std::vector<std::string>& type_names = container.type_names();

	unsigned int i = 1;
	for (std::size_t i = 0u; i < names.size(); ++i)
	{
		QString name = QString::fromStdString(names[i]);
		QString type = QString::fromStdString(type_names[i]);
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
