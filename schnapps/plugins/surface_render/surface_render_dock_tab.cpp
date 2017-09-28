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

#include "surface_render_dock_tab.h"
#include "surface_render.h"

#ifdef USE_TRANSPARENCY
#include <schnapps/plugins/surface_render_transp/surface_render_transp_extern.h>
#endif

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_render
{

SurfaceRender_DockTab::SurfaceRender_DockTab(SCHNApps* s, Plugin_SurfaceRender* p) :
	schnapps_(s),
	plugin_(p),
	current_color_dial_(0),
	updating_ui_(false)
{
	setupUi(this);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(combo_normalVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_vbo_changed(int)));
	connect(combo_colorVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(color_vbo_changed(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(render_vertices_changed(bool)));
	connect(slider_verticesScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertices_scale_factor_changed(int)));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(render_faces_changed(bool)));
	connect(check_doubleSided, SIGNAL(toggled(bool)), this, SLOT(render_backfaces_changed(bool)));
	connect(group_faceShading, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(face_style_changed(QAbstractButton*)));
	connect(check_renderBoundary, SIGNAL(toggled(bool)), this, SLOT(render_boundary_changed(bool)));

	color_dial_ = new QColorDialog(front_color_, nullptr);
	connect(vertexColorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(edgeColorButton, SIGNAL(clicked()), this, SLOT(edge_color_clicked()));
	connect(frontColorButton, SIGNAL(clicked()), this, SLOT(front_color_clicked()));
	connect(backColorButton, SIGNAL(clicked()), this, SLOT(back_color_clicked()));
	connect(bothColorButton, SIGNAL(clicked()), this, SLOT(both_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));

	checkBox_transparency->setChecked(false);
	slider_transparency->setDisabled(true);
#ifdef USE_TRANSPARENCY
	connect(slider_transparency, SIGNAL(valueChanged(int)), this, SLOT(transparency_factor_changed(int)));
	connect(checkBox_transparency, SIGNAL(toggled(bool)), this, SLOT(transparency_enabled_changed(bool)));
#else
	checkBox_transparency->setDisabled(true);
#endif
}

void SurfaceRender_DockTab::position_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
			plugin_->set_position_vbo(schnapps_->get_selected_view(), map, map->get_vbo(combo_positionVBO->currentText()), false);
	}
}

void SurfaceRender_DockTab::normal_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
			plugin_->set_normal_vbo(schnapps_->get_selected_view(), map, map->get_vbo(combo_normalVBO->currentText()), false);
	}
}

void SurfaceRender_DockTab::color_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
			plugin_->set_color_vbo(schnapps_->get_selected_view(), map, map->get_vbo(combo_colorVBO->currentText()), false);
	}
}

void SurfaceRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_vertices(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void SurfaceRender_DockTab::vertices_scale_factor_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_vertex_scale_factor(schnapps_->get_selected_view(), schnapps_->get_selected_map(), i / 50.0, false);
}

void SurfaceRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_edges(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void SurfaceRender_DockTab::render_faces_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_faces(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void SurfaceRender_DockTab::render_backfaces_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_backfaces(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void SurfaceRender_DockTab::face_style_changed(QAbstractButton* b)
{
	if (!updating_ui_)
	{
		MapParameters::FaceShadingStyle fs;
		if (radio_flatShading->isChecked())
			fs = MapParameters::FLAT;
		else if (radio_phongShading->isChecked())
			fs = MapParameters::PHONG;
		plugin_->set_face_style(schnapps_->get_selected_view(), schnapps_->get_selected_map(), fs, false);
	}
}

void SurfaceRender_DockTab::render_boundary_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_boundary(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
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
	QColor col = color_dial_->currentColor();

	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	if (current_color_dial_ == 1)
	{
		vertex_color_ = col;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_vertex_color(view, map, vertex_color_, false);
	}

	if (current_color_dial_ == 2)
	{
		edge_color_ = col;
		edgeColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_edge_color(view, map, edge_color_, false);
	}

	if (current_color_dial_ == 3)
	{
		front_color_ = col;
		frontColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_front_color(view, map, front_color_, false);
	}

	if (current_color_dial_ == 4)
	{
		back_color_ = col;
		backColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_back_color(view, map, back_color_, false);
	}

	if (current_color_dial_ == 5)
	{
		front_color_ = col;
		back_color_ = col;
		bothColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		frontColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		backColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_front_color(view, map, front_color_, false);
		plugin_->set_back_color(view, map, back_color_, false);
	}
}

void SurfaceRender_DockTab::transparency_enabled_changed(bool b)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_)
		plugin_->set_transparency_enabled(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, true);
#endif
}

void SurfaceRender_DockTab::transparency_factor_changed(int n)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_)
		plugin_->set_transparency_factor(schnapps_->get_selected_view(), schnapps_->get_selected_map(), n, false);
#endif
}

void SurfaceRender_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	if (!map)
		return;

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_normalVBO->clear();
	combo_normalVBO->addItem("- select VBO -");

	combo_colorVBO->clear();
	combo_colorVBO->addItem("- select VBO -");

	unsigned int i = 1;
	for (const auto& vbo_it : map->get_vbo_set())
	{
		auto& vbo = vbo_it.second;
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			combo_normalVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_normal_vbo())
				combo_normalVBO->setCurrentIndex(i);

			combo_colorVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_color_vbo())
				combo_colorVBO->setCurrentIndex(i);

			++i;
		}
	}

	check_renderVertices->setChecked(p.render_vertices_);
	slider_verticesScaleFactor->setSliderPosition(p.get_vertex_scale_factor() * 50.0);
	check_renderEdges->setChecked(p.render_edges_);
	check_renderFaces->setChecked(p.render_faces_);
	check_doubleSided->setChecked(p.get_render_backfaces());
	radio_flatShading->setChecked(p.face_style_ == MapParameters::FLAT);
	radio_phongShading->setChecked(p.face_style_ == MapParameters::PHONG);
	check_renderBoundary->setChecked(p.render_boundary_);

	vertex_color_ = p.get_vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

	edge_color_ = p.get_edge_color();
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + edge_color_.name() + " }");

	front_color_ = p.get_front_color();
	frontColorButton->setStyleSheet("QPushButton { background-color:" + front_color_.name() + " }");
	bothColorButton->setStyleSheet("QPushButton { background-color:" + front_color_.name() + "}");

	back_color_ = p.get_back_color();
	backColorButton->setStyleSheet("QPushButton { background-color:" + back_color_.name() + " }");

#ifdef USE_TRANSPARENCY
	checkBox_transparency->setChecked(p.use_transparency_);
	slider_transparency->setValue(p.get_transparency_factor());
	slider_transparency->setEnabled(p.use_transparency_);
#endif

	updating_ui_ = false;
}

} // namespace plugin_surface_render

} // namespace schnapps
