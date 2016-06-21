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
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <cgogn/core/utils/unique_ptr.h>

namespace schnapps
{

bool Plugin_VolumeMeshFromSurface::enable()
{
	dock_tab_ = cgogn::make_unique<VolumeMeshFromSurface_DockTab>(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_.get(), "Volume Mesh From Surface");
	return true;
}

void Plugin_VolumeMeshFromSurface::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_.get());
	dock_tab_.reset();
	//	schnapps_->get_selected_map()
}

void Plugin_VolumeMeshFromSurface::generate_button_pressed()
{

}

} // namespace schnapps
