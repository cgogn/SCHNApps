/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin ExtractSurface                                                        *
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

#ifndef SCHNAPPS_PLUGIN_EXTRACT_SURFACE_EXTRACT_SURFACE_H
#define SCHNAPPS_PLUGIN_EXTRACT_SURFACE_EXTRACT_SURFACE_H

#include <schnapps/plugins/extract_surface/dll.h>

#include <schnapps/core/plugin_processing.h>

#include <QAction>

namespace schnapps
{

class Object;

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMap2Handler;
class CMap3Handler;
} // namespace plugin_cmap_provider



namespace plugin_extract_surface
{

class ExtractDialog;

class SCHNAPPS_PLUGIN_EXTRACT_SURFACE_API Plugin_ExtractSurface : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_ExtractSurface();
	inline ~Plugin_ExtractSurface() override {}
	static QString plugin_name();

	inline plugin_cmap_provider::Plugin_CMapProvider* map_provider() { return plugin_cmap_provider_; }

	void extract_surface(plugin_cmap_provider::CMap3Handler* in_map3, plugin_cmap_provider::CMap2Handler* out_map2, const QString& pos_att_name);

private:

	bool enable() override;
	void disable() override;

	QAction* extract_surface_action_;
	ExtractDialog* extract_dialog_;
	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;

	public slots:
	private slots:
	void extract_surface_dialog();
};

} // namespace plugin_extract_surface

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXTRACT_SURFACE_EXTRACT_SURFACE_H
