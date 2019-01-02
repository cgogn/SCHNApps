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

#ifndef SCHNAPPS_PLUGIN_EXPORT_H_
#define SCHNAPPS_PLUGIN_EXPORT_H_

#include <schnapps/core/plugin_processing.h>
#include <schnapps/plugins/export/dll.h>


#include <cgogn/io/map_export.h>

class QAction;

namespace schnapps
{

class Object;


namespace plugin_cmap0_provider
{
class Plugin_CMap0Provider;
class CMap0Handler;
}

namespace plugin_cmap1_provider
{
class Plugin_CMap1Provider;
class CMap1Handler;
}

namespace plugin_cmap2_provider
{
class Plugin_CMap2Provider;
class CMap2Handler;
}

namespace plugin_cmap3_provider
{
class Plugin_CMap3Provider;
class CMap3Handler;
}

namespace plugin_export
{

class ExportDialog;

/**
* @brief Plugin for CGoGN mesh export
*/
class SCHNAPPS_PLUGIN_EXPORT_API Plugin_Export : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	using CMap2Handler = plugin_cmap2_provider::CMap2Handler;
	using CMap3Handler = plugin_cmap3_provider::CMap3Handler;

	Plugin_Export();
	inline ~Plugin_Export() override {}

	inline plugin_cmap2_provider::Plugin_CMap2Provider* map2_provider() { return plugin_cmap2_provider_;}
	inline plugin_cmap3_provider::Plugin_CMap3Provider* map3_provider() { return plugin_cmap3_provider_;}

	static QString plugin_name();

private:

	bool enable() override;
	void disable() override;

public slots:

	void export_mesh(Object* mhg, cgogn::io::ExportOptions export_params);
	void export_mesh_from_file_dialog();

private:

	QAction* export_mesh_action_;
	ExportDialog* export_dialog_;
	plugin_cmap2_provider::Plugin_CMap2Provider* plugin_cmap2_provider_;
	plugin_cmap3_provider::Plugin_CMap3Provider* plugin_cmap3_provider_;
};

} // namespace plugin_export

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXPORT_H_
