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

#include <schnapps/plugins/surface_differential_properties/surface_differential_properties.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>

#include <cgogn/geometry/algos/normal.h>
#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/curvature.h>

namespace schnapps
{

namespace plugin_sdp
{

Plugin_SurfaceDifferentialProperties::Plugin_SurfaceDifferentialProperties()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceDifferentialProperties::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_SurfaceDifferentialProperties::enable()
{
	compute_normal_dialog_ = new ComputeNormal_Dialog(schnapps_, this);
	compute_curvature_dialog_ = new ComputeCurvature_Dialog(schnapps_, this);

	compute_normal_action_ = schnapps_->add_menu_action("Surface;Differential Properties;Compute Normal", "compute vertex normals");
	compute_curvature_action_ = schnapps_->add_menu_action("Surface;Differential Properties;Compute Curvature", "compute vertex curvatures");

	connect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
	connect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_object([this] (Object* o)
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
		if (mh)
			map_added(mh);
	});

	plugin_cmap2_provider_ = reinterpret_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));

	return true;
}

void Plugin_SurfaceDifferentialProperties::disable()
{
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	disconnect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));

	disconnect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
	disconnect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	schnapps_->remove_menu_action(compute_normal_action_);
	schnapps_->remove_menu_action(compute_curvature_action_);

	delete compute_normal_dialog_;
	delete compute_curvature_dialog_;
}

void Plugin_SurfaceDifferentialProperties::object_added(Object* o)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		map_added(mh);
}

void Plugin_SurfaceDifferentialProperties::map_added(CMap2Handler *map)
{
	connect(map, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
}

void Plugin_SurfaceDifferentialProperties::object_removed(Object* o)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(o);
	if (mh)
		map_removed(mh);
}

void Plugin_SurfaceDifferentialProperties::map_removed(CMap2Handler *map)
{
	disconnect(map, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
}

void Plugin_SurfaceDifferentialProperties::schnapps_closing()
{
	compute_normal_dialog_->close();
	compute_curvature_dialog_->close();
}

void Plugin_SurfaceDifferentialProperties::attribute_changed(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		CMap2Handler* map = static_cast<CMap2Handler*>(sender());
		if (has_compute_normal_last_parameters(map))
		{
			ComputeNormalParameters& params = compute_normal_last_parameters_[map];
			if (params.auto_update_ && params.position_name_ == attribute_name)
				compute_normal(
					map->name(),
					params.position_name_,
					params.normal_name_, params.create_vbo_normal_,
					true
				);
		}
		if (has_compute_curvature_last_parameters(map))
		{
			ComputeCurvatureParameters& params = compute_curvature_last_parameters_[map];
			if (params.auto_update_ && (params.position_name_ == attribute_name || params.normal_name_ == attribute_name))
				compute_curvature(
					map->name(),
					params.position_name_, params.normal_name_,
					params.Kmax_name_, params.create_vbo_Kmax_,
					params.kmax_name_, params.create_vbo_kmax_,
					params.Kmin_name_, params.create_vbo_Kmin_,
					params.kmin_name_, params.create_vbo_kmin_,
					params.Knormal_name_, params.create_vbo_Knormal_,
					params.compute_kmean_, params.kmean_name_, params.create_vbo_kmean_,
					params.compute_kgaussian_, params.kgaussian_name_, params.create_vbo_kgaussian_,
					true
				);
		}
	}
}

void Plugin_SurfaceDifferentialProperties::open_compute_normal_dialog()
{
	compute_normal_dialog_->show();
}

void Plugin_SurfaceDifferentialProperties::open_compute_curvature_dialog()
{
	compute_curvature_dialog_->show();
}

void Plugin_SurfaceDifferentialProperties::compute_normal(
	const QString& map_name,
	const QString& position_attribute_name,
	const QString& normal_attribute_name,
	bool create_vbo_normal,
	bool auto_update)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> normal = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name.toStdString());
	if (!normal.is_valid())
	{
		// if there is another attribute with the same name but with a different type, we remove it
		if (map->has_attribute(CMap2::Vertex::ORBIT, normal_attribute_name.toStdString()))
			mh->remove_attribute(CMap2::Vertex::ORBIT, normal_attribute_name);
		normal = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);
	}

	cgogn::geometry::compute_normal(*map, position, normal);

	mh->notify_attribute_change(CMap2::Vertex::ORBIT, normal_attribute_name);

	if (create_vbo_normal)
		mh->create_vbo(normal_attribute_name);

	compute_normal_last_parameters_[mh] =
		ComputeNormalParameters(
			position_attribute_name,
			normal_attribute_name,
			create_vbo_normal,
			auto_update
		);
}

void Plugin_SurfaceDifferentialProperties::compute_curvature(
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
	bool auto_update)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> normal = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name.toStdString());
	if (!normal.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> Kmax = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmax_attribute_name.toStdString());
	if (!Kmax.is_valid())
		Kmax = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmax_attribute_name);

	CMap2::VertexAttribute<SCALAR> kmax = map->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmax_attribute_name.toStdString());
	if (!kmax.is_valid())
		kmax = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmax_attribute_name);

	CMap2::VertexAttribute<VEC3> Kmin = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmin_attribute_name.toStdString());
	if (!Kmin.is_valid())
		Kmin = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmin_attribute_name);

	CMap2::VertexAttribute<SCALAR> kmin = map->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmin_attribute_name.toStdString());
	if (!kmin.is_valid())
		kmin = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmin_attribute_name);

	CMap2::VertexAttribute<VEC3> Knormal = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(Knormal_attribute_name.toStdString());
	if (!Knormal.is_valid())
		Knormal = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(Knormal_attribute_name);

	CMap2::EdgeAttribute<SCALAR> edge_angle = map->get_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_angle");
	if (!edge_angle.is_valid())
		edge_angle = mh->add_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_angle");

	CMap2::EdgeAttribute<SCALAR> edge_area = map->get_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_area");
	if (!edge_area.is_valid())
		edge_area = mh->add_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_area");

	cgogn::geometry::compute_angle_between_face_normals(*map, position, edge_angle);
	cgogn::geometry::compute_area<CMap2::Edge>(*map, position, edge_area);

	SCALAR mean_edge_length = cgogn::geometry::mean_edge_length(*map, position);
	float32 radius = 2.5f * mean_edge_length;

	cgogn::geometry::compute_curvature(*map, radius, position, normal, edge_angle, edge_area, kmax, kmin, Kmax, Kmin, Knormal);

	mh->notify_attribute_change(CMap2::Vertex::ORBIT, Kmax_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, kmax_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, Kmin_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, kmin_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, Knormal_attribute_name);

	if (create_vbo_Kmax)
		mh->create_vbo(Kmax_attribute_name);
	if (create_vbo_kmax)
		mh->create_vbo(kmax_attribute_name);
	if (create_vbo_Kmin)
		mh->create_vbo(Kmin_attribute_name);
	if (create_vbo_kmin)
		mh->create_vbo(kmin_attribute_name);
	if (create_vbo_Knormal)
		mh->create_vbo(Knormal_attribute_name);

	if (compute_kmean)
	{
		CMap2::VertexAttribute<SCALAR> kmean = map->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmean_attribute_name.toStdString());
		if (!kmean.is_valid())
			kmean = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmean_attribute_name);

		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		container.parallel_foreach_index([&] (uint32 i)
		{
			kmean[i] = (kmin[i] + kmax[i]) / 2.0;
		});

		mh->notify_attribute_change(CMap2::Vertex::ORBIT, kmean_attribute_name);

		if (create_vbo_kmean)
			mh->create_vbo(kmean_attribute_name);
	}

	if (compute_kgaussian)
	{
		CMap2::VertexAttribute<SCALAR> kgaussian = map->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kgaussian_attribute_name.toStdString());
		if (!kgaussian.is_valid())
			kgaussian = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kgaussian_attribute_name);

		const CMap2::ChunkArrayContainer<uint32>& container = map->attribute_container<CMap2::Vertex::ORBIT>();
		container.parallel_foreach_index([&] (uint32 i)
		{
			kgaussian[i] = kmin[i] * kmax[i];
		});

		if (create_vbo_kmin)
			mh->create_vbo(kmin_attribute_name);

		mh->notify_attribute_change(CMap2::Vertex::ORBIT, kgaussian_attribute_name);

		if (create_vbo_kgaussian)
			mh->create_vbo(kgaussian_attribute_name);
	}

	compute_curvature_last_parameters_[mh] =
		ComputeCurvatureParameters(
			position_attribute_name, normal_attribute_name,
			Kmax_attribute_name, create_vbo_Kmax,
			kmax_attribute_name, create_vbo_kmax,
			Kmin_attribute_name, create_vbo_Kmin,
			kmin_attribute_name, create_vbo_kmin,
			Knormal_attribute_name, create_vbo_Knormal,
			compute_kmean, kmean_attribute_name, create_vbo_kmean,
			compute_kgaussian, kgaussian_attribute_name, create_vbo_kgaussian,
			auto_update
		);
}

} // namespace plugin_sdp

} // namespace schnapps
