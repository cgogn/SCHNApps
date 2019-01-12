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

#ifndef SCHNAPPS_PLUGIN_SURFACE_MODELISATION_DIALOG_SUBDIVISION_H_
#define SCHNAPPS_PLUGIN_SURFACE_MODELISATION_DIALOG_SUBDIVISION_H_

#include <schnapps/plugins/surface_modelisation/dll.h>

#include <schnapps/core/types.h>

#include <ui_dialog_subdivision.h>

namespace cgogn { enum Orbit: numerics::uint32; }

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap2Handler;
}

class SCHNApps;
class Object;

namespace plugin_surface_modelisation
{

class Plugin_SurfaceModelisation;
using CMap2Handler = plugin_cmap_provider::CMap2Handler;

class SCHNAPPS_PLUGIN_SURFACE_MODELISATION_API Subdivision_Dialog : public QDialog, public Ui::Subdivision_Dialog
{
	Q_OBJECT

public:

	Subdivision_Dialog(SCHNApps* s, Plugin_SurfaceModelisation* p);
	~Subdivision_Dialog() override;

private:

	SCHNApps* schnapps_;
	Plugin_SurfaceModelisation* plugin_;

	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;

	CMap2Handler* selected_map_;

	bool updating_ui_;

	QString setting_auto_load_position_attribute_;

private slots:

	// slots called from UI signals
	void selected_map_changed();
	void subdivide_loop();
	void subdivide_catmull_clark();
	void subdivide_lsm();

	// slots called from SCHNApps signals
	void object_added(Object* o);
	void object_removed(Object* o);

	// slots called from MapHandlerGen signals
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& attribute_name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& attribute_name);

private:

	void refresh_ui();

	void map_added(CMap2Handler* mh);
	void map_removed(CMap2Handler* mh);
};

} // namespace plugin_surface_modelisation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_MODELISATION_DIALOG_SUBDIVISION_H_
