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

#ifndef SCHNAPPS_PLUGIN_SURFACE_MODELISATION_H_
#define SCHNAPPS_PLUGIN_SURFACE_MODELISATION_H_

#include <schnapps/core/plugin_processing.h>

#include "dll.h"
#include <dialog_decimation.h>

#include <QAction>

namespace schnapps
{

class MapHandlerGen;

namespace plugin_surface_modelisation
{

/**
 * @brief Plugin that exposes some surface modelisation algorithms
 */
class SCHNAPPS_PLUGIN_SURFACE_MODELISATION_API Plugin_SurfaceModelisation : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceModelisation() {}

	~Plugin_SurfaceModelisation() {}

private:

	virtual bool enable();
	virtual void disable();

private slots:

	// slots called from SCHNApps signals
	void schnapps_closing();

	// slots called from action signals
	void open_decimation_dialog();

public slots:

	/**
	 * @brief compute the normals of a mesh
	 * @param map_name name of the 2d map (mesh)
	 * @param position_attribute_name name of position attribute used for computation
	 * @param normal_attribute_name name of result attribute
	 * @param auto_update automatically update the normal attribute when position attribute change.
	 */
	void decimate(
		const QString& map_name,
		const QString& position_attribute_name,
		double percentVerticesToRemove
	);

private:

	Decimation_Dialog* decimation_dialog_;

	QAction* decimation_action_;
};

} // namespace plugin_surface_modelisation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_MODELISATION_H_
