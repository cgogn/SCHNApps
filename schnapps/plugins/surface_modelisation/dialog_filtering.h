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

#ifndef SCHNAPPS_PLUGIN_SURFACE_MODELISATION_DIALOG_FILTERING_H_
#define SCHNAPPS_PLUGIN_SURFACE_MODELISATION_DIALOG_FILTERING_H_

#include <schnapps/plugins/surface_modelisation/dll.h>

#include <ui_dialog_filtering.h>

#include <schnapps/core/map_handler.h>

namespace schnapps
{

class SCHNApps;

namespace plugin_surface_modelisation
{

class Plugin_SurfaceModelisation;

class SCHNAPPS_PLUGIN_SURFACE_MODELISATION_API Filtering_Dialog : public QDialog, public Ui::Filtering_Dialog
{
	Q_OBJECT

public:

	Filtering_Dialog(SCHNApps* s, Plugin_SurfaceModelisation* p);

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceModelisation* plugin_;

	CMap2Handler* selected_map_;

	QString setting_auto_load_position_attribute_;
	QString setting_auto_load_normal_attribute_;

private slots:

	void filter_average();
	void filter_bilateral();
	void filter_taubin();
	void filter_laplacian();
	void selected_map_changed();
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
};

} // namespace plugin_surface_modelisation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_MODELISATION_DIALOG_FILTERING_H_
