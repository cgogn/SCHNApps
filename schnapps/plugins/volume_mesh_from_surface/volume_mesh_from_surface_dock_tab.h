/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Mesh From Surface                                              *
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
#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_VECTOR_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_VECTOR_DOCK_TAB_H_

#include <ui_volume_mesh_from_surface.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;
class Plugin_VolumeMeshFromSurface;

struct MapParameters;

class VolumeMeshFromSurface_DockTab : public QWidget, public Ui::VolumeMeshFromSurface_TabWidget
{
	Q_OBJECT

	friend class Plugin_VolumeMeshFromSurface;

public:

	VolumeMeshFromSurface_DockTab(SCHNApps* s, Plugin_VolumeMeshFromSurface* p);

private:

	SCHNApps* schnapps_;
	Plugin_VolumeMeshFromSurface* plugin_;

	bool updating_ui_;

private slots:
	void selected_map_changed(MapHandlerGen*, MapHandlerGen*);
private:

};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_VECTOR_DOCK_TAB_H_
