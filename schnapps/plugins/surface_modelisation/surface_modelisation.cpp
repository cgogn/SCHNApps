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

#include <schnapps/plugins/surface_modelisation/surface_modelisation.h>

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#include <cgogn/modeling/algos/decimation.h>
#include <cgogn/modeling/algos/loop.h>
#include <cgogn/modeling/algos/catmull_clark.h>
#include <cgogn/modeling/algos/pliant_remeshing.h>

#include <cgogn/geometry/algos/filtering.h>

namespace schnapps
{

namespace plugin_surface_modelisation
{

Plugin_SurfaceModelisation::Plugin_SurfaceModelisation()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceModelisation::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_SurfaceModelisation::enable()
{
	decimation_dialog_ = new Decimation_Dialog(schnapps_, this);
	subdivision_dialog_ = new Subdivision_Dialog(schnapps_, this);
	remeshing_dialog_ = new Remeshing_Dialog(schnapps_, this);
	filtering_dialog_ = new Filtering_Dialog(schnapps_, this);

	decimation_action_ = schnapps_->add_menu_action("Surface;Modelisation;Decimation", "decimate mesh");
	subdivision_action_ = schnapps_->add_menu_action("Surface;Modelisation;Subdivision", "subdivide mesh");
	remeshing_action_ = schnapps_->add_menu_action("Surface;Modelisation;Remeshing", "remesh mesh");
	filtering_action_ = schnapps_->add_menu_action("Surface;Modelisation;Filtering", "filter mesh");

	connect(decimation_action_, SIGNAL(triggered()), this, SLOT(open_decimation_dialog()));
	connect(subdivision_action_, SIGNAL(triggered()), this, SLOT(open_subdivision_dialog()));
	connect(remeshing_action_, SIGNAL(triggered()), this, SLOT(open_remeshing_dialog()));
	connect(filtering_action_, SIGNAL(triggered()), this, SLOT(open_filtering_dialog()));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	plugin_cmap2_provider_ = reinterpret_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));

	return true;
}

void Plugin_SurfaceModelisation::disable()
{
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(decimation_action_, SIGNAL(triggered()), this, SLOT(open_decimation_dialog()));
	disconnect(subdivision_action_, SIGNAL(triggered()), this, SLOT(open_subdivision_dialog()));
	disconnect(remeshing_action_, SIGNAL(triggered()), this, SLOT(open_remeshing_dialog()));
	disconnect(filtering_action_, SIGNAL(triggered()), this, SLOT(open_filtering_dialog()));

	schnapps_->remove_menu_action(decimation_action_);
	schnapps_->remove_menu_action(subdivision_action_);
	schnapps_->remove_menu_action(remeshing_action_);
	schnapps_->remove_menu_action(filtering_action_);

	delete decimation_dialog_;
	delete subdivision_dialog_;
	delete remeshing_dialog_;
	delete filtering_dialog_;
}

void Plugin_SurfaceModelisation::schnapps_closing()
{
	decimation_dialog_->close();
	subdivision_dialog_->close();
	remeshing_dialog_->close();
	filtering_dialog_->close();
}

void Plugin_SurfaceModelisation::open_decimation_dialog()
{
	decimation_dialog_->show();
}

void Plugin_SurfaceModelisation::open_subdivision_dialog()
{
	subdivision_dialog_->show();
}

void Plugin_SurfaceModelisation::open_remeshing_dialog()
{
	remeshing_dialog_->show();
}

void Plugin_SurfaceModelisation::open_filtering_dialog()
{
	filtering_dialog_->show();
}

void Plugin_SurfaceModelisation::decimate(
	const QString& map_name,
	const QString& position_attribute_name,
	double percentVerticesToRemove)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	uint32 nbv = map->nb_cells<CMap2::Vertex>();
	cgogn::modeling::decimate(*map, position, cgogn::modeling::EdgeTraversor_QEM_T, cgogn::modeling::EdgeApproximator_QEM_T, percentVerticesToRemove * nbv);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::subdivide_loop(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	cgogn::modeling::loop(*map, position);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::subdivide_catmull_clark(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	cgogn::modeling::catmull_clark(*map, position);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::subdivide_lsm(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	CMap2::CellCache initial_cache(*map);
	initial_cache.template build<CMap2::Vertex>();
	initial_cache.template build<CMap2::Face>();

	map->foreach_cell([&] (CMap2::Edge e)
	{
		CMap2::Vertex v = map->cut_edge(e);
		position[v] = 0.5 * (position[CMap2::Vertex(e.dart)] + position[CMap2::Vertex(map->phi1(v.dart))]);
	});
	map->foreach_cell([&] (CMap2::Face f)
	{
		cgogn::Dart d0 = map->phi1(f.dart);
		cgogn::Dart d1 = map->template phi<11>(d0);
		map->cut_face(d0, d1);
		cgogn::Dart d2 = map->template phi<11>(d1);
		map->cut_face(d1, d2);
		cgogn::Dart d3 = map->template phi<11>(d2);
		map->cut_face(d2, d3);
	},
	initial_cache);

	// TODO

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::remesh(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	cgogn::modeling::pliant_remeshing(*map, position);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::filter_average(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> position2 = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("__position_average_result");
	cgogn::geometry::filter_average(*map, position, position2);
	map->swap_attributes(position, position2);
	map->remove_attribute(position2);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::filter_bilateral(
	const QString& map_name,
	const QString& position_attribute_name,
	const QString& normal_attribute_name)
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

	CMap2::VertexAttribute<VEC3> position2 = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("__position_bilateral_result");
	cgogn::geometry::filter_bilateral(*map, position, position2, normal);
	map->swap_attributes(position, position2);
	map->remove_attribute(position2);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::filter_taubin(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> position2 = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("__position_taubin_tmp");
	cgogn::geometry::filter_taubin(*map, position, position2);
	map->remove_attribute(position2);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::filter_laplacian(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = plugin_cmap2_provider_->map(map_name);
	if (!mh)
		return;

	CMap2* map = mh->map();

	CMap2::VertexAttribute<VEC3> position = map->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name.toStdString());
	if (!position.is_valid())
		return;

	CMap2::VertexAttribute<VEC3> position2 = map->add_attribute<VEC3, CMap2::Vertex::ORBIT>("__position_laplacian_result");
	cgogn::geometry::filter_laplacian(*map, position, position2);
	map->swap_attributes(position, position2);
	map->remove_attribute(position2);

	schnapps_->selected_view()->current_camera()->disable_views_bb_fitting();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->selected_view()->current_camera()->enable_views_bb_fitting();
}

} // namespace plugin_surface_modelisation

} // namespace schnapps
