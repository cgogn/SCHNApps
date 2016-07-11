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
				const MapParameters& p = plugin_->get_parameters(view, map);
				int idx = p.get_vector_vbo_index(map->get_vbo(item->text()));
				slider_vectorsScaleFactor->setEnabled(true);
				slider_vectorsScaleFactor->setSliderPosition(p.get_vector_scale_factor(idx) * 50.0f);
				combo_color->setEnabled(true);
				combo_color->setColor(p.get_vector_color(idx));
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
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			cgogn::rendering::VBO* vbo = map->get_vbo(item->text());

			if (item->checkState() == Qt::Checked)
			{
				updating_ui_ = true;
				p.add_vector_vbo(vbo);
				int idx = p.get_vector_vbo_index(vbo);
				if (list_vectorVBO->currentItem() != item)
					list_vectorVBO->setCurrentItem(item);
				slider_vectorsScaleFactor->setEnabled(true);
				slider_vectorsScaleFactor->setSliderPosition(p.get_vector_scale_factor(idx) * 50.0f);
				combo_color->setEnabled(true);
				combo_color->setColor(p.get_vector_color(idx));
				updating_ui_ = false;
			}
			else
			{
				p.remove_vector_vbo(vbo);
//				list_vectorVBO->setCurrentItem(item);
				list_vectorVBO->clearSelection();
				slider_vectorsScaleFactor->setDisabled(true);
				combo_color->setDisabled(true);
			}
			view->update();
		}
	}
}

void SurfaceRenderVector_DockTab::vector_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		QListWidgetItem* item = list_vectorVBO->currentItem();
		if (view && map && item)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			int idx = p.get_vector_vbo_index(map->get_vbo(item->text()));
			float scale = p.get_vector_scale_factor(idx);
			float new_scale = float(i) / 50.0f;
			if (fabs(scale - new_scale) > 0.01f)
			{
				p.set_vector_scale_factor(idx, new_scale);
				view->update();
			}
		}
	}
}

void SurfaceRenderVector_DockTab::vector_color_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		QListWidgetItem* item = list_vectorVBO->currentItem();
		if (view && map && item)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			int idx = p.get_vector_vbo_index(map->get_vbo(item->text()));
			const QColor& col = p.get_vector_color(idx);
			if (col != combo_color->color())
			{
				p.set_vector_color(idx, combo_color->color());
				view->update();
			}
		}
	}
}





void SurfaceRenderVector_DockTab::add_position_vbo(QString name)
{
	updating_ui_ = true;
	combo_positionVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::remove_position_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_positionVBO->currentIndex();
	int index = combo_positionVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_positionVBO->setCurrentIndex(0);
	combo_positionVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::add_vector_vbo(QString name)
{
	updating_ui_ = true;
	list_vectorVBO->addItem(name);
	QListWidgetItem* item = list_vectorVBO->item(list_vectorVBO->count() - 1);
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	item->setCheckState(Qt::Unchecked);
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::remove_vector_vbo(QString name)
{
	updating_ui_ = true;
	QList<QListWidgetItem*> vbo = list_vectorVBO->findItems(name, Qt::MatchExactly);
	if (!vbo.empty())
		delete vbo[0];
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	list_vectorVBO->clear();
	list_vectorVBO->clearSelection();

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
			if (p.get_vector_vbo_index(vbo.get()) >= 0)
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
