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

#include <surface_modelisation.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/modeling/algos/decimation.h>

namespace schnapps
{

namespace plugin_surface_modelisation
{

bool Plugin_SurfaceModelisation::enable()
{
	decimation_dialog_ = new Decimation_Dialog(schnapps_, this);

	decimation_action_ = new QAction("Decimation", this);

	decimation_action_ = schnapps_->add_menu_action("Surface;Modelisation;Decimation", "decimate mesh");

	connect(decimation_action_, SIGNAL(triggered()), this, SLOT(open_decimation_dialog()));

	connect(decimation_dialog_, SIGNAL(accepted()), this, SLOT(decimate_from_dialog()));
	connect(decimation_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(decimate_from_dialog()));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	return true;
}

void Plugin_SurfaceModelisation::disable()
{
	disconnect(decimation_action_, SIGNAL(triggered()), this, SLOT(open_decimation_dialog()));

	disconnect(decimation_dialog_, SIGNAL(accepted()), this, SLOT(decimate_from_dialog()));
	disconnect(decimation_dialog_->button_apply, SIGNAL(clicked()), this, SLOT(decimate_from_dialog()));

	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	schnapps_->remove_menu_action(decimation_action_);

	delete decimation_dialog_;
}

void Plugin_SurfaceModelisation::schnapps_closing()
{
	decimation_dialog_->close();
}

void Plugin_SurfaceModelisation::open_decimation_dialog()
{
	decimation_dialog_->show();
}

void Plugin_SurfaceModelisation::decimate_from_dialog()
{
	QList<QListWidgetItem*> currentItems = decimation_dialog_->list_maps->selectedItems();
	if(!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		QString position_name = decimation_dialog_->combo_positionAttribute->currentText();
		int v = decimation_dialog_->slider_percentVertices->value();
		decimate(map_name, position_name, (100 - v) / 100.);
	}
}

void Plugin_SurfaceModelisation::decimate(
	const QString& map_name,
	const QString& position_attribute_name,
	double percentVerticesToRemove)
{
	MapHandler<CMap2>* mh = dynamic_cast<MapHandler<CMap2>*>(schnapps_->get_map(map_name));
	if (!mh)
		return;

	CMap2::VertexAttribute<VEC3> position = mh->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_attribute_name);
	if (!position.is_valid())
		return;

	uint32 nbv = mh->nb_cells(Vertex_Cell);
	cgogn::modeling::decimate<VEC3>(*mh->get_map(), position, cgogn::modeling::EdgeTraversor_QEM_T, cgogn::modeling::EdgeApproximator_QEM_T, percentVerticesToRemove * nbv);

	mh->notify_connectivity_change();
	mh->notify_attribute_change(CMap2::Vertex::ORBIT, position_attribute_name);
}

} // namespace plugin_surface_modelisation

} // namespace schnapps
