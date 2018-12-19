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

#include <schnapps/plugins/point_set_render/point_set_render_dock_tab.h>
#include <schnapps/plugins/point_set_render/point_set_render.h>

#include <schnapps/plugins/cmap0_provider/cmap0_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_point_set_render
{

PointSetRender_DockTab::PointSetRender_DockTab(SCHNApps* s, Plugin_PointSetRender* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap0_provider_(nullptr),
	selected_map_(nullptr),
	updating_ui_(false),
	color_dial_(nullptr),
	current_color_dial_(0)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(combo_colorVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(color_vbo_changed(int)));
	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));

	color_dial_ = new QColorDialog(vertex_color_, nullptr);
	connect(vertexColorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));

	View* v = schnapps_->selected_view();
	connect(v, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(v, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	for (Object* o : v->linked_objects())
		object_linked(o);

	plugin_cmap0_provider_ = static_cast<plugin_cmap0_provider::Plugin_CMap0Provider*>(schnapps_->enable_plugin(plugin_cmap0_provider::Plugin_CMap0Provider::plugin_name()));
}

PointSetRender_DockTab::~PointSetRender_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void PointSetRender_DockTab::selected_map_changed()
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
		selected_map_ = plugin_cmap0_provider_->map(map_name);
	}

	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void PointSetRender_DockTab::position_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_position_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_positionVBO->currentText()), false);
}

void PointSetRender_DockTab::color_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_color_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_colorVBO->currentText()), false);
}

void PointSetRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_vertices(schnapps_->selected_view(), selected_map_, b, false);
}

void PointSetRender_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_vertex_scale_factor(schnapps_->selected_view(), selected_map_, i / 50.0, false);
}

void PointSetRender_DockTab::vertex_color_clicked()
{
	current_color_dial_ = 1;
	color_dial_->show();
	color_dial_->setCurrentColor(vertex_color_);
}

void PointSetRender_DockTab::color_selected()
{
	QColor color = color_dial_->currentColor();

	View* view = schnapps_->selected_view();
	CMap0Handler* mh = selected_map_;

	if (current_color_dial_ == 1)
	{
		vertex_color_ = color;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_vertex_color(view, mh, vertex_color_, false);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void PointSetRender_DockTab::selected_view_changed(View* old, View* cur)
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

void PointSetRender_DockTab::object_linked(Object* o)
{
	CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
	if (mh)
		map_linked(mh);
}

void PointSetRender_DockTab::map_linked(CMap0Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void PointSetRender_DockTab::object_unlinked(Object* o)
{
	CMap0Handler* mh = qobject_cast<CMap0Handler*>(o);
	if (mh)
		map_unlinked(mh);
}

void PointSetRender_DockTab::map_unlinked(CMap0Handler* mh)
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

/*****************************************************************************/
// slots called from CMap0Handler signals
/*****************************************************************************/

void PointSetRender_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		updating_ui_ = true;
		combo_positionVBO->addItem(vbo_name);
		combo_colorVBO->addItem(vbo_name);
		updating_ui_ = false;
	}
}

void PointSetRender_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		updating_ui_ = true;
		int index = combo_positionVBO->findText(vbo_name, Qt::MatchExactly);
		if (index > 0)
			combo_positionVBO->removeItem(index);

		index = combo_colorVBO->findText(vbo_name, Qt::MatchExactly);
		if (index > 0)
			combo_colorVBO->removeItem(index);
		updating_ui_ = false;
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void PointSetRender_DockTab::set_position_vbo(cgogn::rendering::VBO* vbo)
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

void PointSetRender_DockTab::set_color_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	if (vbo && vbo->vector_dimension() == 3)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		int index = combo_colorVBO->findText(vbo_name);
		if (index > 0)
			combo_colorVBO->setCurrentIndex(index);
	}
	else
		combo_positionVBO->setCurrentIndex(0);
	updating_ui_ = false;
}

void PointSetRender_DockTab::set_render_vertices(bool b)
{
	updating_ui_ = true;
	check_renderVertices->setChecked(b);
	updating_ui_ = false;
}

void PointSetRender_DockTab::set_vertex_color(const QColor& color)
{
	updating_ui_ = true;
	vertex_color_ = color;
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void PointSetRender_DockTab::set_vertex_scale_factor(float sf)
{
	updating_ui_ = true;
	slider_vertexScaleFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void PointSetRender_DockTab::refresh_ui()
{
	CMap0Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	if (!mh || !view)
		return;

	const MapParameters& p = plugin_->parameters(view, mh);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_colorVBO->clear();
	combo_colorVBO->addItem("- select VBO -");

	uint32 i = 1;
	mh->foreach_vbo([&] (cgogn::rendering::VBO* vbo)
	{
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo == p.position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			combo_colorVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo == p.color_vbo())
				combo_colorVBO->setCurrentIndex(i);

			++i;
		}
	});

	check_renderVertices->setChecked(p.render_vertices());
	slider_vertexScaleFactor->setSliderPosition(p.vertex_scale_factor() * 50.0);

	vertex_color_ = p.vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

	updating_ui_ = false;
}

} // namespace plugin_point_set_render

} // namespace schnapps
