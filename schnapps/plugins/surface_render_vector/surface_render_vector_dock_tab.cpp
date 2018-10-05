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

#include <schnapps/plugins/surface_render_vector/surface_render_vector_dock_tab.h>
#include <schnapps/plugins/surface_render_vector/surface_render_vector.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_render_vector
{

SurfaceRenderVector_DockTab::SurfaceRenderVector_DockTab(SCHNApps* s, Plugin_SurfaceRenderVector* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap2_provider_(nullptr),
	selected_map_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	list_vectorVBO->setSelectionMode(QAbstractItemView::SingleSelection);
	slider_vectorScaleFactor->setDisabled(true);
	combo_vectorColor->setDisabled(true);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(list_vectorVBO, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)), this, SLOT(selected_vector_vbo_changed(QListWidgetItem*, QListWidgetItem*)));
	connect(list_vectorVBO, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vector_vbo_checked(QListWidgetItem*)));
	connect(slider_vectorScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vector_scale_factor_changed(int)));
	connect(combo_vectorColor, SIGNAL(currentIndexChanged(int)), this, SLOT(vector_color_changed(int)));

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));;

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	schnapps_->foreach_object([this] (Object* o)
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			map_added(mh);
	});

	plugin_cmap2_provider_ = reinterpret_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));
}

SurfaceRenderVector_DockTab::~SurfaceRenderVector_DockTab()
{
	disconnect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	disconnect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void SurfaceRenderVector_DockTab::selected_map_changed()
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
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
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void SurfaceRenderVector_DockTab::position_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_position_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_positionVBO->currentText()), false);
}

void SurfaceRenderVector_DockTab::selected_vector_vbo_changed(QListWidgetItem*, QListWidgetItem*)
{
	if (!updating_ui_)
		update_after_vector_vbo_changed();
}

void SurfaceRenderVector_DockTab::vector_vbo_checked(QListWidgetItem* item)
{
	if (!updating_ui_ && selected_map_)
	{
		if (item->checkState() == Qt::Checked)
			plugin_->add_vector_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(item->text()), true);
		else
			plugin_->remove_vector_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(item->text()), true);
	}
}

void SurfaceRenderVector_DockTab::vector_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		QListWidgetItem* item = list_vectorVBO->currentItem();
		if (item && item->checkState() == Qt::Checked)
		{
			View* view = schnapps_->selected_view();
			if (view && selected_map_)
			{
				cgogn::rendering::VBO* vbo = selected_map_->vbo(item->text());
				plugin_->set_vector_scale_factor(view, selected_map_, vbo, float32(i) / 50.0f, false);
			}
		}
	}
}

void SurfaceRenderVector_DockTab::vector_color_changed(int)
{
	if (!updating_ui_)
	{
		QListWidgetItem* item = list_vectorVBO->currentItem();
		if (item && item->checkState() == Qt::Checked)
		{
			View* view = schnapps_->selected_view();
			if (view && selected_map_)
			{
				cgogn::rendering::VBO* vbo = selected_map_->vbo(item->text());
				plugin_->set_vector_color(view, selected_map_, vbo, combo_vectorColor->color(), false);
			}
		}
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceRenderVector_DockTab::object_added(Object* o)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		map_added(mh);
}

void SurfaceRenderVector_DockTab::map_added(CMap2Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void SurfaceRenderVector_DockTab::object_removed(Object* o)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		map_removed(mh);
}

void SurfaceRenderVector_DockTab::map_removed(CMap2Handler* mh)
{
	if (selected_map_ == mh)
	{
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
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

void SurfaceRenderVector_DockTab::selected_view_changed(View*, View*)
{
	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from CMap2Handler signals
/*****************************************************************************/

void SurfaceRenderVector_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		combo_positionVBO->addItem(vbo_name);
		list_vectorVBO->addItem(vbo_name);
		QListWidgetItem* item = list_vectorVBO->item(list_vectorVBO->count() - 1);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setCheckState(Qt::Unchecked);
	}
	updating_ui_ = false;
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
	if (vbo && vbo->vector_dimension() == 3)
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
	if (vbo && vbo->vector_dimension() == 3)
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
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	if (!mh || !view)
		return;

	const MapParameters& p = plugin_->parameters(view, mh);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	list_vectorVBO->clear();

	unsigned int i = 1;
	mh->foreach_vbo([&] (cgogn::rendering::VBO* vbo)
	{
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo == p.position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			list_vectorVBO->addItem(QString::fromStdString(vbo->name()));
			QListWidgetItem* item = list_vectorVBO->item(list_vectorVBO->count() - 1);
			item->setFlags(item->flags() | Qt::ItemIsEditable);
			item->setCheckState(Qt::Unchecked);
			if (p.vector_vbo_index(vbo) != UINT32_MAX)
				item->setCheckState(Qt::Checked);

			++i;
		}
	});

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
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	const MapParameters& p = plugin_->parameters(view, mh);

	QListWidgetItem* item = list_vectorVBO->currentItem();
	if (item && item->checkState() == Qt::Checked)
	{
		cgogn::rendering::VBO* vbo = mh->vbo(item->text());
		uint32 vboindex = p.vector_vbo_index(vbo);
		if (vboindex != UINT32_MAX)
		{
			slider_vectorScaleFactor->setEnabled(true);
			slider_vectorScaleFactor->setSliderPosition(p.vector_scale_factor(vboindex) * 50.0f);
			combo_vectorColor->setEnabled(true);
			combo_vectorColor->setColor(p.vector_color(vboindex));
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
