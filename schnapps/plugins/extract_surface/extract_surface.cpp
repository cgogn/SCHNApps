/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin ExtractSurface                                                        *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#include <schnapps/plugins/extract_surface/extract_surface.h>
#include <schnapps/plugins/extract_surface/extract_dialog.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>
#include <schnapps/plugins/cmap3_provider/cmap3_provider.h>

#include <cgogn/io/surface_import.h>

namespace schnapps
{

namespace plugin_extract_surface
{

using CMap3Handler = plugin_cmap3_provider::CMap3Handler;
using CMap2Handler = plugin_cmap2_provider::CMap2Handler;

Plugin_ExtractSurface::Plugin_ExtractSurface() :
	extract_surface_action_(nullptr),
	extract_dialog_(nullptr)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_ExtractSurface::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_ExtractSurface::enable()
{
	extract_dialog_ = new ExtractDialog(schnapps_, this);
	plugin_cmap2_provider_ = reinterpret_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));
	plugin_cmap3_provider_ = reinterpret_cast<plugin_cmap3_provider::Plugin_CMap3Provider*>(schnapps_->enable_plugin(plugin_cmap3_provider::Plugin_CMap3Provider::plugin_name()));

	if (!(plugin_cmap2_provider_ && plugin_cmap3_provider_))
		return false;

	extract_surface_action_ = schnapps_->add_menu_action("Export;Extract Surface", "extract surface");
	connect(extract_surface_action_, SIGNAL(triggered()), this, SLOT(extract_surface_dialog()));

	return true;
}

void Plugin_ExtractSurface::disable()
{
	disconnect(extract_surface_action_, SIGNAL(triggered()), this, SLOT(extract_surface_dialog()));

	schnapps_->remove_menu_action(extract_surface_action_);

	delete extract_dialog_;
}

void Plugin_ExtractSurface::extract_surface_dialog()
{
	extract_dialog_->show();
}

void Plugin_ExtractSurface::extract_surface(CMap3Handler* mh3_in, CMap2Handler* mh2_out, const QString& pos_att_name)
{
	if (!mh3_in || !mh2_out || pos_att_name.isEmpty())
		return;

	CMap3& map3 = *mh3_in->map();
	CMap2& map2 = *mh2_out->map();

	cgogn::io::SurfaceImport<CMap2> si(map2);
	std::map<uint32, uint32> old_new_id_map;
	auto in_pos_att = map3.get_attribute<VEC3, CMap3::Vertex::ORBIT>(pos_att_name.toStdString());
	auto* out_pos_att = si.add_vertex_attribute<VEC3>("position");

	map3.foreach_cell([&](CMap3::Vertex v)
	{
		if (map3.is_incident_to_boundary(v))
		{
			const uint32 new_id = si.insert_line_vertex_container();
			old_new_id_map[map3.embedding(v)] = new_id;
			out_pos_att->operator [](new_id) = in_pos_att[v];
		}
	});

	std::vector<uint32> vert_ids;
	map3.foreach_cell([&](CMap3::Face f)
	{
		if (map3.is_incident_to_boundary(f))
		{
			map3.foreach_incident_vertex(f, [&](CMap3::Vertex v)
			{
				vert_ids.push_back(old_new_id_map[map3.embedding(v)]);
			});
			si.add_face(vert_ids);
			vert_ids.clear();
		}
	});

	si.create_map();
	mh2_out->attribute_added(CMap2::Vertex::ORBIT, "position");
	mh2_out->set_bb_vertex_attribute("position");
	mh2_out->create_vbo("position");
}

} // namespace plugin_extract_surface

} // namespace schnapps
