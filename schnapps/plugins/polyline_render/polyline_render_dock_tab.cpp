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

#include <schnapps/plugins/polyline_render/polyline_render_dock_tab.h>
#include <schnapps/plugins/polyline_render/polyline_render.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_polyline_render
{

PolylineRender_DockTab::PolylineRender_DockTab(SCHNApps* s, Plugin_PolylineRender* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap_provider_(nullptr),
	selected_object_(nullptr),
	updating_ui_(false),
	color_dial_(nullptr),
	current_color_dial_(0)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_object_changed()));

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(combo_colorVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(color_vbo_changed(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(render_vertices_changed(bool)));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));

	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));

	color_dial_ = new QColorDialog(vertex_color_, nullptr);
	connect(vertexColorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(edgeColorButton, SIGNAL(clicked()), this, SLOT(edge_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));

	View* v = schnapps_->selected_view();
	connect(v, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(v, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	for (Object* o : v->linked_objects())
		object_linked(o);

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
}

PolylineRender_DockTab::~PolylineRender_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void PolylineRender_DockTab::selected_object_changed()
{
	if (selected_object_)
	{
		CMap1Handler* mh = qobject_cast<CMap1Handler*>(selected_object_);
		UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(selected_object_);
		if (mh)
		{
			disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_added(cgogn::rendering::VBO*)));
			disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_removed(cgogn::rendering::VBO*)));
		}
		if (ugh)
		{
			disconnect(ugh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_added(cgogn::rendering::VBO*)));
			disconnect(ugh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_removed(cgogn::rendering::VBO*)));
		}
	}

	selected_object_ = nullptr;

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& object_name = currentItems[0]->text();
		CMap1Handler* mh = plugin_cmap_provider_->cmap1(object_name);
		UndirectedGraphHandler* ugh = plugin_cmap_provider_->undirected_graph(object_name);
		if (mh) selected_object_ = mh;
		if (ugh) selected_object_ = ugh;
	}

	if (selected_object_)
	{
		CMap1Handler* mh = qobject_cast<CMap1Handler*>(selected_object_);
		UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(selected_object_);
		if (mh)
		{
			connect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
			connect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		}
		if (ugh)
		{
			connect(ugh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
			connect(ugh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		}
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void PolylineRender_DockTab::position_vbo_changed(int)
{
	if (!updating_ui_ && selected_object_)
	{
		CMap1Handler* mh = qobject_cast<CMap1Handler*>(selected_object_);
		UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(selected_object_);
		if (mh)
			plugin_->set_position_vbo(schnapps_->selected_view(), selected_object_, mh->vbo(combo_positionVBO->currentText()), false);
		if (ugh)
			plugin_->set_position_vbo(schnapps_->selected_view(), selected_object_, ugh->vbo(combo_positionVBO->currentText()), false);
	}
}

void PolylineRender_DockTab::color_vbo_changed(int)
{
	if (!updating_ui_ && selected_object_)
	{
		CMap1Handler* mh = qobject_cast<CMap1Handler*>(selected_object_);
		UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(selected_object_);
		if (mh)
			plugin_->set_color_vbo(schnapps_->selected_view(), selected_object_, mh->vbo(combo_colorVBO->currentText()), false);
		if (ugh)
			plugin_->set_color_vbo(schnapps_->selected_view(), selected_object_, ugh->vbo(combo_colorVBO->currentText()), false);
	}
}

void PolylineRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_ && selected_object_)
		plugin_->set_render_vertices(schnapps_->selected_view(), selected_object_, b, false);
}

void PolylineRender_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_ && selected_object_)
		plugin_->set_vertex_scale_factor(schnapps_->selected_view(), selected_object_, i / 50.0, false);
}

void PolylineRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_ && selected_object_)
		plugin_->set_render_edges(schnapps_->selected_view(), selected_object_, b, false);
}

void PolylineRender_DockTab::vertex_color_clicked()
{
	current_color_dial_ = 1;
	color_dial_->show();
	color_dial_->setCurrentColor(vertex_color_);
}

void PolylineRender_DockTab::edge_color_clicked()
{
	current_color_dial_ = 2;
	color_dial_->show();
	color_dial_->setCurrentColor(edge_color_);
}

void PolylineRender_DockTab::color_selected()
{
	QColor color = color_dial_->currentColor();

	View* view = schnapps_->selected_view();
	Object* o = selected_object_;

	if (current_color_dial_ == 1)
	{
		vertex_color_ = color;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_vertex_color(view, o, vertex_color_, false);
	}

	if (current_color_dial_ == 2)
	{
		edge_color_ = color;
		edgeColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_edge_color(view, o, edge_color_, false);
	}

}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void PolylineRender_DockTab::selected_view_changed(View* old, View* cur)
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

void PolylineRender_DockTab::object_linked(Object* o)
{
	CMap1Handler* mh = qobject_cast<CMap1Handler*>(o);
	UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(o);
	if (mh || ugh)
		list_maps->addItem(o->name());
}

void PolylineRender_DockTab::object_unlinked(Object* o)
{
	CMap1Handler* mh = qobject_cast<CMap1Handler*>(o);
	UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(o);
	if (mh || ugh)
	{
		if (selected_object_ == o)
		{
			if (mh)
			{
				disconnect(mh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_added(cgogn::rendering::VBO*)));
				disconnect(mh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_removed(cgogn::rendering::VBO*)));
			}
			if (ugh)
			{
				disconnect(ugh, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_added(cgogn::rendering::VBO*)));
				disconnect(ugh, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_object_vbo_removed(cgogn::rendering::VBO*)));
			}
			selected_object_ = nullptr;
		}

		QList<QListWidgetItem*> items = list_maps->findItems(o->name(), Qt::MatchExactly);
		if (!items.empty())
		{
			updating_ui_ = true;
			delete items[0];
			updating_ui_ = false;
		}
	}
}

/*****************************************************************************/
// slots called from CMap1Handler signals
/*****************************************************************************/

void PolylineRender_DockTab::selected_object_vbo_added(cgogn::rendering::VBO* vbo)
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

void PolylineRender_DockTab::selected_object_vbo_removed(cgogn::rendering::VBO* vbo)
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

void PolylineRender_DockTab::set_position_vbo(cgogn::rendering::VBO* vbo)
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

void PolylineRender_DockTab::set_color_vbo(cgogn::rendering::VBO* vbo)
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

void PolylineRender_DockTab::set_render_vertices(bool b)
{
	updating_ui_ = true;
	check_renderVertices->setChecked(b);
	updating_ui_ = false;
}

void PolylineRender_DockTab::set_render_edges(bool b)
{
	updating_ui_ = true;
	check_renderEdges->setChecked(b);
	updating_ui_ = false;
}

void PolylineRender_DockTab::set_vertex_color(const QColor& color)
{
	updating_ui_ = true;
	vertex_color_ = color;
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void PolylineRender_DockTab::set_edge_color(const QColor& color)
{
	updating_ui_ = true;
	edge_color_ = color;
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void PolylineRender_DockTab::set_vertex_scale_factor(float sf)
{
	updating_ui_ = true;
	slider_vertexScaleFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void PolylineRender_DockTab::refresh_ui()
{
	Object* o = selected_object_;
	View* view = schnapps_->selected_view();

	if (!o || !view)
		return;

	const MapParameters& p = plugin_->parameters(view, o);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_colorVBO->clear();
	combo_colorVBO->addItem("- select VBO -");

	CMap1Handler* mh = qobject_cast<CMap1Handler*>(o);
	UndirectedGraphHandler* ugh = qobject_cast<UndirectedGraphHandler*>(o);

	int32 i = 1;
	auto add_vbo = [&] (cgogn::rendering::VBO* vbo)
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
	};

	if (mh)
		mh->foreach_vbo(add_vbo);
	if (ugh)
		ugh->foreach_vbo(add_vbo);

	check_renderVertices->setChecked(p.render_vertices());
	slider_vertexScaleFactor->setSliderPosition(p.vertex_scale_factor() * 50.0);
	check_renderEdges->setChecked(p.render_edges());
	vertex_color_ = p.vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");
	edge_color_ = p.edge_color();
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + edge_color_.name() + " }");

	updating_ui_ = false;
}

} // namespace plugin_polyline_render

} // namespace schnapps
