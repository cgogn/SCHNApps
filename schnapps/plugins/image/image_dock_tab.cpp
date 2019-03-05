/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Image                                                                 *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#include <schnapps/plugins/image/image_dock_tab.h>
#include <schnapps/plugins/image/image.h>
#include <schnapps/core/schnapps.h>
#include <QMenu>

namespace schnapps
{

namespace plugin_image
{

Image_DockTab::Image_DockTab(SCHNApps* s, Plugin_Image* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);

	connect(s,&SCHNApps::object_added, [=](Object* o) {
		if (dynamic_cast<Image3D*>(o))
			new QListWidgetItem(o->name(), this->listWidget_images);
	});

	listWidget_images->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(listWidget_images, SIGNAL(customContextMenuRequested(QPoint)), this,SLOT(showContextMenu(QPoint)));
	//connect(this->pushButton_remove, SIGNAL(pressed()), p, SLOT(image_removed()));
}

void Image_DockTab::showContextMenu(const QPoint& point)
{
	auto* item = listWidget_images->itemAt(point);
	if (!item)
		return;

	const QString name = item->text();
	QPoint global_pos = listWidget_images->mapToGlobal(point);

	// Create menu and insert some actions
	QMenu menu;
	connect(menu.addAction("Remove"), &QAction::triggered, [&]() {plugin_->image_removed(name);});
	connect(menu.addAction("Export as point set"), &QAction::triggered, [&]() {plugin_->export_image_to_point_set(name);});
	emit plugin_->context_menu_created(&menu, name);
	menu.exec(global_pos);
}

} // namespace schnapps

} // namespace plugin_image
