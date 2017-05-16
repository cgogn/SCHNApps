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

#include <surface_render_transp_dock_tab.h>
#include <surface_render_transp.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{
namespace plugin_surface_render_transp
{

SurfaceRenderTransp_DockTab::SurfaceRenderTransp_DockTab(SCHNApps* s, Plugin_SurfaceRenderTransp* p) :
	schnapps_(s),
	plugin_(p),
	current_color_dial_(0),
	updating_ui_(false)
{
	setupUi(this);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(combo_normalVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(normal_vbo_changed(int)));
	connect(group_faceShading, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(face_style_changed(QAbstractButton*)));

	color_dial_ = new QColorDialog(front_color_, nullptr);
	connect(frontColorButton, SIGNAL(clicked()), this, SLOT(front_color_clicked()));
	connect(backColorButton, SIGNAL(clicked()), this, SLOT(back_color_clicked()));
	connect(bothColorButton, SIGNAL(clicked()), this, SLOT(both_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));
	connect(opaqueSlider,SIGNAL(valueChanged(int)),this,SLOT(opaque_value_changed(int)));
}



void SurfaceRenderTransp_DockTab::position_vbo_changed(int index)
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

void SurfaceRenderTransp_DockTab::normal_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_normal_vbo(map->get_vbo(combo_normalVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRenderTransp_DockTab::face_style_changed(QAbstractButton* b)
{
	if (!updating_ui_)
	{
		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			if (radio_flatShading->isChecked())
				p.face_style_ = MapParameters::FLAT;
			else if (radio_phongShading->isChecked())
				p.face_style_ = MapParameters::PHONG;
			else if (radio_noneShading->isChecked())
				p.face_style_ = MapParameters::NONE;
			view->update();
		}
	}
}


void SurfaceRenderTransp_DockTab::front_color_clicked()
{
	current_color_dial_ = 3;
	color_dial_->show();
	color_dial_->setCurrentColor(front_color_);
}

void SurfaceRenderTransp_DockTab::back_color_clicked()
{
	current_color_dial_ = 4;
	color_dial_->show();
	color_dial_->setCurrentColor(back_color_);
}

void SurfaceRenderTransp_DockTab::both_color_clicked()
{
	current_color_dial_ = 5;
	color_dial_->show();
	color_dial_->setCurrentColor(front_color_);
}

void SurfaceRenderTransp_DockTab::color_selected()
{
	QColor col = color_dial_->currentColor();

	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	if (current_color_dial_ == 3)
	{
		front_color_ = QColor(col.red(),col.green(),col.blue(),opaqueSlider->value());
		frontColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_front_color(front_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 4)
	{
		back_color_ = QColor(col.red(),col.green(),col.blue(),opaqueSlider->value());
		backColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_back_color(back_color_);
			view->update();
		}
	}

	if (current_color_dial_ == 5)
	{
		front_color_ = QColor(col.red(),col.green(),col.blue(),opaqueSlider->value());
		back_color_ = front_color_;
		bothColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		frontColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		backColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_front_color(front_color_);
			p.set_back_color(back_color_);
			view->update();
		}
	}
}

void SurfaceRenderTransp_DockTab::opaque_value_changed(int v)
{
	if (!updating_ui_)
	{
		front_color_.setAlpha(opaqueSlider->value());
		back_color_.setAlpha(opaqueSlider->value());

		View* view = schnapps_->get_selected_view();
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (view && map)
		{
			MapParameters& p = plugin_->get_parameters(view, map);
			p.set_front_color(front_color_);
			p.set_back_color(back_color_);
			view->update();
		}
	}
}


void SurfaceRenderTransp_DockTab::add_position_vbo(QString name)
{
	updating_ui_ = true;
	combo_positionVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRenderTransp_DockTab::remove_position_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_positionVBO->currentIndex();
	int index = combo_positionVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_positionVBO->setCurrentIndex(0);
	combo_positionVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRenderTransp_DockTab::add_normal_vbo(QString name)
{
	updating_ui_ = true;
	combo_normalVBO->addItem(name);
	updating_ui_ = false;
}

void SurfaceRenderTransp_DockTab::remove_normal_vbo(QString name)
{
	updating_ui_ = true;
	int curIndex = combo_normalVBO->currentIndex();
	int index = combo_normalVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_normalVBO->setCurrentIndex(0);
	combo_normalVBO->removeItem(index);
	updating_ui_ = false;
}

void SurfaceRenderTransp_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	if (!map)
		return;

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_normalVBO->clear();
	combo_normalVBO->addItem("- select VBO -");

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

			++i;
		}
	}

	radio_flatShading->setChecked(p.face_style_ == MapParameters::FLAT);
	radio_phongShading->setChecked(p.face_style_ == MapParameters::PHONG);
	radio_noneShading->setChecked(p.face_style_ == MapParameters::NONE);

	front_color_ = p.get_front_color();
	frontColorButton->setStyleSheet("QPushButton { background-color:" + front_color_.name() + " }");
	bothColorButton->setStyleSheet("QPushButton { background-color:" + front_color_.name() + "}");

	back_color_ = p.get_back_color();
	backColorButton->setStyleSheet("QPushButton { background-color:" + back_color_.name() + " }");

	opaqueSlider->setValue(front_color_.alpha());

	updating_ui_ = false;
}

} // namespace plugin_surface_render
} // namespace schnapps
