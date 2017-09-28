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

#include <surface_render_vector_dock_tab.h>
#include <surface_render_vector.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_render_vector
{

SurfaceRenderVector_DockTab::SurfaceRenderVector_DockTab(SCHNApps* s, Plugin_SurfaceRenderVector* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);

	list_vectorVBO->setSelectionMode(QAbstractItemView::SingleSelection);
	slider_vectorsScaleFactor->setDisabled(true);
	combo_color->setDisabled(true);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(list_vectorVBO, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selected_vector_vbo_changed(QListWidgetItem*, QListWidgetItem*)));
	connect(list_vectorVBO, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vector_vbo_checked(QListWidgetItem*)));
	connect(slider_vectorsScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vector_scale_factor_changed(int)));
	connect(combo_color, SIGNAL(currentIndexChanged(int)), this, SLOT(vector_color_changed(int)));
}

void SurfaceRenderVector_DockTab::position_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
			plugin_->set_position_vbo(schnapps_->get_selected_view(), map, map->get_vbo(combo_positionVBO->currentText()), false);
	}
}

void SurfaceRenderVector_DockTab::selected_vector_vbo_changed(QListWidgetItem* item, QListWidgetItem* old)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			if ((item->checkState() == Qt::Checked))
			{
				cgogn::rendering::VBO* vbo = map->get_vbo(item->text());
				float32 sf = plugin_->get_vector_scale_factor(view, map, vbo);
				QColor c = plugin_->get_vector_color(view, map, vbo);
				slider_vectorsScaleFactor->setEnabled(true);
				slider_vectorsScaleFactor->setSliderPosition(sf * 50.0f);
				combo_color->setEnabled(true);
				combo_color->setColor(c);
			}
			else
			{
				slider_vectorsScaleFactor->setDisabled(true);
				combo_color->setDisabled(true);
			}
		}
	}
}

void SurfaceRenderVector_DockTab::vector_vbo_checked(QListWidgetItem* item)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
		{
			if (item->checkState() == Qt::Checked)
				plugin_->add_vector_vbo(schnapps_->get_selected_view(), map, map->get_vbo(item->text()), true);
			else
				plugin_->remove_vector_vbo(schnapps_->get_selected_view(), map, map->get_vbo(item->text()), true);
		}
	}
}

void SurfaceRenderVector_DockTab::vector_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		QListWidgetItem* item = list_vectorVBO->currentItem();
		if (item && item->checkState() == Qt::Checked)
		{
			View* view = schnapps_->get_selected_view();
			MapHandlerGen* map = schnapps_->get_selected_map();
			cgogn::rendering::VBO* vbo = map->get_vbo(item->text());
			float32 sf = plugin_->get_vector_scale_factor(view, map, vbo);
			float32 new_sf = float32(i) / 50.0f;
			if (fabs(sf - new_sf) > 0.01f)
				plugin_->set_vector_scale_factor(view, map, vbo, new_sf, false);
		}
	}
}

void SurfaceRenderVector_DockTab::vector_color_changed(int i)
{
	if (!updating_ui_)
	{
		QListWidgetItem* item = list_vectorVBO->currentItem();
		if (item && item->checkState() == Qt::Checked)
		{
			View* view = schnapps_->get_selected_view();
			MapHandlerGen* map = schnapps_->get_selected_map();
			cgogn::rendering::VBO* vbo = map->get_vbo(item->text());
			QColor color = plugin_->get_vector_color(view, map, vbo);
			if (color != combo_color->color())
				plugin_->set_vector_color(view, map, vbo, combo_color->color(), false);
		}
	}
}



void SurfaceRenderVector_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	list_vectorVBO->clear();

	unsigned int i = 1;
	for(auto& vbo_it : map->get_vbo_set())
	{
		auto& vbo = vbo_it.second;
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			list_vectorVBO->addItem(QString::fromStdString(vbo->name()));
			QListWidgetItem* item = list_vectorVBO->item(list_vectorVBO->count() - 1);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			item->setCheckState(Qt::Unchecked);
			if (p.get_vector_vbo_index(vbo.get())!= UINT32_MAX)
				list_vectorVBO->item(i-1)->setCheckState(Qt::Checked);

			++i;
		}
	}

	slider_vectorsScaleFactor->setDisabled(true);
	combo_color->setDisabled(true);

	updating_ui_ = false;
}

} // namespace plugin_surface_render_vector

} // namespace schnapps
