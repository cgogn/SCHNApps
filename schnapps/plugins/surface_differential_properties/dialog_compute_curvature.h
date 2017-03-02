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

#include "dll.h"
#include <ui_dialog_compute_curvature.h>

#include <schnapps/core/map_handler.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_sdp
{

class Plugin_SurfaceDifferentialProperties;

class SCHNAPPS_PLUGIN_SDP_API ComputeCurvature_Dialog : public QDialog, public Ui::ComputeCurvature_Dialog
{
	Q_OBJECT

public:
	ComputeCurvature_Dialog(SCHNApps* s, Plugin_SurfaceDifferentialProperties* p);

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceDifferentialProperties* plugin_;

	CMap2Handler* selected_map_;

	QString setting_auto_load_position_attribute_;
	QString setting_auto_load_normal_attribute_;
	QString setting_auto_load_Kmax_attribute_;
	QString setting_auto_load_kmax_attribute_;
	QString setting_auto_load_Kmin_attribute_;
	QString setting_auto_load_kmin_attribute_;
	QString setting_auto_load_Knormal_attribute_;

private slots:

	void compute_curvature();
	void selected_map_changed();
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
};

} // namespace plugin_sdp

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_DIALOG_COMPUTE_CURVATURE_H_
