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

#ifndef SCHNAPPS_PLUGIN_SHALLOW_WATER_2_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SHALLOW_WATER_2_DOCK_TAB_H_

#include "dll.h"
#include <ui_shallow_water.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_shallow_water_2
{

class Plugin_ShallowWater;

class SCHNAPPS_PLUGIN_SHALLOW_WATER_2_API ShallowWater_DockTab : public QWidget, public Ui::ShallowWater_TabWidget
{
	Q_OBJECT

	friend class Plugin_ShallowWater;

public:

	ShallowWater_DockTab(SCHNApps* s, Plugin_ShallowWater* p);

private:

	SCHNApps* schnapps_;
	Plugin_ShallowWater* plugin_;

private slots:

	void init();
	void start_stop();
};

} // namespace plugin_shallow_water_2

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_2_DOCK_TAB_H_
