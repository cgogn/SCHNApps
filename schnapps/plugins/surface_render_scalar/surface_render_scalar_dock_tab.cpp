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

#include <surface_render_scalar_dock_tab.h>
#include <surface_render_scalar.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_render_scalar
{

SurfaceRenderScalar_DockTab::SurfaceRenderScalar_DockTab(SCHNApps* s, Plugin_SurfaceRenderScalar* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);

	list_scalarVBO->setSelectionMode(QAbstractItemView::SingleSelection);
	combo_colorMap->setDisabled(true);
	slider_expansion->setDisabled(true);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(list_scalarVBO, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selected_scalar_vbo_changed(QListWidgetItem*, QListWidgetItem*)));
	connect(combo_colorMap, SIGNAL(currentIndexChanged(int)), this, SLOT(color_map_changed(int)));
	connect(check_autoUpdateMinMax, SIGNAL(toggled(bool)), this, SLOT(auto_update_min_max_changed(bool)));
	connect(spin_min, SIGNAL(valueChanged(double)), this, SLOT(scalar_min_changed(double)));
	connect(spin_max, SIGNAL(valueChanged(double)), this, SLOT(scalar_max_changed(double)));
	connect(slider_expansion, SIGNAL(valueChanged(int)), this, SLOT(expansion_changed(int)));
	connect(check_showIsoLines, SIGNAL(toggled(bool)), this, SLOT(show_iso_lines_changed(bool)));
	connect(slider_nbIsoLevels, SIGNAL(valueChanged(int)), this, SLOT(nb_iso_levels_changed(int)));

	selected_map_ = schnapps_->get_selected_map();
	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
	}

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

SurfaceRenderScalar_DockTab::~SurfaceRenderScalar_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_ui()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_ui()));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void SurfaceRenderScalar_DockTab::position_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
			plugin_->set_position_vbo(schnapps_->get_selected_view(), map, map->get_vbo(combo_positionVBO->currentText()), false);
	}
}

void SurfaceRenderScalar_DockTab::selected_scalar_vbo_changed(QListWidgetItem* item, QListWidgetItem* old)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			cgogn::rendering::VBO* vbo = nullptr;
			if (item)
				vbo = map->get_vbo(item->text());
			plugin_->set_scalar_vbo(schnapps_->get_selected_view(), map, vbo, false);
			update_after_scalar_vbo_changed();
		}
	}
}

void SurfaceRenderScalar_DockTab::color_map_changed(int index)
{
	if (!updating_ui_)
		plugin_->set_color_map(schnapps_->get_selected_view(), schnapps_->get_selected_map(), cgogn::rendering::ShaderScalarPerVertex::ColorMap(index), false);
}

void SurfaceRenderScalar_DockTab::auto_update_min_max_changed(bool b)
{
	if (!updating_ui_)
	{
		plugin_->set_auto_update_min_max(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
		update_after_auto_update_min_max_changed();
	}
}

void SurfaceRenderScalar_DockTab::scalar_min_changed(double d)
{
	if (!updating_ui_)
		plugin_->set_scalar_min(schnapps_->get_selected_view(), schnapps_->get_selected_map(), d, false);
}

void SurfaceRenderScalar_DockTab::scalar_max_changed(double d)
{
	if (!updating_ui_)
		plugin_->set_scalar_max(schnapps_->get_selected_view(), schnapps_->get_selected_map(), d, false);
}

void SurfaceRenderScalar_DockTab::expansion_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_expansion(schnapps_->get_selected_view(), schnapps_->get_selected_map(), i, false);
}

void SurfaceRenderScalar_DockTab::show_iso_lines_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_show_iso_lines(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void SurfaceRenderScalar_DockTab::nb_iso_levels_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_nb_iso_levels(schnapps_->get_selected_view(), schnapps_->get_selected_map(), i, false);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceRenderScalar_DockTab::selected_view_changed(View* old, View* cur)
{
	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void SurfaceRenderScalar_DockTab::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
	}
	selected_map_ = cur;
	connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from MapHandlerGen signals
/*****************************************************************************/

void SurfaceRenderScalar_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
		combo_positionVBO->addItem(vbo_name);
	else if (vbo->vector_dimension() == 1)
		list_scalarVBO->addItem(vbo_name);
}

void SurfaceRenderScalar_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		int index = combo_positionVBO->findText(vbo_name);
		if (index > 0)
			combo_positionVBO->removeItem(index);
	}
	else if (vbo->vector_dimension() == 1)
	{
		QList<QListWidgetItem*> items = list_scalarVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
			delete items[0];
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void SurfaceRenderScalar_DockTab::set_position_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	if (vbo)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		int index = combo_positionVBO->findText(vbo_name);
		if (index > 0)
			combo_positionVBO->setCurrentIndex(index);
	}
	else
		combo_positionVBO->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_scalar_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	list_scalarVBO->clearSelection();
	if (vbo)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		QList<QListWidgetItem*> items = list_scalarVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
			items[0]->setSelected(true);
	}
	update_after_scalar_vbo_changed();
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_color_map(cgogn::rendering::ShaderScalarPerVertex::ColorMap cm)
{
	updating_ui_ = true;
	combo_colorMap->setCurrentIndex(cm);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_auto_update_min_max(bool b)
{
	updating_ui_ = true;
	check_autoUpdateMinMax->setChecked(b);
	update_after_auto_update_min_max_changed();
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_scalar_min(double d)
{
	updating_ui_ = true;
	spin_min->setValue(d);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_scalar_max(double d)
{
	updating_ui_ = true;
	spin_max->setValue(d);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_expansion(int i)
{
	updating_ui_ = true;
	slider_expansion->setSliderPosition(i);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_show_iso_lines(bool b)
{
	updating_ui_ = true;
	check_showIsoLines->setChecked(b);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::set_nb_iso_levels(int i)
{
	updating_ui_ = true;
	slider_nbIsoLevels->setSliderPosition(i);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::refresh_ui()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (!map || !view)
		return;

	const MapParameters& p = plugin_->get_parameters(view, map);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	list_scalarVBO->clear();

	unsigned int i = 1;
	unsigned int j = 0;
	for (auto& vbo_it : map->get_vbo_set())
	{
		auto& vbo = vbo_it.second;
		const uint32 dimension = vbo->vector_dimension();
		if (dimension == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_position_vbo())
				combo_positionVBO->setCurrentIndex(i);
			++i;
		}
		if (dimension == 1)
		{
			list_scalarVBO->addItem(QString::fromStdString(vbo->name()));
			list_scalarVBO->item(j)->setSelected(false);
			if (vbo.get() == p.get_scalar_vbo())
				list_scalarVBO->item(j)->setSelected(true);
			++j;
		}
	}

	if (p.get_scalar_vbo())
	{
		combo_colorMap->setEnabled(true);
		combo_colorMap->setCurrentIndex(p.get_color_map());
		check_autoUpdateMinMax->setEnabled(true);
		check_autoUpdateMinMax->setChecked(p.get_auto_update_min_max());
		spin_min->setValue(p.get_scalar_min());
		spin_max->setValue(p.get_scalar_max());
		if (!p.get_auto_update_min_max())
		{
			spin_min->setEnabled(true);
			spin_max->setEnabled(true);
		}
		else
		{
			spin_min->setDisabled(true);
			spin_max->setDisabled(true);
		}
		slider_expansion->setEnabled(true);
		slider_expansion->setSliderPosition(p.get_expansion());
		check_showIsoLines->setEnabled(true);
		check_showIsoLines->setChecked(p.get_show_iso_lines());
		slider_nbIsoLevels->setEnabled(true);
		slider_nbIsoLevels->setSliderPosition(p.get_nb_iso_levels());
	}
	else
	{
		combo_colorMap->setDisabled(true);
		check_autoUpdateMinMax->setDisabled(true);
		spin_min->setDisabled(true);
		spin_max->setDisabled(true);
		slider_expansion->setDisabled(true);
		check_showIsoLines->setDisabled(true);
		slider_nbIsoLevels->setDisabled(true);
	}

	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void SurfaceRenderScalar_DockTab::update_after_scalar_vbo_changed()
{
	updating_ui_ = true;
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	const MapParameters& p = plugin_->get_parameters(view, map);

	if (p.get_scalar_vbo())
	{
		combo_colorMap->setEnabled(true);
		check_autoUpdateMinMax->setEnabled(true);
		check_autoUpdateMinMax->setChecked(p.get_auto_update_min_max());
		spin_min->setValue(p.get_scalar_min());
		spin_max->setValue(p.get_scalar_max());
		if (!p.get_auto_update_min_max())
		{
			spin_min->setEnabled(true);
			spin_max->setEnabled(true);
		}
		else
		{
			spin_min->setDisabled(true);
			spin_max->setDisabled(true);
		}
		slider_expansion->setEnabled(true);
		check_showIsoLines->setEnabled(true);
		slider_nbIsoLevels->setEnabled(true);
	}
	else
	{
		combo_colorMap->setDisabled(true);
		check_autoUpdateMinMax->setDisabled(true);
		spin_min->setDisabled(true);
		spin_max->setDisabled(true);
		slider_expansion->setDisabled(true);
		check_showIsoLines->setDisabled(true);
		slider_nbIsoLevels->setDisabled(true);
	}
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::update_after_auto_update_min_max_changed()
{
	updating_ui_ = true;
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	const MapParameters& p = plugin_->get_parameters(view, map);

	spin_min->setValue(p.get_scalar_min());
	spin_max->setValue(p.get_scalar_max());
	if (!p.get_auto_update_min_max())
	{
		spin_min->setEnabled(true);
		spin_max->setEnabled(true);
	}
	else
	{
		spin_min->setDisabled(true);
		spin_max->setDisabled(true);
	}
	updating_ui_ = false;
}

} // namespace plugin_surface_render_scalar

} // namespace schnapps
