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

bool Plugin_SurfaceDifferentialProperties::enable()
{
//	//	magic line that init static variables of GenericMap in the plugins
//	GenericMap::copyAllStatics(schnapps_->getStaticPointers());

	compute_normal_dialog_ = new ComputeNormal_Dialog(schnapps_);
	compute_curvature_dialog_ = new ComputeCurvature_Dialog(schnapps_);

	compute_normal_action_ = new QAction("Compute Normal", this);
	compute_curvature_action_ = new QAction("Compute Curvature", this);

	compute_normal_action_ = schnapps_->add_menu_action("Surface;Differential Properties;Compute Normal", "compute vertex normals");
	compute_curvature_action_ = schnapps_->add_menu_action("Surface;Differential Properties;Compute Curvature", "compute vertex curvatures");

	connect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
	connect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	connect(compute_normal_dialog_, SIGNAL(accepted()), this, SLOT(compute_normal_from_dialog()));
	connect(compute_normal_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_normal_from_dialog()));

	connect(compute_curvature_dialog_, SIGNAL(accepted()), this, SLOT(compute_curvature_from_dialog()));
	connect(compute_curvature_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_curvature_from_dialog()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->foreach_map([this] (MapHandlerGen* map) { map_added(map); });

	return true;
}

void Plugin_SurfaceDifferentialProperties::disable()
{
	disconnect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
	disconnect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	disconnect(compute_normal_dialog_, SIGNAL(accepted()), this, SLOT(compute_normal_from_dialog()));
	disconnect(compute_normal_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_normal_from_dialog()));

	disconnect(compute_curvature_dialog_, SIGNAL(accepted()), this, SLOT(compute_curvature_from_dialog()));
	disconnect(compute_curvature_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_curvature_from_dialog()));

	disconnect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->remove_menu_action(compute_normal_action_);
	schnapps_->remove_menu_action(compute_curvature_action_);

	delete compute_normal_dialog_;
	delete compute_curvature_dialog_;
}

void Plugin_SurfaceDifferentialProperties::map_added(MapHandlerGen *map)
{
	if (map->dimension() == 2)
		connect(map, SIGNAL(attribute_modified(cgogn::Orbit, QString)), this, SLOT(attribute_modified(cgogn::Orbit, const QString&)));
}

void Plugin_SurfaceDifferentialProperties::map_removed(MapHandlerGen *map)
{
	if (map->dimension() == 2)
		disconnect(map, SIGNAL(attribute_modified(cgogn::Orbit, QString)), this, SLOT(attribute_modified(cgogn::Orbit, const QString&)));
}

void Plugin_SurfaceDifferentialProperties::schnapps_closing()
{
	compute_normal_dialog_->close();
	compute_curvature_dialog_->close();
}

void Plugin_SurfaceDifferentialProperties::attribute_modified(cgogn::Orbit orbit, const QString& attribute_name)
{
	if (orbit == CMap2::Vertex::ORBIT)
	{
		MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
		if (compute_normal_last_parameters_.count(map->get_name()) > 0ul)
		{
			ComputeNormalParameters& params = compute_normal_last_parameters_[map->get_name()];
			if (params.auto_update && params.position_name == attribute_name)
				compute_normal(map->get_name(), params.position_name, params.normal_name, true);
		}
		if (compute_curvature_last_parameters_.count(map->get_name()) > 0ul)
		{
			ComputeCurvatureParameters& params = compute_curvature_last_parameters_[map->get_name()];
			if (params.auto_update && (params.position_name == attribute_name || params.normal_name == attribute_name))
				compute_curvature(
					map->get_name(),
					params.position_name, params.normal_name,
					params.Kmax_name, params.kmax_name, params.Kmin_name, params.kmin_name, params.Knormal_name,
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

void Plugin_SurfaceDifferentialProperties::compute_normal_from_dialog()
{
	QList<QListWidgetItem*> currentItems = compute_normal_dialog_->list_maps->selectedItems();
	if(!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		QString position_name = compute_normal_dialog_->combo_positionAttribute->currentText();

		QString normal_name;
		if (compute_normal_dialog_->normal_attribute_name->text().isEmpty())
			normal_name = compute_normal_dialog_->combo_normalAttribute->currentText();
		else
			normal_name = compute_normal_dialog_->normal_attribute_name->text();

		bool auto_update = currentItems[0]->checkState() == Qt::Checked;

		compute_normal(map_name, position_name, normal_name, auto_update);

		// create VBO if asked
		if (compute_normal_dialog_->enableVBO->isChecked())
		{
			MapHandlerGen* mhg = schnapps_->get_map(map_name);
			if (mhg != NULL)
				mhg->create_vbo(normal_name);
		}
	}
}

void Plugin_SurfaceDifferentialProperties::compute_curvature_from_dialog()
{
	QList<QListWidgetItem*> currentItems = compute_curvature_dialog_->list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		QString position_name = compute_curvature_dialog_->combo_positionAttribute->currentText();
		QString normal_name = compute_curvature_dialog_->combo_normalAttribute->currentText();

		QString Kmax_name;
		if (compute_curvature_dialog_->Kmax_attribute_name->text().isEmpty())
			Kmax_name = compute_curvature_dialog_->combo_KmaxAttribute->currentText();
		else
			Kmax_name = compute_curvature_dialog_->Kmax_attribute_name->text();

		QString kmax_name;
		if (compute_curvature_dialog_->kmax_attribute_name->text().isEmpty())
			kmax_name = compute_curvature_dialog_->combo_kmaxAttribute->currentText();
		else
			kmax_name = compute_curvature_dialog_->kmax_attribute_name->text();

		QString Kmin_name;
		if (compute_curvature_dialog_->Kmin_attribute_name->text().isEmpty())
			Kmin_name = compute_curvature_dialog_->combo_KminAttribute->currentText();
		else
			Kmin_name = compute_curvature_dialog_->Kmin_attribute_name->text();

		QString kmin_name;
		if (compute_curvature_dialog_->kmin_attribute_name->text().isEmpty())
			kmin_name = compute_curvature_dialog_->combo_kminAttribute->currentText();
		else
			kmin_name = compute_curvature_dialog_->kmin_attribute_name->text();

		QString Knormal_name;
		if (compute_curvature_dialog_->Knormal_attribute_name->text().isEmpty())
			Knormal_name = compute_curvature_dialog_->combo_KnormalAttribute->currentText();
		else
			Knormal_name = compute_curvature_dialog_->Knormal_attribute_name->text();

		bool compute_kmean = compute_curvature_dialog_->check_computeKmean->checkState() == Qt::Checked;
		bool compute_kgaussian = compute_curvature_dialog_->check_computeKgaussian->checkState() == Qt::Checked;
		bool auto_update = currentItems[0]->checkState() == Qt::Checked;

		compute_curvature(
			map_name,
			position_name, normal_name,
			Kmax_name, kmax_name, Kmin_name, kmin_name, Knormal_name,
			compute_kmean, compute_kgaussian,
			auto_update
		);
	}
}

void Plugin_SurfaceDifferentialProperties::compute_normal(
	const QString& map_name,
	const QString& position_attribute_name,
	const QString& normal_attribute_name,
	bool auto_update)
{
	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> normal = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);
	if (!normal.is_valid())
		normal = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);

	cgogn::geometry::normal<VEC3>(*mh->get_map(), position, normal);

	compute_normal_last_parameters_[map_name] =
		ComputeNormalParameters(position_attribute_name, normal_attribute_name, auto_update);

	mh->notify_attribute_change(CMap2::Vertex::ORBIT, normal_attribute_name);
}

void Plugin_SurfaceDifferentialProperties::compute_curvature(
	const QString& map_name,
	const QString& position_attribute_name,
	const QString& normal_attribute_name,
	const QString& Kmax_attribute_name,
	const QString& kmax_attribute_name,
	const QString& Kmin_attribute_name,
	const QString& kmin_attribute_name,
	const QString& Knormal_attribute_name,
	bool compute_kmean,
	bool compute_kgaussian,
	bool auto_update)
{
	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(schnapps_->get_map(map_name));
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

	cgogn::geometry::angle_between_face_normals<VEC3>(*map2, position, edge_angle);
	cgogn::geometry::area<VEC3, CMap2::Edge>(*map2, position, edge_area);

	SCALAR mean_edge_length = cgogn::geometry::mean_edge_length<VEC3>(*map2, position);
	float32 radius = 2.0f * mean_edge_length;
	cgogn::geometry::curvature<VEC3>(*map2, radius, position, normal, edge_angle, edge_area, kmax, kmin, Kmax, Kmin, Knormal);

	compute_curvature_last_parameters_[map_name] =
		ComputeCurvatureParameters(
			position_attribute_name, normal_attribute_name,
			Kmax_attribute_name, kmax_attribute_name, Kmin_attribute_name, kmin_attribute_name, Knormal_attribute_name,
			compute_kmean, compute_kgaussian, auto_update);

	mh->notify_attribute_change(CMap2::Vertex::ORBIT, Kmax_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, kmax_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, Kmin_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, kmin_attribute_name);
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, Knormal_attribute_name);

	if (compute_kmean)
	{
		CMap2::VertexAttribute<SCALAR> kmean = mh->get_attribute<SCALAR, CMap2::Vertex::ORBIT>("kmean");
		if (!kmean.is_valid())
			kmean = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("kmean");

		const CMap2::ChunkArrayContainer<uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
		for (uint32 i = container.begin(); i != container.end(); container.next(i))
			kmean[i] = (kmin[i] + kmax[i]) / 2.0;

		mh->notify_attribute_change(CMap2::Vertex::ORBIT, "kmean");
	}

	if (compute_kgaussian)
	{
		CMap2::VertexAttribute<SCALAR> kgaussian = mh->get_attribute<SCALAR, CMap2::Vertex::ORBIT>("kgaussian");
		if (!kgaussian.is_valid())
			kgaussian = mh->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("kgaussian");

		const CMap2::ChunkArrayContainer<uint32>& container = map2->const_attribute_container<CMap2::Vertex::ORBIT>();
		for (uint32 i = container.begin(); i != container.end(); container.next(i))
			kgaussian[i] = kmin[i] * kmax[i];

		mh->notify_attribute_change(CMap2::Vertex::ORBIT, "kgaussian");
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
