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

#include <surface_render_dock_tab.h>
#include <surface_render.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
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
	connect(slider_verticesScaleFactor, SIGNAL(sliderPressed()), this, SLOT(vertices_scale_factor_pressed()));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(render_faces_changed(bool)));
	connect(group_faceShading, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(face_style_changed(QAbstractButton*)));
	connect(check_renderBoundary, SIGNAL(toggled(bool)), this, SLOT(render_boundary_changed(bool)));
	connect(check_doubleSided, SIGNAL(toggled(bool)), this, SLOT(render_backface_changed(bool)));

	color_dial_ = new QColorDialog(diffuse_color_, nullptr);
	connect(dcolorButton, SIGNAL(clicked()), this, SLOT(diffuse_color_clicked()));
	connect(scolorButton, SIGNAL(clicked()), this, SLOT(simple_color_clicked()));
	connect(vcolorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(bfcolorButton, SIGNAL(clicked()), this, SLOT(back_color_clicked()));
	connect(bothcolorButton, SIGNAL(clicked()), this, SLOT(both_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));
}





void SurfaceRender_DockTab::position_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));
			plugin_->parameter_set_[view][map].set_position_vbo(map->get_vbo(combo_positionVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRender_DockTab::normal_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_normal_vbo(map->get_vbo(combo_normalVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRender_DockTab::color_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_color_vbo(map->get_vbo(combo_colorVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			if (b)
				plugin_->parameter_set_[view][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));

			plugin_->parameter_set_[view][map].renderVertices = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::vertices_scale_factor_pressed()
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));
		}
	}
}

void SurfaceRender_DockTab::vertices_scale_factor_changed(int i)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].verticesScaleFactor = i / 50.0;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].renderEdges = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::render_faces_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].renderFaces = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::face_style_changed(QAbstractButton* b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			if (radio_flatShading->isChecked())
				plugin_->parameter_set_[view][map].faceStyle = MapParameters::FLAT;
			else if (radio_phongShading->isChecked())
				plugin_->parameter_set_[view][map].faceStyle = MapParameters::PHONG;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::render_boundary_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].renderBoundary = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::render_backface_changed(bool b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].renderBackfaces = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::diffuse_color_clicked()
{
	current_color_dial_ = 1;
	color_dial_->show();
	color_dial_->setCurrentColor(diffuse_color_);
}

void SurfaceRender_DockTab::simple_color_clicked()
{
	current_color_dial_ = 2;
	color_dial_->show();
	color_dial_->setCurrentColor(simple_color_);
}

void SurfaceRender_DockTab::vertex_color_clicked()
{
	current_color_dial_ = 3;
	color_dial_->show();
	color_dial_->setCurrentColor(vertex_color_);
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
	color_dial_->setCurrentColor(diffuse_color_);
}

void SurfaceRender_DockTab::color_selected()
{
	QColor col = color_dial_->currentColor();
	if (current_color_dial_ == 1)
	{
		diffuse_color_ = col;
		dcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		bothcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_diffuse_color(diffuse_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 2)
	{
		simple_color_ = col;
		scolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_simple_color(simple_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 3)
	{
		vertex_color_ = col;
		vcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_vertex_color(vertex_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 4)
	{
		back_color_ = col;
		bfcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_back_color(back_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 5)
	{
		back_color_ = col;
		bfcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		diffuse_color_ = col;
		dcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		bothcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			plugin_->parameter_set_[view][map].set_back_color(back_color_);
			plugin_->parameter_set_[view][map].set_diffuse_color(diffuse_color_);
			view->update();
		}
	}
}





void SurfaceRender_DockTab::add_position_vbo(QString name)
{
	updating_ui_ = true;
	combo_positionVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::remove_position_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_positionVBO->currentIndex();
	int index = combo_positionVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_positionVBO->setCurrentIndex(0);
	combo_positionVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::add_normal_vbo(QString name)
{
	updating_ui_ = true;
	combo_normalVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::remove_normal_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_normalVBO->currentIndex();
	int index = combo_normalVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_normalVBO->setCurrentIndex(0);
	combo_normalVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::add_color_vbo(QString name)
{
	updating_ui_ = true;
	combo_colorVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::remove_color_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_colorVBO->currentIndex();
	int index = combo_colorVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_colorVBO->setCurrentIndex(0);
	combo_colorVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRender_DockTab::update_map_parameters()
{
	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_normalVBO->clear();
	combo_normalVBO->addItem("- select VBO -");

	combo_colorVBO->clear();
	combo_colorVBO->addItem("- select VBO -");

	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	if (view && map)
	{
		const MapParameters& p = plugin_->parameter_set_[view][map];

		unsigned int i = 1;
		foreach(cgogn::rendering::VBO* vbo, map->get_vbo_set().values())
		{
			if (vbo->vector_dimension() == 3)
			{
				combo_positionVBO->addItem(QString::fromStdString(vbo->get_name()));
				if (vbo == p.get_position_vbo())
					combo_positionVBO->setCurrentIndex(i);

				combo_normalVBO->addItem(QString::fromStdString(vbo->get_name()));
				if (vbo == p.get_normal_vbo())
					combo_normalVBO->setCurrentIndex(i);

				combo_colorVBO->addItem(QString::fromStdString(vbo->get_name()));
				if (vbo == p.get_color_vbo())
					combo_colorVBO->setCurrentIndex(i);

				++i;
			}
		}

		check_renderVertices->setChecked(p.renderVertices);
		slider_verticesScaleFactor->setSliderPosition(p.verticesScaleFactor * 50.0);
		check_renderEdges->setChecked(p.renderEdges);
		check_renderFaces->setChecked(p.renderFaces);
		radio_flatShading->setChecked(p.faceStyle == MapParameters::FLAT);
		radio_phongShading->setChecked(p.faceStyle == MapParameters::PHONG);

		diffuse_color_ = p.get_diffuse_color();
		dcolorButton->setStyleSheet("QPushButton { background-color:" + diffuse_color_.name() + " }");
		bothcolorButton->setStyleSheet("QPushButton { background-color:" + diffuse_color_.name() + "}");

		simple_color_ = p.get_simple_color();
		scolorButton->setStyleSheet("QPushButton { background-color:" + simple_color_.name() + " }");

		vertex_color_ = p.get_vertex_color();
		vcolorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

		back_color_ = p.get_back_color();
		bfcolorButton->setStyleSheet("QPushButton { background-color:" + back_color_.name() + " }");
	}

	updating_ui_ = false;
}

} // namespace schnapps
