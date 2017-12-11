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

#include "surface_modelisation.h"

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/modeling/algos/decimation.h>
#include <cgogn/modeling/algos/loop.h>
#include <cgogn/modeling/algos/catmull_clark.h>

namespace schnapps
{

namespace plugin_surface_modelisation
{

QString Plugin_SurfaceModelisation::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

Plugin_SurfaceModelisation::Plugin_SurfaceModelisation()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_SurfaceModelisation::enable()
{
	decimation_dialog_ = new Decimation_Dialog(schnapps_, this);
	subdivision_dialog_ = new Subdivision_Dialog(schnapps_, this);

	decimation_action_ = schnapps_->add_menu_action("Surface;Modelisation;Decimation", "decimate mesh");
	subdivision_action_ = schnapps_->add_menu_action("Surface;Modelisation;Subdivision", "subdivide mesh");

	connect(decimation_action_, SIGNAL(triggered()), this, SLOT(open_decimation_dialog()));
	connect(subdivision_action_, SIGNAL(triggered()), this, SLOT(open_subdivision_dialog()));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	return true;
}

void Plugin_SurfaceModelisation::disable()
{
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(decimation_action_, SIGNAL(triggered()), this, SLOT(open_decimation_dialog()));
	disconnect(subdivision_action_, SIGNAL(triggered()), this, SLOT(open_subdivision_dialog()));

	schnapps_->remove_menu_action(decimation_action_);
	schnapps_->remove_menu_action(subdivision_action_);

	delete decimation_dialog_;
	delete subdivision_dialog_;
}

void Plugin_SurfaceModelisation::schnapps_closing()
{
	decimation_dialog_->close();
	subdivision_dialog_->close();
}

void Plugin_SurfaceModelisation::open_decimation_dialog()
{
	decimation_dialog_->show();
}

void Plugin_SurfaceModelisation::open_subdivision_dialog()
{
	subdivision_dialog_->show();
}

void Plugin_SurfaceModelisation::decimate(
	const QString& map_name,
	const QString& position_attribute_name,
	double percentVerticesToRemove)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	uint32 nbv = mh->nb_cells(Vertex_Cell);
	cgogn::modeling::decimate<VEC3>(*mh->get_map(), position, cgogn::modeling::EdgeTraversor_QEM_T, cgogn::modeling::EdgeApproximator_QEM_T, percentVerticesToRemove * nbv);

	schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::subdivide_loop(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	cgogn::modeling::loop<VEC3>(*mh->get_map(), position);

	schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
}

void Plugin_SurfaceModelisation::subdivide_catmull_clark(
	const QString& map_name,
	const QString& position_attribute_name)
{
	CMap2Handler* mh = dynamic_cast<CMap2Handler*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	cgogn::modeling::catmull_clark<VEC3>(*mh->get_map(), position);

	schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
	schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
}

} // namespace plugin_surface_modelisation

} // namespace schnapps
