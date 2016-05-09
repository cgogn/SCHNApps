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

#ifndef SCHNAPPS_CORE_VIEW_DIALOG_LIST_H_
#define SCHNAPPS_CORE_VIEW_DIALOG_LIST_H_

#include <schnapps/core/dll.h>

#include <QDialog>
#include <QVBoxLayout>
#include <QListWidget>
#include <QListWidgetItem>

namespace schnapps
{

class SCHNAPPS_CORE_API ViewDialogList : public QDialog
{
	Q_OBJECT

public:

	ViewDialogList(const QString& name, QWidget* parent = nullptr);
	virtual ~ViewDialogList();

	virtual void show();

	QListWidget* list();

	QListWidgetItem* add_item(const QString& str, Qt::CheckState checked = Qt::Unchecked);
	QListWidgetItem* get_item(unsigned int row);
	unsigned int nb_items() const;
	QListWidgetItem* find_item(const QString& str) const;
	bool remove_item(const QString& str);

	void check(const QString& str, Qt::CheckState ck);
	bool is_checked(const QString& str);

protected:

	QVBoxLayout* layout_;
	QListWidget* list_items_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_VIEW_DIALOG_LIST_H_
