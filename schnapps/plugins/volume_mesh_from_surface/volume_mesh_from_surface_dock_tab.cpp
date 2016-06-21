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

#include <volume_mesh_from_surface.h>
#include <volume_mesh_from_surface_dock_tab.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

VolumeMeshFromSurface_DockTab::VolumeMeshFromSurface_DockTab(SCHNApps* s, Plugin_VolumeMeshFromSurface* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);
	this->pushButton_gen_volume_mesh->setDisabled(true);
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	connect(this->pushButton_gen_volume_mesh,SIGNAL(pressed()), plugin_, SLOT(generate_button_pressed()));
	connect(this->lineEdit_tetgen_args, SIGNAL(textChanged(QString)), plugin_, SLOT(tetgen_args_updated(QString)));

	plugin_->tetgen_args_updated(lineEdit_tetgen_args->text());
}

void VolumeMeshFromSurface_DockTab::selected_map_changed(MapHandlerGen*, MapHandlerGen* curr)
{
	if (dynamic_cast<const CMap2*>(curr->get_map()))
		this->pushButton_gen_volume_mesh->setDisabled(false);
}

} // namespace schnapps
