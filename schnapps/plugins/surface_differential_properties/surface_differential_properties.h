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

#ifndef SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_H_
#define SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_H_

#include <schnapps/core/plugin_processing.h>

#include <dialog_compute_normal.h>
#include <dialog_compute_curvature.h>

#include <QAction>

namespace schnapps
{

class MapHandlerGen;

/**
 * @brief Plugin that manages the computation of differential properties
 * - Normals
 * - Curvatures
 */
class Plugin_SurfaceDifferentialProperties: public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceDifferentialProperties() {}

	~Plugin_SurfaceDifferentialProperties() {}

private:

	virtual bool enable();
	virtual void disable();

private slots:

	// slots called from SCHNApps signals
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);
	void schnapps_closing();

	// slots called from MapHandler signals
	void attribute_modified(cgogn::Orbit orbit, const QString& attribute_name);

	// slots called from action signals
	void open_compute_normal_dialog();
	void open_compute_curvature_dialog();

	// slots called from dialogs signals
	void compute_normal_from_dialog();
	void compute_curvature_from_dialog();

public slots:

	/**
	 * @brief compute the normals of a mesh
	 * @param map_name name of the 2d map (mesh)
	 * @param position_attribute_name name of position attribute used for computation
	 * @param normal_attribute_name name of result attribute
	 * @param auto_update automatically update the normal attribute when position attribute change.
	 */
	void compute_normal(
		const QString& map_name,
		const QString& position_attribute_name = "position",
		const QString& normal_attribute_name = "normal",
		bool auto_update = true
	);

	/**
	 * @brief compute curvatures of a mesh
	 * @param map_name name of 2d map
	 * @param position_attribute_name name of input position attribute
	 * @param normal_attribute_name name of input normal attributes
	 * @param Kmax_attribute_name ?? result attribute name
	 * @param kmax_attribute_name ?? result attribute name
	 * @param Kmin_attribute_name ?? result attribute name
	 * @param kmin_attribute_name ?? result attribute name
	 * @param Knormal_attribute_name ?? result attribute aname
	 * @param compute_kmean compute the mean curvature
	 * @param compute_kgaussian compute the gaussian curvature
	 * @param auto_update automatically update the output attributes when input attribute change.
	 */
	void compute_curvature(
		const QString& map_name,
		const QString& position_attribute_name = "position",
		const QString& normal_attribute_name = "normal",
		const QString& Kmax_attribute_name = "Kmax",
		const QString& kmax_attribute_name = "kmax",
		const QString& Kmin_attribute_name = "Kmin",
		const QString& kmin_attribute_name = "kmin",
		const QString& Knormal_attribute_name = "Knormal",
		bool compute_kmean = true,
		bool compute_kgaussian = true,
		bool auto_update = true
	);

private:

	ComputeNormal_Dialog* compute_normal_dialog_;
	ComputeCurvature_Dialog* compute_curvature_dialog_;

	QAction* compute_normal_action_;
	QAction* compute_curvature_action_;

	struct ComputeNormalParameters
	{
		ComputeNormalParameters() {}
		ComputeNormalParameters(const QString& p, const QString& n, bool update) :
			position_name(p), normal_name(n), auto_update(update)
		{}
		QString position_name;
		QString normal_name;
		bool auto_update;
	};

	std::map<QString, ComputeNormalParameters> compute_normal_last_parameters_;

	struct ComputeCurvatureParameters
	{
		ComputeCurvatureParameters() {}
		ComputeCurvatureParameters(
			const QString& p, const QString& n,
			const QString& Kmax, const QString& kmax, const QString& Kmin, const QString& kmin, const QString& Knormal,
			bool kmean, bool kgaussian, bool update) :
			position_name(p), normal_name(n),
			Kmax_name(Kmax), kmax_name(kmax), Kmin_name(Kmin), kmin_name(kmin), Knormal_name(Knormal),
			compute_kmean(kmean), compute_kgaussian(kgaussian), auto_update(update)
		{}
		QString position_name;
		QString normal_name;
		QString Kmax_name;
		QString kmax_name;
		QString Kmin_name;
		QString kmin_name;
		QString Knormal_name;
		bool compute_kmean;
		bool compute_kgaussian;
		bool auto_update;
	};

	std::map<QString, ComputeCurvatureParameters> compute_curvature_last_parameters_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_H_
