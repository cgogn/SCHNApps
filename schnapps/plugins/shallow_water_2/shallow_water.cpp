/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#include <shallow_water.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#include <cgogn/modeling/tiling/triangular_grid.h>
#include <cgogn/geometry/algos/centroid.h>
#include <cgogn/geometry/algos/length.h>

namespace schnapps
{

namespace plugin_shallow_water_2
{

bool Plugin_ShallowWater::enable()
{
	dock_tab_ = new ShallowWater_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Shallow Water 2");

	const unsigned int nbc = 10u;

	map_ = static_cast<CMap2Handler*>(schnapps_->add_map("shallow_water_2", 2));
	map2_ = static_cast<CMap2*>(map_->get_map());

	position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position");

	dart_level_ = map_->add_attribute<uint8, CMap2::CDart::ORBIT>("dart_level");
	subd_code_ = map_->add_attribute<uint32, CMap2::Face::ORBIT>("subdivision_code");

	cgogn::modeling::TriangularGrid<CMap2> grid(*map2_, nbc, nbc);
	grid.embed_into_grid(position_, 200.0f, 200.0f, 0.0f);

	map2_->parallel_foreach_cell([&] (CMap2::Face f, uint32)
	{
		subd_code_[f] = 0;
		map2_->foreach_dart_of_orbit(f, [&] (cgogn::Dart d) { dart_level_[d] = 0; });
	});

	init();

	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), this, SLOT(execute_time_step()));

	return true;
}

void Plugin_ShallowWater::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	schnapps_->remove_map("shallow_water_2");
	delete dock_tab_;
}

void Plugin_ShallowWater::init()
{
	t_ = 0.;
	dt_ = 0.01;
}

void Plugin_ShallowWater::start()
{
	if (!timer_->isActive())
	{
		timer_->start(0);
		schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	}
}

void Plugin_ShallowWater::stop()
{
	if (timer_->isActive())
	{
		timer_->stop();
		schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
	}
}

bool Plugin_ShallowWater::is_running()
{
	return timer_->isActive();
}

void Plugin_ShallowWater::execute_time_step()
{
	connectivity_changed_ = false;

	map2_->parallel_foreach_cell(
		[&] (CMap2::Edge e, uint32)
		{
			// solve flux on edge
		},
		// avoid boundary edges
		[&] (CMap2::Edge e) { return !map2_->is_incident_to_boundary(e); }
	);

	map2_->parallel_foreach_cell([&] (CMap2::Face f, uint32)
	{
		// update quantities on faces
	});

	try_simplification();
	try_subdivision();

	if (connectivity_changed_)
		map_->notify_connectivity_change();
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "position");

	t_ += dt_;
}

void Plugin_ShallowWater::try_subdivision()
{
	CMap2::CellCache cache(*map2_);
	cache.build<CMap2::Face>();

	map2_->foreach_cell(
		[&] (CMap2::Face f)
		{
			if ( /* a certain condition is met */ true)
			{
				subdivide_face(f);
				connectivity_changed_ = true;
			}
		},
		cache
	);
}

void Plugin_ShallowWater::try_simplification()
{
	map2_->foreach_cell(
		[&] (CMap2::Face f)
		{
			if ( /* a certain condition is met */ true)
			{
				simplify_face(f);
				connectivity_changed_ = true;
			}
		},
		// manage only central triangles
		[&] (CMap2::Face f) { return subd_code_[f] % 4 == 0; }
	);
}

void Plugin_ShallowWater::subdivide_face(CMap2::Face f)
{
	f.dart = oldest_dart(f);
	uint32 fl = face_level(f);
	uint32 fc = subd_code_[f];

	// check neighbours level
	map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
	{
		if (face_level(af) < fl)
			subdivide_face(af);
	});

	// cut edges (if not already done)
	cgogn::Dart it = f.dart;
	do
	{
		cgogn::Dart next = map2_->phi1(it);
		if (dart_level_[next] > fl)
			next = map2_->phi1(it);
		else
		{
			map2_->cut_edge(CMap2::Edge(it));
			dart_level_[map2_->phi1(it)] = fl+1;
			dart_level_[map2_->phi2(it)] = fl+1;
		}
		it = next;
	} while (it != f.dart);

	// cut face into 4 triangles
	it = map2_->phi1(it);
	cgogn::Dart it2 = map2_->template phi<11>(it);
	CMap2::Edge e = map2_->cut_face(it, it2);
	dart_level_[e.dart] = fl+1;
	dart_level_[map2_->phi2(e.dart)] = fl+1;
	subd_code_[CMap2::Face(it)] = 4*fc + 1;
	it = map2_->template phi<11>(it2);
	e = map2_->cut_face(it, it2);
	dart_level_[e.dart] = fl+1;
	dart_level_[map2_->phi2(e.dart)] = fl+1;
	subd_code_[CMap2::Face(it2)] = 4*fc + 2;
	it2 = map2_->template phi<11>(it);
	e = map2_->cut_face(it, it2);
	dart_level_[e.dart] = fl+1;
	dart_level_[map2_->phi2(e.dart)] = fl+1;
	subd_code_[CMap2::Face(it)] = 4*fc + 3;
	subd_code_[CMap2::Face(it2)] = 4*fc + 4;
}

void Plugin_ShallowWater::simplify_face(CMap2::Face f)
{

}

uint32 Plugin_ShallowWater::face_level(CMap2::Face f)
{
	uint32 code = subd_code_[f];
	if (code == 0) return 0;
	if (code < 5) return 1;
	if (code < 21) return 2;
	if (code < 85) return 3;
	if (code < 341) return 4;
	if (code < 1365) return 5;
	if (code < 5461) return 6;
}

cgogn::Dart Plugin_ShallowWater::oldest_dart(CMap2::Face f)
{
	cgogn::Dart res = f.dart;
	uint8 min = dart_level_[f.dart];
	map2_->foreach_dart_of_orbit(f, [&] (cgogn::Dart d)
	{
		if (dart_level_[d] < min)
			res = d;
	});
	return res;
}

} // namespace plugin_shallow_water_2

} // namespace schnapps
