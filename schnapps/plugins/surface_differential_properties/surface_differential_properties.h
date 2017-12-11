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

#include <schnapps/plugins/surface_differential_properties/dll.h>
#include <schnapps/plugins/surface_differential_properties/dialog_compute_normal.h>
#include <schnapps/plugins/surface_differential_properties/dialog_compute_curvature.h>

#include <QAction>

namespace schnapps
{

class MapHandlerGen;

namespace plugin_sdp
{

/**
 * @brief Plugin that manages the computation of differential properties
 * - Normals
 * - Curvatures
 */
class SCHNAPPS_PLUGIN_SDP_API Plugin_SurfaceDifferentialProperties : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceDifferentialProperties();
	inline ~Plugin_SurfaceDifferentialProperties() override {}
	static QString plugin_name();

private:

	virtual bool enable() override;
	virtual void disable() override;

private slots:

	// slots called from SCHNApps signals
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);
	void schnapps_closing();

	// slots called from MapHandler signals
	void attribute_changed(cgogn::Orbit orbit, const QString& attribute_name);

	// slots called from action signals
	void open_compute_normal_dialog();
	void open_compute_curvature_dialog();

public slots:

	/**
	 * @brief compute the normals of a mesh
	 * @param map_name name of the 2d map
	 * @param position_attribute_name name of position attribute used for computation
	 * @param normal_attribute_name name of result attribute
	 * @param create_vbo_normal create a vbo for the computed normal attribute
	 * @param auto_update automatically update the normal attribute when position attribute change
	 */
	void compute_normal(
		const QString& map_name,
		const QString& position_attribute_name,
		const QString& normal_attribute_name,
		bool create_vbo_normal,
		bool auto_update
	);

	/**
	 * @brief compute curvatures of a mesh
	 * @param map_name name of 2d map
	 * @param position_attribute_name name of input position attribute
	 * @param normal_attribute_name name of input normal attribute
	 * @param Kmax_attribute_name name of output maximum curvature direction attribute
	 * @param create_vbo_Kmax create a vbo for the computed Kmax attribute
	 * @param kmax_attribute_name name of output maximum curvature magnitude attribute
	 * @param create_vbo_kmax create a vbo for the computed kmax attribute
	 * @param Kmin_attribute_name name of output minimum curvature direction attribute
	 * @param create_vbo_Kmin create a vbo for the computed Kmin attribute
	 * @param kmin_attribute_name name of output minimum curvature magnitude attribute
	 * @param create_vbo_kmin create a vbo for the computed kmin attribute
	 * @param Knormal_attribute_name name of output normal direction attribute
	 * @param create_vbo_Knormal create a vbo for the computed Knormal attribute
	 * @param compute_kmean compute the mean curvature
	 * @param kmean_attribute_name name of mean curvature attribute
	 * @param create_vbo_kmean create a vbo for the computed kmean attribute
	 * @param compute_kgaussian compute the gaussian curvature
	 * @param kgaussian_attribute_name name of gaussian curvature attribute
	 * @param create_vbo_kgaussian create a vbo for the computed kgaussian attribute
	 * @param auto_update automatically update the output attributes when input attribute change
	 */
	void compute_curvature(
		const QString& map_name,
		const QString& position_attribute_name,
		const QString& normal_attribute_name,
		const QString& Kmax_attribute_name,
		bool create_vbo_Kmax,
		const QString& kmax_attribute_name,
		bool create_vbo_kmax,
		const QString& Kmin_attribute_name,
		bool create_vbo_Kmin,
		const QString& kmin_attribute_name,
		bool create_vbo_kmin,
		const QString& Knormal_attribute_name,
		bool create_vbo_Knormal,
		bool compute_kmean,
		const QString& kmean_attribute_name,
		bool create_vbo_kmean,
		bool compute_kgaussian,
		const QString& kgaussian_attribute_name,
		bool create_vbo_kgaussian,
		bool auto_update
	);

public:

	struct ComputeNormalParameters
	{
		ComputeNormalParameters() {}
		ComputeNormalParameters(
			const QString& position_name, const QString& normal_name,
			bool create_vbo_normal,
			bool update
		) : position_name_(position_name), normal_name_(normal_name),
			create_vbo_normal_(create_vbo_normal),
			auto_update_(update)
		{}
		QString position_name_;
		QString normal_name_;
		bool create_vbo_normal_;
		bool auto_update_;
	};

	bool has_compute_normal_last_parameters(const QString& map_name)
	{
		return compute_normal_last_parameters_.count(map_name) > 0;
	}
	const ComputeNormalParameters& get_compute_normal_last_parameters(const QString& map_name)
	{
		cgogn_message_assert(has_compute_normal_last_parameters(map_name), "Getting inexistant parameters");
		return compute_normal_last_parameters_[map_name];
	}

	struct ComputeCurvatureParameters
	{
		ComputeCurvatureParameters() {}
		ComputeCurvatureParameters(
			const QString& position_name, const QString& normal_name,
			const QString& Kmax_name, bool create_vbo_Kmax,
			const QString& kmax_name, bool create_vbo_kmax,
			const QString& Kmin_name, bool create_vbo_Kmin,
			const QString& kmin_name, bool create_vbo_kmin,
			const QString& Knormal_name,  bool create_vbo_Knormal,
			bool compute_kmean, const QString& kmean_name, bool create_vbo_kmean,
			bool compute_kgaussian, const QString& kgaussian_name, bool create_vbo_kgaussian,
			bool update
		) :	position_name_(position_name), normal_name_(normal_name),
			Kmax_name_(Kmax_name), create_vbo_Kmax_(create_vbo_Kmax),
			kmax_name_(kmax_name), create_vbo_kmax_(create_vbo_kmax),
			Kmin_name_(Kmin_name), create_vbo_Kmin_(create_vbo_Kmin),
			kmin_name_(kmin_name), create_vbo_kmin_(create_vbo_kmin),
			Knormal_name_(Knormal_name), create_vbo_Knormal_(create_vbo_Knormal),
			compute_kmean_(compute_kmean), kmean_name_(kmean_name), create_vbo_kmean_(create_vbo_kmean),
			compute_kgaussian_(compute_kgaussian), kgaussian_name_(kgaussian_name), create_vbo_kgaussian_(create_vbo_kgaussian),
			auto_update_(update)
		{}
		QString position_name_;
		QString normal_name_;
		QString Kmax_name_;
		bool create_vbo_Kmax_;
		QString kmax_name_;
		bool create_vbo_kmax_;
		QString Kmin_name_;
		bool create_vbo_Kmin_;
		QString kmin_name_;
		bool create_vbo_kmin_;
		QString Knormal_name_;
		bool create_vbo_Knormal_;
		bool compute_kmean_;
		QString kmean_name_;
		bool create_vbo_kmean_;
		bool compute_kgaussian_;
		QString kgaussian_name_;
		bool create_vbo_kgaussian_;
		bool auto_update_;
	};

	bool has_compute_curvature_last_parameters(const QString& map_name)
	{
		return compute_curvature_last_parameters_.count(map_name) > 0;
	}
	const ComputeCurvatureParameters& get_compute_curvature_last_parameters(const QString& map_name)
	{
		cgogn_message_assert(has_compute_curvature_last_parameters(map_name), "Getting inexistant parameters");
		return compute_curvature_last_parameters_[map_name];
	}

private:

	ComputeNormal_Dialog* compute_normal_dialog_;
	ComputeCurvature_Dialog* compute_curvature_dialog_;

	QAction* compute_normal_action_;
	QAction* compute_curvature_action_;

	std::map<QString, ComputeNormalParameters> compute_normal_last_parameters_;
	std::map<QString, ComputeCurvatureParameters> compute_curvature_last_parameters_;
};

} // namespace plugin_sdp

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_DIFFERENTIAL_PROPERTIES_H_
