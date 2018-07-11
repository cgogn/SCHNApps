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

#include <schnapps/plugins/surface_modelisation/dll.h>
#include <schnapps/plugins/surface_modelisation/dialog_decimation.h>
#include <schnapps/plugins/surface_modelisation/dialog_subdivision.h>
#include <schnapps/plugins/surface_modelisation/dialog_remeshing.h>
#include <schnapps/plugins/surface_modelisation/dialog_filtering.h>

#include <schnapps/core/plugin_processing.h>

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

	Plugin_SurfaceModelisation();
	inline ~Plugin_SurfaceModelisation() {}
	static QString plugin_name();

private:

	virtual bool enable();
	virtual void disable();

private slots:

	// slots called from SCHNApps signals
	void schnapps_closing();

	// slots called from action signals
	void open_decimation_dialog();
	void open_subdivision_dialog();
	void open_remeshing_dialog();
	void open_filtering_dialog();

public slots:

	/**
	 * @brief decimate a mesh through edge collapses
	 * @param map_name name of the 2d map (mesh)
	 * @param position_attribute_name name of position attribute used for computation
	 * @param percentVerticesToRemove % of the number of vertices to remove
	 */
	void decimate(
		const QString& map_name,
		const QString& position_attribute_name,
		double percentVerticesToRemove
	);

	void subdivide_loop(
		const QString& map_name,
		const QString& position_attribute_name
	);

	void subdivide_catmull_clark(
		const QString& map_name,
		const QString& position_attribute_name
	);

	void subdivide_lsm(
		const QString& map_name,
		const QString& position_attribute_name
	);

	void remesh(
		const QString& map_name,
		const QString& position_attribute_name
	);

	void filter_average(
		const QString& map_name,
		const QString& position_attribute_name
	);

	void filter_bilateral(
		const QString& map_name,
		const QString& position_attribute_name,
		const QString& normal_attribute_name
	);

	void filter_taubin(
		const QString& map_name,
		const QString& position_attribute_name
	);

	void filter_laplacian(
		const QString& map_name,
		const QString& position_attribute_name
	);

private:

	Decimation_Dialog* decimation_dialog_;
	Subdivision_Dialog* subdivision_dialog_;
	Remeshing_Dialog* remeshing_dialog_;
	Filtering_Dialog* filtering_dialog_;

	QAction* decimation_action_;
	QAction* subdivision_action_;
	QAction* remeshing_action_;
	QAction* filtering_action_;
};

} // namespace plugin_surface_modelisation

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_MODELISATION_H_
