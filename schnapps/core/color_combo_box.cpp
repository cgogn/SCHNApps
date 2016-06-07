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

#include <schnapps/core/color_combo_box.h>

#include <QtGui>

ColorComboBox::ColorComboBox(QWidget *widget) : QComboBox(widget)
{
	//connect( this, SIGNAL(highlighted(int)), this, SLOT(slotHighlight(int)) );
	populate_list();
}

QColor ColorComboBox::color() const
{
	return qvariant_cast<QColor>(itemData(currentIndex(), Qt::DecorationRole));
}

void ColorComboBox::setColor(QColor color)
{
	setCurrentIndex(findData(color, int(Qt::DecorationRole)));
}

void ColorComboBox::populate_list()
{
	//QStringList color_names = QColor::colorNames();
	QStringList color_names;
	color_names <<
		"red" <<
		"green" <<
		"blue" <<
		"cyan "<<
		"magenta" <<
		"yellow" <<
		"gray" <<
		"white" <<
		"black";

	for (int i = 0; i < color_names.size(); ++i)
	{
		QColor color(color_names[i]);
		insertItem(i, color_names[i]);
		setItemData(i, color, Qt::DecorationRole);
	}
}

void ColorComboBox::slot_highlight(int index)
{
	const QStringList color_names = QColor::colorNames();
	QColor color(color_names.at(index));

	QPalette palette = this->palette();
	palette.setColor(QPalette::Highlight, color);
	setPalette(palette);
}
