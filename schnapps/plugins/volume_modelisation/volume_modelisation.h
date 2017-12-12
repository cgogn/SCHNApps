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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_
#define SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_

#include <schnapps/plugins/volume_modelisation/dll.h>

#include <schnapps/core/plugin_processing.h>
#include <schnapps/plugins/volume_modelisation/volume_modelisation_docktab.h>
#include <schnapps/plugins/volume_modelisation/volume_operation.h>

#include <cgogn/core/basic/cell.h>

#include <memory>

namespace schnapps
{

namespace plugin_volume_modelisation
{

class SCHNAPPS_PLUGIN_VOLUME_MODELISATION_API VolumeModelisationPlugin : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class VolumeModelisation_DockTab;

public:

	VolumeModelisationPlugin();
	inline ~VolumeModelisationPlugin() override {}
	static QString plugin_name();

private:

	bool enable() override;
	void disable() override;

	public slots:
	void process_operation();
	private slots:
	void current_map_changed(MapHandlerGen* prev, MapHandlerGen* next);
	void current_cells_set_added(CellType ct, const QString& name);
	void current_cells_set_removed(CellType ct, const QString& name);
	void current_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void current_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void update_dock_tab();

private:

	std::unique_ptr<VolumeModelisation_DockTab> docktab_;
	std::unique_ptr<VolumeOperation> operations_;
};

} // namespace plugin_volume_modelisation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_
