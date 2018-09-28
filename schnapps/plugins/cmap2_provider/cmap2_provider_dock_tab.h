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

#ifndef SCHNAPPS_PLUGIN_CMAP2_PROVIDER_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_CMAP2_PROVIDER_DOCK_TAB_H_

#include <schnapps/plugins/cmap2_provider/dll.h>

#include <schnapps/core/types.h>

#include <ui_cmap2_provider.h>

namespace cgogn
{
enum Orbit: numerics::uint32;
namespace rendering { class VBO; }
}

namespace schnapps
{

class SCHNApps;
class Object;

namespace plugin_cmap2_provider
{

class Plugin_CMap2Provider;
class CMap2Handler;

class SCHNAPPS_PLUGIN_CMAP2_PROVIDER_API CMap2Provider_DockTab : public QWidget, public Ui::CMap2Provider_TabWidget
{
	Q_OBJECT

public:

	CMap2Provider_DockTab(SCHNApps* s, Plugin_CMap2Provider* p);
	~CMap2Provider_DockTab() override;

private:

	SCHNApps* schnapps_;
	Plugin_CMap2Provider* plugin_;

	CMap2Handler* selected_map_;

	bool updating_ui_;

private slots:

	// slots called from UI signals
	void selected_map_changed();

//	void duplicate_current_map_clicked();
	void remove_current_map_clicked();

	void bb_vertex_attribute_changed(int index);
	void vertex_attribute_check_state_changed(QListWidgetItem* item);

//	void cells_set_check_state_changed(QListWidgetItem* item);
//	void add_cells_set();
//	void remove_cells_set();

	// slots called from SCHNApps signals

	// slots called from CMap2Handler signals
	void selected_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void selected_map_bb_vertex_attribute_changed(const QString& name);
	void selected_map_vbo_added(cgogn::rendering::VBO* vbo);
	void selected_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void selected_map_connectivity_changed();
//	void selected_map_cells_set_added(cgogn::Orbit orbit, const QString& name);
//	void selected_map_cells_set_removed(cgogn::Orbit orbit, const QString& name);
//	void selected_map_cells_set_mutually_exclusive_changed(cgogn::Orbit orbit, const QString& name);

public:

	// methods used to update the UI from the plugin
	void add_map(CMap2Handler* mh);
	void remove_map(CMap2Handler* mh);

	void refresh_ui();
};

} // namespace plugin_cmap2_provider

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CMAP2_PROVIDER_DOCK_TAB_H_
