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

#ifndef SCHNAPPS_CORE_VIEW_BUTTON_AREA_H_
#define SCHNAPPS_CORE_VIEW_BUTTON_AREA_H_

#include <iostream>

#include <QPixmap>
#include <QSize>
#include <QRect>
#include <QPainter>
#include <QList>

#include <cgogn/rendering/wall_paper.h>

namespace schnapps
{

class View;

class ViewButton : public QObject
{
	Q_OBJECT

public:

	static const int SIZE = 24;
	static const int SPACE = 4;

	ViewButton(const QString& image, View* view);
	~ViewButton();

	QSize get_size();

	void click(int x, int y, int globalX, int globalY);
	void draw_at(int x, int y);

protected:

	QString img_;
	View* view_;
	cgogn::rendering::WallPaper* wall_paper_;
	cgogn::rendering::WallPaper::Renderer* wall_paper_renderer_;

signals:

	void clicked(int x, int y, int globalX, int globalY);
};

class ViewButtonArea : public QObject
{
	Q_OBJECT

public:

	ViewButtonArea(View* view);
	~ViewButtonArea();

	void add_button(ViewButton* button);
	void remove_button(ViewButton* button);

	bool is_clicked(int x, int y);
	void click_button(int x, int y, int globalX, int globalY);

	const QRect& get_form() { return form_; }
	void set_top_right_position(int x, int y);
	void set_top_left_position(int x, int y);
	void draw();

protected:

	View* view_;
	QRect form_;
	QList<ViewButton*> buttons_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_VIEW_BUTTON_AREA_H_
