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

#include <surface_differential_properties.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/geometry/algos/normal.h>
#include <cgogn/geometry/algos/angle.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/curvature.h>

namespace schnapps
{

namespace plugin_sdp
{

bool Plugin_SurfaceDifferentialProperties::enable()
{
	compute_normal_dialog_ = new ComputeNormal_Dialog(schnapps_, this);
	compute_curvature_dialog_ = new ComputeCurvature_Dialog(schnapps_, this);

	compute_normal_action_ = schnapps_->add_menu_action("Surface;Differential Properties;Compute Normal", "compute vertex normals");
	compute_curvature_action_ = schnapps_->add_menu_action("Surface;Differential Properties;Compute Curvature", "compute vertex curvatures");

	connect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
	connect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });

	return true;
}

void Plugin_SurfaceDifferentialProperties::disable()
{
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	disconnect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
	disconnect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	schnapps_->remove_menu_action(compute_normal_action_);
	schnapps_->remove_menu_action(compute_curvature_action_);

	delete compute_normal_dialog_;
	delete compute_curvature_dialog_;
}

void Plugin_SurfaceDifferentialProperties::map_added(MapHandlerGen *map)
{
	if (map->dimension() == 2)
		connect(map, SIGNAL(attribute_changed(cgogn::Orbit, QString)), this, SLOT(attribute_changed(cgogn::Orbit, const QString&)));
}

void Plugin_SurfaceDifferentialProperties::map_removed(MapHandlerGen *map)
{
	if (map->dimension() == 2)
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
		MapHandlerGen* map = static_cast<MapHandlerGen*>(sender());
		if (compute_normal_last_parameters_.count(map->get_name()) > 0ul)
		{
			ComputeNormalParameters& params = compute_normal_last_parameters_[map->get_name()];
			if (params.auto_update_ && params.position_name_ == attribute_name)
				compute_normal(
					map->get_name(),
					params.position_name_,
					params.normal_name_, params.create_vbo_normal_,
					true
				);
		}
		if (compute_curvature_last_parameters_.count(map->get_name()) > 0ul)
		{
			ComputeCurvatureParameters& params = compute_curvature_last_parameters_[map->get_name()];
			if (params.auto_update_ && (params.position_name_ == attribute_name || params.normal_name_ == attribute_name))
				compute_curvature(
					map->get_name(),
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
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> normal = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);
	if (!normal.is_valid())
	{
		// if there is another attribute with the same name but with a different type, we remove it
		if (mh->has_attribute(CMap2::Vertex::ORBIT, normal_attribute_name))
			mh->remove_attribute(CellType::Vertex_Cell, normal_attribute_name);
		normal = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);
	}

	cgogn::geometry::compute_normal<VEC3>(*mh->get_map(), position, normal);

	mh->notify_attribute_change(CMap2::Vertex::ORBIT, normal_attribute_name);

	if (create_vbo_normal)
		mh->create_vbo(normal_attribute_name);

	compute_normal_last_parameters_[map_name] =
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
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> normal = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);
	if (!normal.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> Kmax = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmax_attribute_name);
	if (!Kmax.is_valid())
		Kmax = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmax_attribute_name);

	CMap2::VertexAttribute<SCALAR> kmax = mh->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmax_attribute_name);
	if (!kmax.is_valid())
		kmax = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmax_attribute_name);

	CMap2::VertexAttribute<VEC3> Kmin = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmin_attribute_name);
	if (!Kmin.is_valid())
		Kmin = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(Kmin_attribute_name);

	CMap2::VertexAttribute<SCALAR> kmin = mh->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmin_attribute_name);
	if (!kmin.is_valid())
		kmin = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmin_attribute_name);

	CMap2::VertexAttribute<VEC3> Knormal = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(Knormal_attribute_name);
	if (!Knormal.is_valid())
		Knormal = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(Knormal_attribute_name);

	CMap2::EdgeAttribute<SCALAR> edge_angle = mh->get_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_angle");
	if (!edge_angle.is_valid())
		edge_angle = mh->add_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_angle");

	CMap2::EdgeAttribute<SCALAR> edge_area = mh->get_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_area");
	if (!edge_area.is_valid())
		edge_area = mh->add_attribute<SCALAR, CMap2::Edge::ORBIT>("edge_area");

	CMap2* map2 = mh->get_map();

	cgogn::geometry::compute_angle_between_face_normals<VEC3>(*map2, position, edge_angle);
	cgogn::geometry::compute_area<VEC3, CMap2::Edge>(*map2, position, edge_area);

	SCALAR mean_edge_length = cgogn::geometry::mean_edge_length<VEC3>(*map2, position);
	float32 radius = 2.5f * mean_edge_length;

	cgogn::geometry::compute_curvature<VEC3>(*map2, radius, position, normal, edge_angle, edge_area, kmax, kmin, Kmax, Kmin, Knormal);

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
		CMap2::VertexAttribute<SCALAR> kmean = mh->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmean_attribute_name);
		if (!kmean.is_valid())
			kmean = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kmean_attribute_name);

		const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
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
		CMap2::VertexAttribute<SCALAR> kgaussian = mh->get_attribute<SCALAR, CMap2::Vertex::ORBIT>(kgaussian_attribute_name);
		if (!kgaussian.is_valid())
			kgaussian = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>(kgaussian_attribute_name);

		const CMap2::ChunkArrayContainer<uint32>& container = map2->attribute_container<CMap2::Vertex::ORBIT>();
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

	compute_curvature_last_parameters_[map_name] =
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
