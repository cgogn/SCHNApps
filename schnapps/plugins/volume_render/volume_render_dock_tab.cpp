/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Render                                                         *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* Inspired by the surface render plugin                                        *
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

#define SCHNAPPS_PLUGIN_VOLUME_RENDER_DLL_EXPORT

#include <volume_render_dock_tab.h>
#include <volume_render.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_volume_render
{

VolumeRender_DockTab::VolumeRender_DockTab(SCHNApps* s, Plugin_VolumeRender* p) :
	schnapps_(s),
	plugin_(p),
	current_color_dial_(0),
	updating_ui_(false)
{
	setupUi(this);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(render_vertices_changed(bool)));
	connect(slider_verticesScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertices_scale_factor_changed(int)));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(render_faces_changed(bool)));
	connect(check_renderBoundary, SIGNAL(toggled(bool)), this, SLOT(render_boundary_changed(bool)));
	connect(sliderExplodeVolumes, SIGNAL(valueChanged(int)), this, SLOT(explode_volumes_changed(int)));
	connect(check_clippingPlane, SIGNAL(toggled(bool)), this, SLOT(apply_clipping_plane_changed(bool)));

	color_dial_ = new QColorDialog(face_color_, nullptr);
	connect(vertexColorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(edgeColorButton, SIGNAL(clicked()), this, SLOT(edge_color_clicked()));
	connect(faceColorButton, SIGNAL(clicked()), this, SLOT(face_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));
}





void VolumeRender_DockTab::position_vbo_changed(int index)
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

void VolumeRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.render_vertices_ = b;
			view->update();
		}
	}
}

void VolumeRender_DockTab::vertices_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_vertex_scale_factor(i / 50.0);
			view->update();
		}
	}
}

void VolumeRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.render_edges_ = b;
			view->update();
		}
	}
}

void VolumeRender_DockTab::render_faces_changed(bool b)
{
	if (!updating_ui_)
	{
		this->sliderExplodeVolumes->setEnabled(b);
		this->labelExplodVolumes->setEnabled(b);
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.render_faces_ = b;
			view->update();
		}
	}
}

void VolumeRender_DockTab::render_boundary_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.render_boundary_ = b;
			view->update();
		}
	}
}

void VolumeRender_DockTab::explode_volumes_changed(int fact)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_volume_explode_factor(float32(fact) / 100.0f);
			view->update();
		}
	}
}

void VolumeRender_DockTab::apply_clipping_plane_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_apply_clipping_plane(b);
			view->update();
		}
	}
}

void VolumeRender_DockTab::vertex_color_clicked()
{
	current_color_dial_ = 1;
	color_dial_->show();
	color_dial_->setCurrentColor(vertex_color_);
}

void VolumeRender_DockTab::edge_color_clicked()
{
	current_color_dial_ = 2;
	color_dial_->show();
	color_dial_->setCurrentColor(edge_color_);
}

void VolumeRender_DockTab::face_color_clicked()
{
	current_color_dial_ = 3;
	color_dial_->show();
	color_dial_->setCurrentColor(face_color_);
}


void VolumeRender_DockTab::color_selected()
{
	QColor col = color_dial_->currentColor();

	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	if (current_color_dial_ == 1)
	{
		vertex_color_ = col;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_vertex_color(vertex_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 2)
	{
		edge_color_ = col;
		edgeColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_edge_color(edge_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 3)
	{
		face_color_ = col;
		faceColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_face_color(face_color_);
			view->update();
		}
	}
}

void VolumeRender_DockTab::add_position_vbo(QString name)
{
	updating_ui_ = true;
	combo_positionVBO->addItem(name);
	updating_ui_ = false;
}

void VolumeRender_DockTab::remove_position_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_positionVBO->currentIndex();
	int index = combo_positionVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_positionVBO->setCurrentIndex(0);
	combo_positionVBO->removeItem(index);
	updating_ui_ = false;
}

void VolumeRender_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	if (!map)
		return;

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	unsigned int i = 1;
	for (const auto& vbo_it : map->get_vbo_set())
	{
		auto& vbo = vbo_it.second;
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			++i;
		}
	}

	check_renderVertices->setChecked(p.render_vertices_);
	slider_verticesScaleFactor->setSliderPosition(p.get_vertex_scale_factor() * 50.0);
	check_renderEdges->setChecked(p.render_edges_);
	check_renderFaces->setChecked(p.render_faces_);
	check_renderBoundary->setChecked(p.render_boundary_);
	sliderExplodeVolumes->setValue(std::round(100.0f*p.get_volume_explode_factor()));

	vertex_color_ = p.get_vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

	edge_color_ = p.get_edge_color();
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + edge_color_.name() + " }");

	face_color_ = p.get_face_color();
	faceColorButton->setStyleSheet("QPushButton { background-color:" + face_color_.name() + " }");

	updating_ui_ = false;
}

} // namespace plugin_volume_render
} // namespace schnapps
