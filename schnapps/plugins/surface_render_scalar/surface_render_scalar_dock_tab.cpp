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
	connect(slider_expansion, SIGNAL(valueChanged(int)), this, SLOT(expansion_changed(int)));
	connect(check_showIsoLines, SIGNAL(toggled(bool)), this, SLOT(show_iso_lines_changed(bool)));
}

void SurfaceRenderScalar_DockTab::position_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_position_vbo(map->get_vbo(combo_positionVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRenderScalar_DockTab::selected_scalar_vbo_changed(QListWidgetItem* item, QListWidgetItem* old)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			updating_ui_ = true;
			p.set_scalar_vbo(map->get_vbo(item->text()));
			combo_colorMap->setEnabled(true);
			combo_colorMap->setCurrentIndex(p.get_color_map());
			slider_expansion->setEnabled(true);
			slider_expansion->setSliderPosition(p.get_expansion());
			updating_ui_ = false;
			view->update();
		}
	}
}

void SurfaceRenderScalar_DockTab::color_map_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_color_map(index);
			view->update();
		}
	}
}

void SurfaceRenderScalar_DockTab::expansion_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_expansion(i);
			view->update();
		}
	}
}

void SurfaceRenderScalar_DockTab::show_iso_lines_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_show_iso_lines(b);
			view->update();
		}
	}
}





void SurfaceRenderScalar_DockTab::add_position_vbo(QString name)
{
	updating_ui_ = true;
	combo_positionVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::remove_position_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_positionVBO->currentIndex();
	int index = combo_positionVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_positionVBO->setCurrentIndex(0);
	combo_positionVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::add_scalar_vbo(QString name)
{
	updating_ui_ = true;
	list_scalarVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::remove_scalar_vbo(QString name)
{
	updating_ui_ = true;
	QList<QListWidgetItem*> vbo = list_scalarVBO->findItems(name, Qt::MatchExactly);
	if (!vbo.empty())
		delete vbo[0];
	updating_ui_ = false;
}

void SurfaceRenderScalar_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	list_scalarVBO->clear();
	list_scalarVBO->clearSelection();

	unsigned int i = 1;
	unsigned int j = 0;
	for(auto& vbo_it : map->get_vbo_set())
	{
		auto& vbo = vbo_it.second;
		const uint32 dimension = vbo->vector_dimension();
		if (dimension == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->get_name()));
			if (vbo.get() == p.get_position_vbo())
				combo_positionVBO->setCurrentIndex(i);
			++i;
		}
		if (dimension == 1)
		{
			list_scalarVBO->addItem(QString::fromStdString(vbo->get_name()));
			if (vbo.get() == p.get_scalar_vbo())
				list_scalarVBO->item(j)->setSelected(true);
			++j;
		}
	}

	combo_colorMap->setDisabled(true);
	slider_expansion->setDisabled(true);
	check_showIsoLines->setChecked(p.get_show_iso_lines());

	updating_ui_ = false;
}

} // namespace schnapps
