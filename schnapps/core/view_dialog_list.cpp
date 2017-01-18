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
#include <schnapps/core/view_dialog_list.h>
#include <cgogn/core/utils/logger.h>

namespace schnapps
{

ViewDialogList::ViewDialogList(const QString& name, QWidget* parent) :
	QDialog(parent)
{
	setWindowTitle(name);
	setWindowFlags(/*windowFlags() | Qt::FramelessWindowHint*/ /*| Qt::SplashScreen*/ Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);

	layout_ = new QVBoxLayout(this);
	setLayout(layout_);
	layout_->setContentsMargins(1, 1, 1, 1);

	list_items_ = new QListWidget();
	list_items_->setSelectionMode(QAbstractItemView::NoSelection);
	layout_->addWidget(list_items_);
}

ViewDialogList::~ViewDialogList()
{}

void ViewDialogList::show()
{
	int rows = list_items_->model()->rowCount();
	int rowSize = list_items_->sizeHintForRow(0);
	int height = rows * rowSize + 6;
	if (height < 96) // 96??
		height = 96;
	list_items_->setFixedHeight(height);
	QDialog::show();
}

QListWidget* ViewDialogList::list()
{
	return list_items_;
}

QListWidgetItem* ViewDialogList::add_item(const QString& str, Qt::CheckState checked)
{
	QListWidgetItem* item = new QListWidgetItem(str, list_items_);
	item->setCheckState(checked);

	int rows = list_items_->model()->rowCount();
	int rowSize = list_items_->sizeHintForRow(0);
	int height = rows * rowSize;
	if (height >= list_items_->size().height())
		list_items_->setFixedHeight(height + 6);
		
	//QFontMetrics fm(list_items_->font());
	//int maxTextWidth = fm.width(item->text());
	//if (maxTextWidth < 140)
	//	maxTextWidth = 140;
	//list_items_->setFixedWidth(maxTextWidth + 40);
	
	return item;
}

QListWidgetItem* ViewDialogList::get_item(unsigned int row)
{
	return list_items_->item(row);
}

unsigned int ViewDialogList::nb_items() const
{
	return list_items_->count();
}

QListWidgetItem* ViewDialogList::find_item(const QString& str) const
{
	QList<QListWidgetItem*> items = list_items_->findItems(str, Qt::MatchExactly);
	if (!items.empty())
		return items[0];
	return nullptr;
}

bool ViewDialogList::remove_item(const QString& str)
{
	QList<QListWidgetItem*> items = list_items_->findItems(str, Qt::MatchExactly);
	if (items.empty())
		return false;
	list_items_->takeItem(list_items_->row(items[0]));
	return true;
}

void ViewDialogList::check(const QString& str, Qt::CheckState ck)
{
	QList<QListWidgetItem*> items = list_items_->findItems(str, Qt::MatchExactly);
	if (!items.empty())
		items[0]->setCheckState(ck);
	else
		cgogn_log_warning("ViewDialogList::check") << "\"" << str.toStdString() << "\" not in list.";
}

 bool ViewDialogList::is_checked(const QString& str)
{
	QList<QListWidgetItem*> items = list_items_->findItems(str, Qt::MatchExactly);
	if (!items.empty())
		return (items[0]->checkState() == Qt::Checked);
	cgogn_log_warning("ViewDialogList::is_checked") << "\"" << str.toStdString() << "\" not in list.";
	return false;
}

} // namespace schnapps
