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

#include <iostream>

#include <schnapps/core/view_button_area.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

//#include "Utils/Shaders/shaderWallPaper.h"

namespace schnapps
{

ViewButton::ViewButton(const QString& image, View* view) :
	img_(image),
	view_(view),
	wall_paper_(nullptr),
	wall_paper_renderer_(nullptr)
{
	wall_paper_ = new cgogn::rendering::WallPaper(QImage(image));
	wall_paper_renderer_ = wall_paper_->generate_renderer();
}

ViewButton::~ViewButton()
{
	delete wall_paper_renderer_;
	delete wall_paper_;
}

void ViewButton::click(int x, int y, int globalX, int globalY)
{
	emit(clicked(x, y, globalX, globalY));
}

void ViewButton::draw_at(int x, int y)
{
	QSize szw = view_->size();
	wall_paper_->set_local_position(float(x) / szw.width(), float(y) / szw.height(), float(ViewButton::SIZE)  / szw.width(), float(ViewButton::SIZE) / szw.height());
	wall_paper_renderer_->draw(view_);
}



ViewButtonArea::ViewButtonArea(View* view) :
	view_(view),
	form_(0,0,0,0)
{}

ViewButtonArea::~ViewButtonArea()
{}

void ViewButtonArea::add_button(ViewButton* button)
{
	if (!buttons_.contains(button))
	{
		form_.setWidth(form_.width() + ViewButton::SIZE + ViewButton::SPACE);
		form_.moveTopLeft(QPoint(form_.x() - ViewButton::SIZE - ViewButton::SPACE, form_.y()));
		form_.setHeight(ViewButton::SIZE + 2 * ViewButton::SPACE);

		buttons_.push_back(button);
	}
}

void ViewButtonArea::remove_button(ViewButton* button)
{
	if (buttons_.removeOne(button))
	{
		form_.setWidth(form_.width() - ViewButton::SIZE - ViewButton::SPACE);
		form_.moveTopLeft(QPoint(form_.x() + ViewButton::SIZE + ViewButton::SPACE, form_.y()));
		form_.setHeight(ViewButton::SIZE + 2 * ViewButton::SPACE);
	}
}

bool ViewButtonArea::is_clicked(int x, int y)
{
	return form_.contains(x, y);
}

void ViewButtonArea::click_button(int x, int y, int globalX, int globalY)
{
	QPoint p = form_.topLeft();
	p.setY(p.y() + ViewButton::SPACE);
	foreach (ViewButton* b, buttons_)
	{
		if (QRect(p, QSize(ViewButton::SIZE, ViewButton::SIZE)).contains(x, y))
		{
			b->click(x, y, globalX, globalY);
			return;
		}
		p.setX(p.x() + ViewButton::SPACE + ViewButton::SIZE);
	}
}

void ViewButtonArea::set_top_right_position(int x, int y)
{
	form_.moveTopRight(QPoint(x, y));
}

void ViewButtonArea::set_top_left_position(int x, int y)
{
	form_.moveTopLeft(QPoint(x + ViewButton::SPACE, y));
}

void ViewButtonArea::draw()
{
	int p_x = form_.x();
	int p_y = form_.y();

	foreach (ViewButton* b, buttons_)
	{
		b->draw_at(p_x, p_y + ViewButton::SPACE);
		p_x += ViewButton::SIZE + ViewButton::SPACE;
	}
}

} // namespace schnapps
