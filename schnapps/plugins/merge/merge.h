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

#ifndef SCHNAPPS_PLUGIN_MERGE_PLUGIN_H_
#define SCHNAPPS_PLUGIN_MERGE_PLUGIN_H_

#include <schnapps/plugins/merge/dll.h>

#include <schnapps/core/plugin_processing.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <QAction>

namespace schnapps
{

namespace plugin_merge
{

class MergeDialog;

/**
* @brief Merge plugin
*/
class SCHNAPPS_PLUGIN_MERGE_PLUGIN_API Plugin_Merge : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_Merge();
	inline ~Plugin_Merge() override {}
	static QString plugin_name();

private:

	bool enable() override;
	void disable() override;

private slots:

	// slots called from SCHNApps signals
	void schnapps_closing();

	// slots called from action signals
	void merge_dialog();

	// slots called from UI signals
	void merge_validated();

public slots:

	/**
	 * @brief merge second_map into first_map
	 */
	bool merge(plugin_cmap_provider::CMap2Handler* first_map, const plugin_cmap_provider::CMap2Handler* second_map);
	bool merge(plugin_cmap_provider::CMap3Handler* first_map, const plugin_cmap_provider::CMap3Handler* second_map);

private:

	MergeDialog* merge_dialog_;
	QAction* merge_action_;
	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;
};

} // namespace plugin_merge

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_MERGE_PLUGIN_H_
