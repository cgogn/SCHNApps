/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Modelisation                                                   *
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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_DOCKTAB_H
#define SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_DOCKTAB_H

#include "dll.h"
#include <ui_volume_modelisation.h>
#include <schnapps/core/types.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_volume_modelisation
{

class VolumeModelisationPlugin;

class SCHNAPPS_PLUGIN_VOLUME_MODELISATION_API VolumeModelisation_DockTab final : public QWidget, private Ui::VolumeModelisation_TabWidget
{
	Q_OBJECT

	friend class VolumeModelisationPlugin;

public:
	VolumeModelisation_DockTab(SCHNApps* s, VolumeModelisationPlugin* p);
	~VolumeModelisation_DockTab() override;

public slots:
private slots:
	void selected_map_vertex_attribute_added();
	void selected_map_vertex_attribute_removed();
private:
	void update(MapHandlerGen* map);
	QComboBox* get_cell_set_combo_box(CellType ct);

	SCHNApps* schnapps_;
	VolumeModelisationPlugin* plugin_;
	bool updating_ui_;
};

} // namespace plugin_volume_modelisation
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_DOCKTAB_H
