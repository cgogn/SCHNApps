/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#ifndef SCHNAPPS_PLUGIN_EMPTY_PLUGIN_H_
#define SCHNAPPS_PLUGIN_EMPTY_PLUGIN_H_

#include <schnapps/plugins/empty_plugin/dll.h>

#include <schnapps/core/plugin_processing.h>
// OR #include <schnapps/core/plugin_interaction.h>

namespace schnapps
{

namespace plugin_empty_plugin
{

/**
* @brief Empty plugin example
*/
class SCHNAPPS_PLUGIN_EMPTY_PLUGIN_API Plugin_EmptyPlugin : public PluginProcessing
// OR
//class SCHNAPPS_PLUGIN_EMPTY_PLUGIN_API Plugin_EmptyPlugin : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_EmptyPlugin();
	inline ~Plugin_EmptyPlugin() override {}
	static QString plugin_name();

private:

	bool enable() override;
	void disable() override;

public slots:

private slots:

};

} // namespace plugin_empty_plugin

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EMPTY_PLUGIN_H_
