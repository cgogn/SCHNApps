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

#include <schnapps/plugins/selection/selection_dock_tab.h>
#include <schnapps/plugins/selection/selection.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_selection
{

Selection_DockTab::Selection_DockTab(SCHNApps* s, Plugin_Selection* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);

	connect(combo_positionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(position_attribute_changed(int)));
	connect(combo_normalAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_attribute_changed(int)));
	connect(combo_cellType, SIGNAL(currentIndexChanged(int)), this, SLOT(cell_type_changed(int)));
	connect(combo_cellsSet, SIGNAL(currentIndexChanged(int)), this, SLOT(cells_set_changed(int)));
	connect(combo_selectionMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(selection_method_changed(int)));
	connect(button_clear, SIGNAL(clicked()), this, SLOT(clear_clicked()));
	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));
	connect(combo_color, SIGNAL(currentIndexChanged(int)), this, SLOT(color_changed(int)));

	selected_map_ = schnapps_->get_selected_map();
	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		connect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

Selection_DockTab::~Selection_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void Selection_DockTab::position_attribute_changed(int index)
{
	if (!updating_ui_)
		plugin_->set_position_attribute(schnapps_->get_selected_view(), schnapps_->get_selected_map(), combo_positionAttribute->currentText(), false);
}

void Selection_DockTab::normal_attribute_changed(int index)
{
	if (!updating_ui_)
		plugin_->set_normal_attribute(schnapps_->get_selected_view(), schnapps_->get_selected_map(), combo_normalAttribute->currentText(), false);
}

void Selection_DockTab::cell_type_changed(int index)
{
	if (!updating_ui_)
	{
		plugin_->set_cells_set(schnapps_->get_selected_view(), schnapps_->get_selected_map(), nullptr, false);
		update_after_cells_set_changed();
	}
}

void Selection_DockTab::cells_set_changed(int index)
{
	if (!updating_ui_)
	{
		CellsSetGen* cs = selected_map_->get_cells_set(static_cast<CellType>(combo_cellType->currentIndex()), combo_cellsSet->currentText());
		plugin_->set_cells_set(schnapps_->get_selected_view(), schnapps_->get_selected_map(), cs, false);
		update_after_cells_set_changed();
	}
}

void Selection_DockTab::selection_method_changed(int index)
{
	if (!updating_ui_)
	{
		plugin_->set_selection_method(schnapps_->get_selected_view(), schnapps_->get_selected_map(), MapParameters::SelectionMethod(index), false);
		update_after_selection_method_changed();
	}
}

void Selection_DockTab::clear_clicked()
{
	if (!updating_ui_)
	{
		CellsSetGen* cs = selected_map_->get_cells_set(static_cast<CellType>(combo_cellType->currentIndex()), combo_cellsSet->currentText());
		cs->clear();
	}
}

void Selection_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_vertex_scale_factor(schnapps_->get_selected_view(), schnapps_->get_selected_map(), i / 50.0, false);
}

void Selection_DockTab::color_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_color(schnapps_->get_selected_view(), schnapps_->get_selected_map(), combo_color->color(), false);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void Selection_DockTab::selected_view_changed(View* old, View* cur)
{
	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void Selection_DockTab::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
		disconnect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
		disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
	}
	selected_map_ = cur;
	connect(selected_map_, SIGNAL(cells_set_added(CellType, const QString&)), this, SLOT(selected_map_cells_set_added(CellType, const QString&)));
	connect(selected_map_, SIGNAL(cells_set_removed(CellType, const QString&)), this, SLOT(selected_map_cells_set_removed(CellType, const QString&)));
	connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, const QString&)));
	connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from MapHandlerGen signals
/*****************************************************************************/

void Selection_DockTab::selected_map_cells_set_added(CellType ct, const QString& name)
{
	updating_ui_ = true;
	if (ct == CellType(combo_cellType->currentIndex()))
		combo_cellsSet->addItem(name);
	updating_ui_ = false;
}

void Selection_DockTab::selected_map_cells_set_removed(CellType ct, const QString& name)
{
	if (ct == CellType(combo_cellType->currentIndex()))
	{
		int index = combo_cellsSet->findText(name);
		if (index > 0)
			combo_cellsSet->removeItem(index);
	}
}

void Selection_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;

	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		QString vec3_type_name = QString::fromStdString(cgogn::name_of_type(VEC3()));

		const MapHandlerGen::ChunkArrayContainer<uint32>* container = map->attribute_container(Vertex_Cell);
		QString attribute_type_name = QString::fromStdString(container->get_chunk_array(name.toStdString())->type_name());

		if (attribute_type_name == vec3_type_name)
		{
			combo_positionAttribute->addItem(name);
			combo_normalAttribute->addItem(name);
		}
	}

	updating_ui_ = false;
}

void Selection_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
{
	MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());

	if (map->cell_type(orbit) == Vertex_Cell)
	{
		int index = combo_positionAttribute->findText(name, Qt::MatchExactly);
		if (index > 0)
			combo_positionAttribute->removeItem(index);

		index = combo_normalAttribute->findText(name, Qt::MatchExactly);
		if (index > 0)
			combo_normalAttribute->removeItem(index);
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void Selection_DockTab::set_position_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_positionAttribute->findText(name);
	if (index > 0)
		combo_positionAttribute->setCurrentIndex(index);
	else
		combo_positionAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void Selection_DockTab::set_normal_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_normalAttribute->findText(name);
	if (index > 0)
		combo_normalAttribute->setCurrentIndex(index);
	else
		combo_normalAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void Selection_DockTab::set_cells_set(CellsSetGen* cs)
{
	update_after_cells_set_changed();
}

void Selection_DockTab::set_selection_method(MapParameters::SelectionMethod m)
{
	combo_selectionMethod->setCurrentIndex(static_cast<int>(m));
	update_after_selection_method_changed();
}

void Selection_DockTab::set_vertex_scale_factor(float sf)
{
	updating_ui_ = true;
	slider_vertexScaleFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void Selection_DockTab::set_color(const QColor &color)
{
	updating_ui_ = true;
	combo_color->setColor(color);
	updating_ui_ = false;
}

void Selection_DockTab::refresh_ui()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (!map || !view)
		return;

	const MapParameters& p = plugin_->get_parameters(view, map);

	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_positionAttribute->addItem("- select attribute -");

	combo_normalAttribute->clear();
	combo_normalAttribute->addItem("- select attribute -");

	combo_cellsSet->clear();
	combo_cellsSet->addItem("- select set -");

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
			const MapHandlerGen::Attribute_T<VEC3>& pos = p.get_position_attribute();
			if (pos.is_valid() && QString::fromStdString(pos.name()) == name)
				combo_positionAttribute->setCurrentIndex(i);

			combo_normalAttribute->addItem(name);
			const MapHandlerGen::Attribute_T<VEC3>& nor = p.get_normal_attribute();
			if (nor.is_valid() && QString::fromStdString(nor.name()) == name)
				combo_normalAttribute->setCurrentIndex(i);

			++i;
		}
	}

	CellType ct = static_cast<CellType>(combo_cellType->currentIndex());
	CellsSetGen* cs = p.get_cells_set();
	if (cs)
		ct = cs->get_cell_type();

	combo_cellType->setCurrentIndex(static_cast<int>(ct));

	i = 1;
	map->foreach_cells_set(CellType(combo_cellType->currentIndex()), [&] (CellsSetGen* cells_set)
	{
		combo_cellsSet->addItem(cells_set->get_name());
		if (cs == cells_set)
			combo_cellsSet->setCurrentIndex(i);
		++i;
	});

	combo_selectionMethod->setCurrentIndex(static_cast<int>(p.get_selection_method()));

	combo_color->setColor(p.get_color());

	slider_vertexScaleFactor->setSliderPosition(p.get_vertex_scale_factor() * 50.0);

	switch (p.get_selection_method())
	{
		case MapParameters::SingleCell:
			spin_angle_radius->setVisible(false);
			label_angle_radius->setText(QString());
			break;
		case MapParameters::WithinSphere:
			spin_angle_radius->setVisible(true);
			spin_angle_radius->setValue(p.get_vertex_base_size() * 10.0f * p.get_selection_radius_scale_factor_());
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

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void Selection_DockTab::update_after_cells_set_changed()
{
	updating_ui_ = true;
	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	const MapParameters& p = plugin_->get_parameters(view, map);

	CellType ct = static_cast<CellType>(combo_cellType->currentIndex());
	CellsSetGen* cs = p.get_cells_set();
	if (cs)
		ct = cs->get_cell_type();

	combo_cellType->setCurrentIndex(static_cast<int>(ct));

	combo_cellsSet->clear();
	combo_cellsSet->addItem("- select set -");

	uint32 i = 1;
	map->foreach_cells_set(ct, [&] (CellsSetGen* cells_set)
	{
		combo_cellsSet->addItem(cells_set->get_name());
		if (cells_set == cs)
			combo_cellsSet->setCurrentIndex(i);
		++i;
	});
	updating_ui_ = false;
}

void Selection_DockTab::update_after_selection_method_changed()
{
	updating_ui_ = true;
	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	const MapParameters& p = plugin_->get_parameters(view, map);

	switch (p.get_selection_method())
	{
		case MapParameters::SingleCell:
			spin_angle_radius->setVisible(false);
			label_angle_radius->setText(QString());
			break;
		case MapParameters::WithinSphere:
			spin_angle_radius->setVisible(true);
			spin_angle_radius->setValue(p.get_vertex_base_size() * 10.0f * p.get_selection_radius_scale_factor_());
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

} // namespace plugin_selection

} // namespace schnapps
