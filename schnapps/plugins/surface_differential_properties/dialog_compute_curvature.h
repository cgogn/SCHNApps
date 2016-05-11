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

#ifndef SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_DIALOG_COMPUTE_CURVATURE_H_
#define SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_DIALOG_COMPUTE_CURVATURE_H_

#include <ui_dialog_compute_curvature.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;
class Plugin_SurfaceDifferentialProperties;

//class ComputeCurvature_Dialog : public QDialog, public Ui::ComputeCurvature_Dialog
//{
//	Q_OBJECT

//	friend class Plugin_SurfaceDifferentialProperties;

//public:
//	ComputeCurvature_Dialog(SCHNApps* s);

//private:

//	SCHNApps* schnapps_;
//	MapHandlerGen* selected_map_;

//private slots:

//	void selected_map_changed();

//private:

//	void add_map_to_list(MapHandlerGen* map);
//	void remove_map_from_list(MapHandlerGen* map);
//	void add_attribute_to_list(unsigned int orbit, const QString& attr_name);
//};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_DIALOG_COMPUTE_CURVATURE_H_
