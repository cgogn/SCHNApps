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

#ifndef SCHNAPPS_PLUGIN_IMAGE_IMAGE_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_IMAGE_IMAGE_DOCK_TAB_H_

#include <schnapps/plugins/image/dll.h>

#include <ui_image.h>

namespace schnapps
{

class SCHNApps;

namespace plugin_image
{

class Plugin_Image;

class SCHNAPPS_PLUGIN_IMAGE_API Image_DockTab : public QWidget, public Ui::ImagePlugin_TabWidget
{
	Q_OBJECT

	friend class Plugin_Image;

public:
	Image_DockTab(SCHNApps* s, Plugin_Image* p);

private slots:
	void showContextMenu(const QPoint& point);
private:
	SCHNApps* schnapps_;
	Plugin_Image* plugin_;
	bool updating_ui_;
};

} // namespace schnapps

} // namespace plugin_image

#endif // SCHNAPPS_PLUGIN_IMAGE_IMAGE_DOCK_TAB_H_
