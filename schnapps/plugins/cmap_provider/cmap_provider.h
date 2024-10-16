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

#ifndef SCHNAPPS_PLUGIN_CMAP_PROVIDER_H_
#define SCHNAPPS_PLUGIN_CMAP_PROVIDER_H_

#include <schnapps/plugins/cmap_provider/plugin_cmap_provider_export.h>

#include <schnapps/plugins/cmap_provider/cmap0_handler.h>
#include <schnapps/plugins/cmap_provider/cmap1_handler.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>
#include <schnapps/plugins/cmap_provider/undirected_graph_handler.h>

#include <schnapps/core/plugin_provider.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

class CMap0Provider_DockTab;
class CMap1Provider_DockTab;
class CMap2Provider_DockTab;
class CMap3Provider_DockTab;

/**
* @brief CGoGN CMap2 provider
*/
class PLUGIN_CMAP_PROVIDER_EXPORT Plugin_CMapProvider : public PluginProvider
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_CMapProvider();
	inline ~Plugin_CMapProvider() override {}
	static QString plugin_name();

private:

	bool enable() override;
	void disable() override;

public:

	CMapHandlerGen* cmap(const QString& name) const;

	CMap0Handler* add_cmap0(const QString& name);
	void remove_cmap0(const QString& name);
	CMap0Handler* duplicate_cmap0(const QString& name);
	CMap0Handler* cmap0(const QString& name) const;

	CMap1Handler* add_cmap1(const QString& name);
	void remove_cmap1(const QString& name);
	CMap1Handler* duplicate_cmap1(const QString& name);
	CMap1Handler* cmap1(const QString& name) const;

	CMap2Handler* add_cmap2(const QString& name);
	void remove_cmap2(const QString& name);
	CMap2Handler* duplicate_cmap2(const QString& name);
	CMap2Handler* cmap2(const QString& name) const;

	CMap3Handler* add_cmap3(const QString& name);
	void remove_cmap3(const QString& name);
	CMap3Handler* duplicate_cmap3(const QString& name);
	CMap3Handler* cmap3(const QString& name) const;

	UndirectedGraphHandler* add_undirected_graph(const QString& name);
	void remove_undirected_graph(const QString& name);
	UndirectedGraphHandler* duplicate_undirected_graph(const QString& name);
	UndirectedGraphHandler* undirected_graph(const QString& name) const;

private slots:
	void display_explorer_widget();

	// slots called from SCHNApps signals
	void schnapps_closing();

private:

	CMap0Provider_DockTab* cmap0_dock_tab_;
	CMap1Provider_DockTab* cmap1_dock_tab_;
	CMap2Provider_DockTab* cmap2_dock_tab_;
	CMap3Provider_DockTab* cmap3_dock_tab_;

	QAction* action_cmap_provider_{Q_NULLPTR};
	QDialog* cmap_explorer_{Q_NULLPTR};
	QTabWidget* cmap_explorer_dock_tab_widget_{Q_NULLPTR};
};

} // namespace plugin_cmap_provider

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CMAP_PROVIDER_H_
