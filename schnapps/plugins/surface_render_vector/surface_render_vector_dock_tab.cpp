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
	slider_vectorScaleFactor->setDisabled(true);
	combo_vectorColor->setDisabled(true);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(list_vectorVBO, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selected_vector_vbo_changed(QListWidgetItem*, QListWidgetItem*)));
	connect(list_vectorVBO, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vector_vbo_checked(QListWidgetItem*)));
	connect(slider_vectorScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vector_scale_factor_changed(int)));
	connect(combo_vectorColor, SIGNAL(currentIndexChanged(int)), this, SLOT(vector_color_changed(int)));

	selected_map_ = schnapps_->get_selected_map();
	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
	}

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

SurfaceRenderVector_DockTab::~SurfaceRenderVector_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_ui()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_ui()));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

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
		update_after_vector_vbo_changed();
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
			if (view && map)
			{
				cgogn::rendering::VBO* vbo = map->get_vbo(item->text());
				plugin_->set_vector_scale_factor(view, map, vbo, float32(i) / 50.0f, false);
			}
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
			if (view && map)
			{
				cgogn::rendering::VBO* vbo = map->get_vbo(item->text());
				plugin_->set_vector_color(view, map, vbo, combo_vectorColor->color(), false);
			}
		}
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceRenderVector_DockTab::selected_view_changed(View* old, View* cur)
{
	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void SurfaceRenderVector_DockTab::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
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

void SurfaceRenderVector_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		combo_positionVBO->addItem(vbo_name);
		list_vectorVBO->addItem(vbo_name);
		QListWidgetItem* item = list_vectorVBO->item(list_vectorVBO->count() - 1);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setCheckState(Qt::Unchecked);
	}
}

void SurfaceRenderVector_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		int index = combo_positionVBO->findText(vbo_name);
		if (index > 0)
			combo_positionVBO->removeItem(index);

		QList<QListWidgetItem*> items = list_vectorVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
			delete items[0];
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void SurfaceRenderVector_DockTab::set_position_vbo(cgogn::rendering::VBO* vbo)
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

void SurfaceRenderVector_DockTab::add_vector_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	if (vbo)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		QList<QListWidgetItem*> items = list_vectorVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
		{
			items[0]->setCheckState(Qt::Checked);
			update_after_vector_vbo_changed();
		}
	}
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::remove_vector_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	if (vbo)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		QList<QListWidgetItem*> items = list_vectorVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
		{
			items[0]->setCheckState(Qt::Unchecked);
			update_after_vector_vbo_changed();
		}
	}
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::set_vector_size(cgogn::rendering::VBO* vbo, double d)
{
	updating_ui_ = true;
	if (vbo)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		QList<QListWidgetItem*> items = list_vectorVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
		{
			if (items[0]->checkState() == Qt::Checked)
				slider_vectorScaleFactor->setSliderPosition(d * 50.0f);
		}
	}
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::set_vector_color(cgogn::rendering::VBO* vbo, QColor c)
{
	updating_ui_ = true;
	if (vbo)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		QList<QListWidgetItem*> items = list_vectorVBO->findItems(vbo_name, Qt::MatchExactly);
		if (!items.empty())
		{
			if (items[0]->checkState() == Qt::Checked)
				combo_vectorColor->setColor(c);
		}
	}
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::refresh_ui()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (!map || !view)
		return;

	const MapParameters& p = plugin_->get_parameters(view, map);

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
			if (p.get_vector_vbo_index(vbo.get()) != UINT32_MAX)
				item->setCheckState(Qt::Checked);

			++i;
		}
	}

	slider_vectorScaleFactor->setDisabled(true);
	combo_vectorColor->setDisabled(true);

	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void SurfaceRenderVector_DockTab::update_after_vector_vbo_changed()
{
	updating_ui_ = true;
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	const MapParameters& p = plugin_->get_parameters(view, map);

	QListWidgetItem* item = list_vectorVBO->currentItem();
	if (item && item->checkState() == Qt::Checked)
	{
		cgogn::rendering::VBO* vbo = map->get_vbo(item->text());
		uint32 vboindex = p.get_vector_vbo_index(vbo);
		if (vboindex != UINT32_MAX)
		{
			slider_vectorScaleFactor->setEnabled(true);
			slider_vectorScaleFactor->setSliderPosition(p.get_vector_scale_factor(vboindex) * 50.0f);
			combo_vectorColor->setEnabled(true);
			combo_vectorColor->setColor(p.get_vector_color(vboindex));
		}
		else
		{
			slider_vectorScaleFactor->setDisabled(true);
			combo_vectorColor->setDisabled(true);
		}
	}
	else
	{
		slider_vectorScaleFactor->setDisabled(true);
		combo_vectorColor->setDisabled(true);
	}
	updating_ui_ = false;
}

} // namespace plugin_surface_render_vector

} // namespace schnapps
