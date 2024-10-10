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

#include <schnapps/plugins/surface_render/surface_render_dock_tab.h>
#include <schnapps/plugins/surface_render/surface_render.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_render
{

SurfaceRender_DockTab::SurfaceRender_DockTab(SCHNApps* s, Plugin_SurfaceRender* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap_provider_(nullptr),
	selected_map_(nullptr),
	updating_ui_(false),
	color_dial_(nullptr),
	current_color_dial_(0)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(combo_normalVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_vbo_changed(int)));
	connect(combo_colorVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(color_vbo_changed(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(render_vertices_changed(bool)));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(render_faces_changed(bool)));
	connect(check_renderBackFaces, SIGNAL(toggled(bool)), this, SLOT(render_backfaces_changed(bool)));
	connect(group_faceStyle, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(face_style_changed(QAbstractButton*)));
	connect(check_renderBoundary, SIGNAL(toggled(bool)), this, SLOT(render_boundary_changed(bool)));
	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));

	color_dial_ = new QColorDialog(front_color_, nullptr);
	connect(vertexColorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(edgeColorButton, SIGNAL(clicked()), this, SLOT(edge_color_clicked()));
	connect(frontColorButton, SIGNAL(clicked()), this, SLOT(front_color_clicked()));
	connect(backColorButton, SIGNAL(clicked()), this, SLOT(back_color_clicked()));
	connect(bothColorButton, SIGNAL(clicked()), this, SLOT(both_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));

	check_useTransparency->setChecked(false);
	slider_transparency->setDisabled(true);
#ifdef USE_TRANSPARENCY
	connect(check_useTransparency, SIGNAL(toggled(bool)), this, SLOT(transparency_enabled_changed(bool)));
	connect(slider_transparency, SIGNAL(valueChanged(int)), this, SLOT(transparency_factor_changed(int)));
#else
	check_useTransparency->setDisabled(true);
#endif

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));

	View* v = schnapps_->selected_view();
	connect(v, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(v, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	for (Object* o : v->linked_objects())
		object_linked(o);

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
}

SurfaceRender_DockTab::~SurfaceRender_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void SurfaceRender_DockTab::selected_map_changed()
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
		selected_map_ = plugin_cmap_provider_->cmap2(map_name);
	}

	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void SurfaceRender_DockTab::position_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_position_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_positionVBO->currentText()), false);
}

void SurfaceRender_DockTab::normal_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_normal_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_normalVBO->currentText()), false);
}

void SurfaceRender_DockTab::color_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_color_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_colorVBO->currentText()), false);
}

void SurfaceRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_vertices(schnapps_->selected_view(), selected_map_, b, false);
}

void SurfaceRender_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_vertex_scale_factor(schnapps_->selected_view(), selected_map_, i / 50.0, false);
}

void SurfaceRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_edges(schnapps_->selected_view(), selected_map_, b, false);
}

void SurfaceRender_DockTab::render_faces_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_faces(schnapps_->selected_view(), selected_map_, b, false);
}

void SurfaceRender_DockTab::render_backfaces_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_backfaces(schnapps_->selected_view(), selected_map_, b, false);
}

void SurfaceRender_DockTab::face_style_changed(QAbstractButton*)
{
	if (!updating_ui_ && selected_map_)
	{
		MapParameters::FaceShadingStyle fs;
		if (radio_flatShading->isChecked())
			fs = MapParameters::FLAT;
		else if (radio_phongShading->isChecked())
			fs = MapParameters::PHONG;
		plugin_->set_face_style(schnapps_->selected_view(), selected_map_, fs, false);
	}
}

void SurfaceRender_DockTab::render_boundary_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_boundary(schnapps_->selected_view(), selected_map_, b, false);
}

void SurfaceRender_DockTab::transparency_enabled_changed(bool b)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_ && selected_map_)
	{
		plugin_->set_transparency_enabled(schnapps_->selected_view(), selected_map_, b, false);
		update_after_use_transparency_changed();
	}
#endif
}

void SurfaceRender_DockTab::transparency_factor_changed(int n)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_ && selected_map_)
		plugin_->set_transparency_factor(schnapps_->selected_view(), selected_map_, n, false);
#endif
}

void SurfaceRender_DockTab::vertex_color_clicked()
{
	current_color_dial_ = 1;
	color_dial_->show();
	color_dial_->setCurrentColor(vertex_color_);
}

void SurfaceRender_DockTab::edge_color_clicked()
{
	current_color_dial_ = 2;
	color_dial_->show();
	color_dial_->setCurrentColor(edge_color_);
}

void SurfaceRender_DockTab::front_color_clicked()
{
	current_color_dial_ = 3;
	color_dial_->show();
	color_dial_->setCurrentColor(front_color_);
}

void SurfaceRender_DockTab::back_color_clicked()
{
	current_color_dial_ = 4;
	color_dial_->show();
	color_dial_->setCurrentColor(back_color_);
}

void SurfaceRender_DockTab::both_color_clicked()
{
	current_color_dial_ = 5;
	color_dial_->show();
	color_dial_->setCurrentColor(front_color_);
}

void SurfaceRender_DockTab::color_selected()
{
	QColor color = color_dial_->currentColor();

	View* view = schnapps_->selected_view();
	CMap2Handler* mh = selected_map_;

	if (current_color_dial_ == 1)
	{
		vertex_color_ = color;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_vertex_color(view, mh, vertex_color_, false);
	}

	if (current_color_dial_ == 2)
	{
		edge_color_ = color;
		edgeColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_edge_color(view, mh, edge_color_, false);
	}

	if (current_color_dial_ == 3)
	{
		front_color_ = color;
		frontColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_front_color(view, mh, front_color_, false);
	}

	if (current_color_dial_ == 4)
	{
		back_color_ = color;
		backColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_back_color(view, mh, back_color_, false);
	}

	if (current_color_dial_ == 5)
	{
		front_color_ = color;
		back_color_ = color;
		bothColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		frontColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		backColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
		plugin_->set_front_color(view, mh, front_color_, false);
		plugin_->set_back_color(view, mh, back_color_, false);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void SurfaceRender_DockTab::selected_view_changed(View* old, View* cur)
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

void SurfaceRender_DockTab::object_linked(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_linked(mh);
}

void SurfaceRender_DockTab::map_linked(CMap2Handler* mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void SurfaceRender_DockTab::object_unlinked(Object* o)
{
	CMap2Handler* mh = qobject_cast<CMap2Handler*>(o);
	if (mh)
		map_unlinked(mh);
}

void SurfaceRender_DockTab::map_unlinked(CMap2Handler* mh)
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
// slots called from CMap2Handler signals
/*****************************************************************************/

void SurfaceRender_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		updating_ui_ = true;
		combo_positionVBO->addItem(vbo_name);
		combo_normalVBO->addItem(vbo_name);
		combo_colorVBO->addItem(vbo_name);
		updating_ui_ = false;
	}
}

void SurfaceRender_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		updating_ui_ = true;
		int index = combo_positionVBO->findText(vbo_name, Qt::MatchExactly);
		if (index > 0)
			combo_positionVBO->removeItem(index);

		index = combo_normalVBO->findText(vbo_name, Qt::MatchExactly);
		if (index > 0)
			combo_normalVBO->removeItem(index);

		index = combo_colorVBO->findText(vbo_name, Qt::MatchExactly);
		if (index > 0)
			combo_colorVBO->removeItem(index);
		updating_ui_ = false;
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void SurfaceRender_DockTab::set_position_vbo(cgogn::rendering::VBO* vbo)
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

void SurfaceRender_DockTab::set_normal_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	if (vbo && vbo->vector_dimension() == 3)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		int index = combo_normalVBO->findText(vbo_name);
		if (index > 0)
			combo_normalVBO->setCurrentIndex(index);
	}
	else
		combo_positionVBO->setCurrentIndex(0);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_color_vbo(cgogn::rendering::VBO* vbo)
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

void SurfaceRender_DockTab::set_render_vertices(bool b)
{
	updating_ui_ = true;
	check_renderVertices->setChecked(b);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_render_edges(bool b)
{
	updating_ui_ = true;
	check_renderEdges->setChecked(b);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_render_faces(bool b)
{
	updating_ui_ = true;
	check_renderFaces->setChecked(b);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_render_backfaces(bool b)
{
	updating_ui_ = true;
	check_renderBackFaces->setChecked(b);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_face_style(MapParameters::FaceShadingStyle f)
{
	updating_ui_ = true;
	radio_flatShading->setChecked(f == MapParameters::FLAT);
	radio_phongShading->setChecked(f == MapParameters::PHONG);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_render_boundary(bool b)
{
	updating_ui_ = true;
	check_renderBoundary->setChecked(b);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_vertex_color(const QColor& color)
{
	updating_ui_ = true;
	vertex_color_ = color;
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_edge_color(const QColor& color)
{
	updating_ui_ = true;
	edge_color_ = color;
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_front_color(const QColor& color)
{
	updating_ui_ = true;
	front_color_ = color;
	frontColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_back_color(const QColor& color)
{
	updating_ui_ = true;
	back_color_ = color;
	backColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_vertex_scale_factor(float sf)
{
	updating_ui_ = true;
	slider_vertexScaleFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_transparency_enabled(bool b)
{
	updating_ui_ = true;
	check_useTransparency->setChecked(b);
	update_after_use_transparency_changed();
	updating_ui_ = false;
}

void SurfaceRender_DockTab::set_transparency_factor(int tf)
{
	updating_ui_ = true;
	slider_transparency->setValue(tf);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::refresh_ui()
{
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	if (!mh || !view)
		return;

	const MapParameters& p = plugin_->parameters(view, mh);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_normalVBO->clear();
	combo_normalVBO->addItem("- select VBO -");

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

			combo_normalVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo == p.normal_vbo())
				combo_normalVBO->setCurrentIndex(i);

			combo_colorVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo == p.color_vbo())
				combo_colorVBO->setCurrentIndex(i);

			++i;
		}
	});

	check_renderVertices->setChecked(p.render_vertices());
	slider_vertexScaleFactor->setSliderPosition(p.vertex_scale_factor() * 50.0);
	check_renderEdges->setChecked(p.render_edges());
	check_renderFaces->setChecked(p.render_faces());
	check_renderBackFaces->setChecked(p.render_backfaces());
	radio_flatShading->setChecked(p.face_style() == MapParameters::FLAT);
	radio_phongShading->setChecked(p.face_style() == MapParameters::PHONG);
	check_renderBoundary->setChecked(p.render_boundary());

	vertex_color_ = p.vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

	edge_color_ = p.edge_color();
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + edge_color_.name() + " }");

	front_color_ = p.front_color();
	frontColorButton->setStyleSheet("QPushButton { background-color:" + front_color_.name() + " }");
	bothColorButton->setStyleSheet("QPushButton { background-color:" + front_color_.name() + "}");

	back_color_ = p.back_color();
	backColorButton->setStyleSheet("QPushButton { background-color:" + back_color_.name() + " }");

#ifdef USE_TRANSPARENCY
	check_useTransparency->setChecked(p.transparency_enabled());
	slider_transparency->setValue(p.transparency_factor());
	slider_transparency->setEnabled(p.transparency_enabled());
#endif

	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void SurfaceRender_DockTab::update_after_use_transparency_changed()
{
	updating_ui_ = true;
	CMap2Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	const MapParameters& p = plugin_->parameters(view, mh);

	slider_transparency->setEnabled(p.transparency_enabled());
	if (p.transparency_enabled())
		slider_transparency->setValue(p.transparency_factor());

	updating_ui_ = false;
}

} // namespace plugin_surface_render

} // namespace schnapps
