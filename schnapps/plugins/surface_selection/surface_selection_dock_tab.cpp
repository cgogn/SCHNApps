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

#include <schnapps/plugins/surface_selection/surface_selection_dock_tab.h>
#include <schnapps/plugins/surface_selection/surface_selection.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_selection
{

SurfaceSelection_DockTab::SurfaceSelection_DockTab(SCHNApps* s, Plugin_SurfaceSelection* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap2_provider_(nullptr),
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(combo_positionAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(position_attribute_changed(int)));
	connect(combo_normalAttribute, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_attribute_changed(int)));
	connect(combo_cellType, SIGNAL(currentIndexChanged(int)), this, SLOT(cell_type_changed(int)));
	connect(combo_cellsSet, SIGNAL(currentIndexChanged(int)), this, SLOT(cells_set_changed(int)));
	connect(combo_selectionMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(selection_method_changed(int)));
	connect(button_clear, SIGNAL(clicked()), this, SLOT(clear_clicked()));
	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));
	connect(combo_color, SIGNAL(currentIndexChanged(int)), this, SLOT(color_changed(int)));

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));

	View* v = schnapps_->selected_view();
	connect(v, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(v, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	for (Object* o : v->linked_objects())
		object_linked(o);

	plugin_cmap2_provider_ = qobject_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));
}

SurfaceSelection_DockTab::~SurfaceSelection_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

cgogn::Orbit SurfaceSelection_DockTab::orbit_from_index(int index)
{
	switch (index)
	{
		case 0: return CMap2::CDart::ORBIT;
		case 1: return CMap2::Vertex::ORBIT;
		case 2: return CMap2::Edge::ORBIT;
		case 3: return CMap2::Face::ORBIT;
		case 4: return CMap2::Volume::ORBIT;
		default: return CMap2::CDart::ORBIT;
	}
}

int SurfaceSelection_DockTab::index_from_orbit(cgogn::Orbit orbit)
{
	switch (orbit)
	{
		case CMap2::CDart::ORBIT : return 0;
		case CMap2::Vertex::ORBIT : return 1;
		case CMap2::Edge::ORBIT : return 2;
		case CMap2::Face::ORBIT : return 3;
		case CMap2::Volume::ORBIT : return 4;
		default: return 0;
	}
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void SurfaceSelection_DockTab::selected_map_changed()
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
		selected_map_ = plugin_cmap2_provider_->map(map_name);
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

void SurfaceSelection_DockTab::position_attribute_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_position_attribute(schnapps_->selected_view(), selected_map_, combo_positionAttribute->currentText(), false);
}

void SurfaceSelection_DockTab::normal_attribute_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_normal_attribute(schnapps_->selected_view(), selected_map_, combo_normalAttribute->currentText(), false);
}

void SurfaceSelection_DockTab::cell_type_changed(int)
{
	if (!updating_ui_ && selected_map_)
	{
		plugin_->set_cells_set(schnapps_->selected_view(), selected_map_, nullptr, false);
		update_after_cells_set_changed();
	}
}

void SurfaceSelection_DockTab::cells_set_changed(int)
{
	if (!updating_ui_ && selected_map_)
	{
		CMap2CellsSetGen* cs = selected_map_->cells_set(orbit_from_index(combo_cellType->currentIndex()), combo_cellsSet->currentText());
		plugin_->set_cells_set(schnapps_->selected_view(), selected_map_, cs, false);
		update_after_cells_set_changed();
	}
}

void SurfaceSelection_DockTab::selection_method_changed(int index)
{
	if (!updating_ui_ && selected_map_)
	{
		plugin_->set_selection_method(schnapps_->selected_view(), selected_map_, SelectionMethod(index), false);
		update_after_selection_method_changed();
	}
}

void SurfaceSelection_DockTab::clear_clicked()
{
	if (!updating_ui_ && selected_map_)
	{
		CMap2CellsSetGen* cs = selected_map_->cells_set(orbit_from_index(combo_cellType->currentIndex()), combo_cellsSet->currentText());
		if (cs)
			cs->clear();
	}
}

void SurfaceSelection_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_vertex_scale_factor(schnapps_->selected_view(), selected_map_, i / 50.0, false);
}

void SurfaceSelection_DockTab::color_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_color(schnapps_->selected_view(), selected_map_, combo_color->color(), false);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceSelection_DockTab::selected_view_changed(View* old, View* cur)
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

void SurfaceSelection_DockTab::object_linked(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_linked(mh);
}

void SurfaceSelection_DockTab::map_linked(CMap2Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::object_unlinked(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_unlinked(mh);
}

void SurfaceSelection_DockTab::map_unlinked(CMap2Handler* mh)
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
// slots called from CMap2Handler signals
/*****************************************************************************/

void SurfaceSelection_DockTab::selected_map_cells_set_added(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	if (orbit == orbit_from_index(combo_cellType->currentIndex()))
		combo_cellsSet->addItem(name);
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::selected_map_cells_set_removed(cgogn::Orbit orbit, const QString& name)
{
	updating_ui_ = true;
	if (orbit == orbit_from_index(combo_cellType->currentIndex()))
	{
		int index = combo_cellsSet->findText(name);
		if (index > 0)
			combo_cellsSet->removeItem(index);
	}
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::selected_map_attribute_added(cgogn::Orbit orbit, const QString& name)
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
			combo_normalAttribute->addItem(name);
			updating_ui_ = false;
		}
	}
}

void SurfaceSelection_DockTab::selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name)
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

			int index = combo_positionAttribute->findText(name, Qt::MatchExactly);
			if (index > 0)
				combo_positionAttribute->removeItem(index);

			index = combo_normalAttribute->findText(name, Qt::MatchExactly);
			if (index > 0)
				combo_normalAttribute->removeItem(index);

			updating_ui_ = false;
		}
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void SurfaceSelection_DockTab::set_position_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_positionAttribute->findText(name);
	if (index > 0)
		combo_positionAttribute->setCurrentIndex(index);
	else
		combo_positionAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::set_normal_attribute(const QString& name)
{
	updating_ui_ = true;
	int index = combo_normalAttribute->findText(name);
	if (index > 0)
		combo_normalAttribute->setCurrentIndex(index);
	else
		combo_normalAttribute->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::set_cells_set(CMap2CellsSetGen*)
{
	update_after_cells_set_changed();
}

void SurfaceSelection_DockTab::set_selection_method(SelectionMethod m)
{
	combo_selectionMethod->setCurrentIndex(static_cast<int>(m));
	update_after_selection_method_changed();
}

void SurfaceSelection_DockTab::set_vertex_scale_factor(float sf)
{
	updating_ui_ = true;
	slider_vertexScaleFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::set_color(const QColor &color)
{
	updating_ui_ = true;
	combo_color->setColor(color);
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::refresh_ui()
{
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	if (!mh || !view)
		return;

	const MapParameters& p = plugin_->parameters(view, mh);

	updating_ui_ = true;

	combo_positionAttribute->clear();
	combo_positionAttribute->addItem("- select attribute -");

	combo_normalAttribute->clear();
	combo_normalAttribute->addItem("- select attribute -");

	combo_cellsSet->clear();
	combo_cellsSet->addItem("- select set -");

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
				combo_positionAttribute->setCurrentIndex(i);

			combo_normalAttribute->addItem(name);
			if (p.normal_attribute().is_valid() && QString::fromStdString(p.normal_attribute().name()) == name)
				combo_normalAttribute->setCurrentIndex(i);

			++i;
		}
	}

	cgogn::Orbit orbit = orbit_from_index(combo_cellType->currentIndex());
	CMap2CellsSetGen* cs = p.cells_set();
	if (cs)
		orbit = cs->orbit();

	combo_cellType->setCurrentIndex(index_from_orbit(orbit));

	i = 1;
	mh->foreach_cells_set(orbit_from_index(combo_cellType->currentIndex()), [&] (CMap2CellsSetGen* cells_set)
	{
		combo_cellsSet->addItem(cells_set->name());
		if (cs == cells_set)
			combo_cellsSet->setCurrentIndex(i);
		++i;
	});

	combo_selectionMethod->setCurrentIndex(static_cast<int>(p.selection_method()));

	combo_color->setColor(p.color());

	slider_vertexScaleFactor->setSliderPosition(p.vertex_scale_factor() * 50.0);

	switch (p.selection_method())
	{
		case SingleCell:
			spin_angle_radius->setVisible(false);
			label_angle_radius->setText(QString());
			break;
		case WithinSphere:
			spin_angle_radius->setVisible(true);
			spin_angle_radius->setValue(p.vertex_base_size() * 10.0f * p.selection_radius_scale_factor());
			label_angle_radius->setText(QString("Radius:"));
			break;
		case NormalAngle:
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

void SurfaceSelection_DockTab::update_after_cells_set_changed()
{
	updating_ui_ = true;
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	const MapParameters& p = plugin_->parameters(view, mh);

	cgogn::Orbit orbit = orbit_from_index(combo_cellType->currentIndex());
	CMap2CellsSetGen* cs = p.cells_set();
	if (cs)
		orbit = cs->orbit();

	combo_cellType->setCurrentIndex(index_from_orbit(orbit));

	combo_cellsSet->clear();
	combo_cellsSet->addItem("- select set -");

	uint32 i = 1;
	mh->foreach_cells_set(orbit, [&] (CMap2CellsSetGen* cells_set)
	{
		combo_cellsSet->addItem(cells_set->name());
		if (cells_set == cs)
			combo_cellsSet->setCurrentIndex(i);
		++i;
	});
	updating_ui_ = false;
}

void SurfaceSelection_DockTab::update_after_selection_method_changed()
{
	updating_ui_ = true;
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	const MapParameters& p = plugin_->parameters(view, mh);

	switch (p.selection_method())
	{
		case SingleCell:
			spin_angle_radius->setVisible(false);
			label_angle_radius->setText(QString());
			break;
		case WithinSphere:
			spin_angle_radius->setVisible(true);
			spin_angle_radius->setValue(p.vertex_base_size() * 10.0f * p.selection_radius_scale_factor());
			label_angle_radius->setText(QString("Radius:"));
			break;
		case NormalAngle:
			spin_angle_radius->setVisible(true);
//			spin_angle_radius->setValue(plugin_->m_normalAngleThreshold / M_PI * 180);
			label_angle_radius->setText(QString("Angle:"));
			break;
		default:
			break;
	}
	updating_ui_ = false;
}

} // namespace plugin_surface_selection

} // namespace schnapps
