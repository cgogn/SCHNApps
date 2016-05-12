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

namespace schnapps
{

bool Plugin_SurfaceDifferentialProperties::enable()
{
//	//	magic line that init static variables of GenericMap in the plugins
//	GenericMap::copyAllStatics(schnapps_->getStaticPointers());

	compute_normal_dialog_ = new ComputeNormal_Dialog(schnapps_);
//	compute_curvature_dialog_ = new ComputeCurvature_Dialog(schnapps_);

	compute_normal_action_ = new QAction("Compute Normal", this);
//	compute_curvature_action_ = new QAction("Compute Curvature", this);

	compute_normal_action_ = schnapps_->add_menu_action(this, "Surface;Differential Properties;Compute Normal", "compute vertex normals");
//	compute_curvature_action_ = schnapps_->add_menu_action(this, "Surface;Differential Properties;Compute Curvature", "compute vertex curvatures");

	connect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
//	connect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	connect(compute_normal_dialog_, SIGNAL(accepted()), this, SLOT(compute_normal_from_dialog()));
	connect(compute_normal_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_normal_from_dialog()));

//	connect(compute_curvature_dialog_, SIGNAL(accepted()), this, SLOT(compute_curvature_from_dialog()));
//	connect(compute_curvature_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_curvature_from_dialog()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	for (const auto& map_it : schnapps_->get_map_set())
		map_added(map_it.second);

	return true;
}

void Plugin_SurfaceDifferentialProperties::disable()
{
	disconnect(compute_normal_action_, SIGNAL(triggered()), this, SLOT(open_compute_normal_dialog()));
//	disconnect(compute_curvature_action_, SIGNAL(triggered()), this, SLOT(open_compute_curvature_dialog()));

	disconnect(compute_normal_dialog_, SIGNAL(accepted()), this, SLOT(compute_normal_from_dialog()));
	disconnect(compute_normal_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_normal_from_dialog()));

//	disconnect(compute_curvature_dialog_, SIGNAL(accepted()), this, SLOT(compute_curvature_from_dialog()));
//	disconnect(compute_curvature_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(compute_curvature_from_dialog()));

	disconnect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	disconnect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->remove_menu_action(this, compute_normal_action_);
//	schnapps_->remove_menu_action(this, compute_curvature_action_);

	delete compute_normal_dialog_;
//	delete compute_curvature_dialog_;
}

void Plugin_SurfaceDifferentialProperties::map_added(MapHandlerGen *map)
{
//	connect(map, SIGNAL(attribute_modified(unsigned int, QString)), this, SLOT(attribute_modified(unsigned int, QString)));
}

void Plugin_SurfaceDifferentialProperties::map_removed(MapHandlerGen *map)
{
//	disconnect(map, SIGNAL(attribute_modified(unsigned int, QString)), this, SLOT(attribute_modified(unsigned int, QString)));
}

void Plugin_SurfaceDifferentialProperties::schnapps_closing()
{
	compute_normal_dialog_->close();
//	compute_curvature_dialog_->close();
}

//void Plugin_SurfaceDifferentialProperties::attribute_modified(unsigned int orbit, QString nameAttr)
//{
//	if(orbit == VERTEX)
//	{
//		MapHandlerGen* map = static_cast<MapHandlerGen*>(QObject::sender());
//		if(compute_normalLastParameters.contains(map->get_name()))
//		{
//			ComputeNormalParameters& params = compute_normalLastParameters[map->get_name()];
//			if(params.auto_update && params.position_name == nameAttr)
//				compute_normal(map->get_name(), params.position_name, params.normal_name, true);
//		}
//		if(compute_curvatureLastParameters.contains(map->get_name()))
//		{
//			ComputeCurvatureParameters& params = compute_curvatureLastParameters[map->get_name()];
//			if(params.auto_update && (params.position_name == nameAttr || params.normal_name == nameAttr))
//				compute_curvature(
//					map->get_name(),
//					params.position_name, params.normal_name,
//					params.Kmax_name, params.kmax_name, params.Kmin_name, params.kmin_name, params.Knormal_name,
//					true
//				);
//		}
//	}
//}

void Plugin_SurfaceDifferentialProperties::open_compute_normal_dialog()
{
	compute_normal_dialog_->show();
}

//void Plugin_SurfaceDifferentialProperties::open_compute_curvature_dialog()
//{
//	compute_curvature_dialog_->show();
//}

void Plugin_SurfaceDifferentialProperties::compute_normal_from_dialog()
{
	QList<QListWidgetItem*> currentItems = compute_normal_dialog_->list_maps->selectedItems();
	if(!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();

		QString position_name = compute_normal_dialog_->combo_positionAttribute->currentText();

		QString normal_name;
		if(compute_normal_dialog_->normal_attribute_name->text().isEmpty())
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

//void Plugin_SurfaceDifferentialProperties::compute_curvature_from_dialog()
//{
//	QList<QListWidgetItem*> currentItems = compute_curvature_dialog_->list_maps->selectedItems();
//	if(!currentItems.empty())
//	{
//		const QString& map_name = currentItems[0]->text();

//		QString position_name = compute_curvature_dialog_->combo_positionAttribute->currentText();
//		QString normal_name = compute_curvature_dialog_->combo_normalAttribute->currentText();

//		QString Kmax_name;
//		if(compute_curvature_dialog_->Kmax_attribute_name->text().isEmpty())
//			Kmax_name = compute_curvature_dialog_->combo_KmaxAttribute->currentText();
//		else
//			Kmax_name = compute_curvature_dialog_->Kmax_attribute_name->text();

//		QString kmax_name;
//		if(compute_curvature_dialog_->kmax_attribute_name->text().isEmpty())
//			kmax_name = compute_curvature_dialog_->combo_kmaxAttribute->currentText();
//		else
//			kmax_name = compute_curvature_dialog_->kmax_attribute_name->text();

//		QString Kmin_name;
//		if(compute_curvature_dialog_->Kmin_attribute_name->text().isEmpty())
//			Kmin_name = compute_curvature_dialog_->combo_KminAttribute->currentText();
//		else
//			Kmin_name = compute_curvature_dialog_->Kmin_attribute_name->text();

//		QString kmin_name;
//		if(compute_curvature_dialog_->kmin_attribute_name->text().isEmpty())
//			kmin_name = compute_curvature_dialog_->combo_kminAttribute->currentText();
//		else
//			kmin_name = compute_curvature_dialog_->kmin_attribute_name->text();

//		QString Knormal_name;
//		if(compute_curvature_dialog_->Knormal_attribute_name->text().isEmpty())
//			Knormal_name = compute_curvature_dialog_->combo_KnormalAttribute->currentText();
//		else
//			Knormal_name = compute_curvature_dialog_->Knormal_attribute_name->text();

//		bool compute_kmean = compute_curvature_dialog_->check_computeKmean->checkState() == Qt::Checked;
//		bool compute_kgaussian = compute_curvature_dialog_->check_computeKgaussian->checkState() == Qt::Checked;
//		bool auto_update = currentItems[0]->checkState() == Qt::Checked;

//		compute_curvature(
//			map_name,
//			position_name, normal_name,
//			Kmax_name, kmax_name, Kmin_name, kmin_name, Knormal_name,
//			compute_kmean, compute_kgaussian,
//			auto_update
//		);
//	}
//}

void Plugin_SurfaceDifferentialProperties::compute_normal(
	const QString& map_name,
	const QString& position_attribute_name,
	const QString& normal_attribute_name,
	bool auto_update)
{
	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(schnapps_->get_map(map_name));
	if(!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if(!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> normal = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);
	if(!normal.is_valid())
		normal = mh->add_attribute<VEC3, CMap2::Vertex::ORBIT>(normal_attribute_name);

	cgogn::geometry::compute_normal_vertices<VEC3>(*mh->get_map(), position, normal);

	compute_normal_last_parameters_[map_name] =
		ComputeNormalParameters(position_attribute_name, normal_attribute_name, auto_update);

	mh->notify_attribute_change(CMap2::Vertex::ORBIT, normal_attribute_name);
}

//void Plugin_SurfaceDifferentialProperties::compute_curvature(
//	const QString& map_name,
//	const QString& position_attribute_name,
//	const QString& normal_attribute_name,
//	const QString& Kmax_attribute_name,
//	const QString& kmax_attribute_name,
//	const QString& Kmin_attribute_name,
//	const QString& kmin_attribute_name,
//	const QString& Knormal_attribute_name,
//	bool compute_kmean,
//	bool compute_kgaussian,
//	bool auto_update)
//{
//	MapHandler<CMap2>* mh = static_cast<MapHandler<CMap2>*>(schnapps_->get_map(map_name));
//	if(mh == NULL)
//		return;

//	CMap2* map = mh->get_map();

//	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex>(position_attribute_name);
//	if(!position.isValid())
//		return;

//	CMap2::VertexAttribute<VEC3> normal = map->get_attribute<VEC3, CMap2::Vertex>(normal_attribute_name);
//	if(!normal.isValid())
//		return;

//	CMap2::VertexAttribute<VEC3> Kmax = map->get_attribute<VEC3, CMap2::Vertex>(Kmax_attribute_name);
//	if(!Kmax.isValid())
//		Kmax = map->add_attribute<VEC3, CMap2::Vertex>(Kmax_attribute_name);

//	CMap2::VertexAttribute<SCALAR> kmax = map->get_attribute<SCALAR, CMap2::Vertex>(kmax_attribute_name);
//	if(!kmax.isValid())
//		kmax = map->add_attribute<SCALAR, CMap2::Vertex>(kmax_attribute_name);

//	CMap2::VertexAttribute<VEC3> Kmin = map->get_attribute<VEC3, CMap2::Vertex>(Kmin_attribute_name);
//	if(!Kmin.isValid())
//		Kmin = map->add_attribute<VEC3, CMap2::Vertex>(Kmin_attribute_name);

//	CMap2::VertexAttribute<SCALAR> kmin = map->get_attribute<SCALAR, CMap2::Vertex>(kmin_attribute_name);
//	if(!kmin.isValid())
//		kmin = map->add_attribute<SCALAR, CMap2::Vertex>(kmin_attribute_name);

//	CMap2::VertexAttribute<VEC3> Knormal = map->get_attribute<VEC3, CMap2::Vertex>(Knormal_attribute_name);
//	if(!Knormal.isValid())
//		Knormal = map->add_attribute<VEC3, CMap2::Vertex>(Knormal_attribute_name);

//	CMap2::EdgeAttribute<SCALAR> edge_angle = map->get_attribute<SCALAR, CMap2::Edge>("edge_angle");
//	if(!edge_angle.isValid())
//		edge_angle = map->add_attribute<SCALAR, CMap2::Edge>("edge_angle");

//	CMap2::EdgeAttribute<SCALAR> edge_area = map->get_attribute<SCALAR, CMap2::Edge>("edge_area");
//	if(!edge_area.isValid())
//		edge_area = map->add_attribute<SCALAR, CMap2::Edge>("edge_area");

//	cgogn::geometry::compute_angles_between_normals_on_edges<PFP2>(*map, position, edge_angle);
//	cgogn::geometry::compute_area_edges<PFP2>(*map, position, edge_area);

//	SCALAR mean_edge_length = cgogn::geometry::mean_edge_length<PFP2>(*map, position);

//	float radius = 2.0f * mean_edge_length;
//	cgogn::geometry::compute_curvature_vertices_normal_cycles_projected<VEC3>(*map, radius, position, normal, edge_angle, edge_area, kmax, kmin, Kmax, Kmin, Knormal);

//	compute_curvature_last_parameters_[map_name] =
//		ComputeCurvatureParameters(
//			position_attribute_name, normal_attribute_name,
//			Kmax_attribute_name, kmax_attribute_name, Kmin_attribute_name, kmin_attribute_name, Knormal_attribute_name,
//			compute_kmean, compute_kgaussian, auto_update);

//	mh->notify_attribute_modification(Kmax);
//	mh->notify_attribute_modification(kmax);
//	mh->notify_attribute_modification(Kmin);
//	mh->notify_attribute_modification(kmin);
//	mh->notify_attribute_modification(Knormal);

//	if(compute_kmean)
//	{
//		CMap2::VertexAttribute<SCALAR> kmean = map->get_attribute<SCALAR, CMap2::Vertex>("kmean");
//		if(!kmean.is_valid())
//			kmean = map->add_attribute<SCALAR, CMap2::Vertex>("kmean");

//		for(uint32 i = kmin.begin(); i != kmin.end(); kmin.next(i))
//			kmean[i] = (kmin[i] + kmax[i]) / 2.0;

//		mh->notify_attribute_modification(kmean);
//	}

//	if(compute_kgaussian)
//	{
//		CMap2::VertexAttribute<SCALAR> kgaussian = map->get_attribute<SCALAR, CMap2::Vertex>("kgaussian");
//		if(!kgaussian.is_valid())
//			kgaussian = map->add_attribute<SCALAR, CMap2::Vertex>("kgaussian");

//		for(unsigned int i = kmin.begin(); i != kmin.end(); kmin.next(i))
//			kgaussian[i] = kmin[i] * kmax[i];

//		mh->notify_attribute_modification(kgaussian);
//	}
//}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
