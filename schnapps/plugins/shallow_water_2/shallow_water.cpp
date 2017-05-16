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
	face_level_ = map_->add_attribute<uint8, CMap2::Face::ORBIT>("face_level");
	face_type_ = map_->add_attribute<uint8, CMap2::Face::ORBIT>("face_type");

	cgogn::modeling::TriangularGrid<CMap2> grid(*map2_, nbc, nbc);
	grid.embed_into_grid(position_, 200.0f, 200.0f, 0.0f);

	map2_->parallel_foreach_cell([&] (CMap2::Face f, uint32)
	{
		face_level_[f] = 0;
		face_type_[f] = 0;
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

//	try_simplification();
//	try_subdivision();

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
		[&] (CMap2::Face f) -> bool // check connectivity condition
		{
			if (face_level_[f] == 0)
				return false;

			if (map2_->codegree(f) != 3)
				return false;

			switch (face_type_[f])
			{
				case 0: {
					cgogn::Dart it = oldest_dart(f);
					it = map2_->phi<12>(it); // central face
					if (map2_->codegree(CMap2::Face(it)) != 3)
						return false;
					cgogn::Dart it2 = map2_->phi<12>(it); // corner face 1
					if (map2_->codegree(CMap2::Face(it2)) != 3)
						return false;
					it2 = map2_->phi2(map2_->phi_1(it)); // corner face 2
					if (map2_->codegree(CMap2::Face(it2)) != 3)
						return false;
					break;
				}
				case 1: {
					cgogn::Dart it = f.dart;
					if (map2_->codegree(CMap2::Face(map2_->phi2(it))) != 3) // corner face 1
						return false;
					it = map2_->phi1(it);
					if (map2_->codegree(CMap2::Face(map2_->phi2(it))) != 3) // corner face 2
						return false;
					it = map2_->phi1(it);
					if (map2_->codegree(CMap2::Face(map2_->phi2(it))) != 3) // corner face 3
						return false;
					break;
				}
			}

			return true;
		}
	);
}

void Plugin_ShallowWater::subdivide_face(CMap2::Face f)
{
	f.dart = oldest_dart(f);
	uint32 fl = face_level_[f];

	// check neighbours level
	map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
	{
		if (face_level_[af] < fl)
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
	face_level_[CMap2::Face(it)] = fl+1;
	face_type_[CMap2::Face(it)] = 0;

	it = map2_->template phi<11>(it2);
	e = map2_->cut_face(it, it2);
	dart_level_[e.dart] = fl+1;
	dart_level_[map2_->phi2(e.dart)] = fl+1;
	face_level_[CMap2::Face(it2)] = fl+1;
	face_type_[CMap2::Face(it2)] = 0;

	it2 = map2_->template phi<11>(it);
	e = map2_->cut_face(it, it2);
	dart_level_[e.dart] = fl+1;
	dart_level_[map2_->phi2(e.dart)] = fl+1;
	face_level_[CMap2::Face(it)] = fl+1;
	face_type_[CMap2::Face(it)] = 0;
	face_level_[CMap2::Face(it2)] = fl+1;
	face_type_[CMap2::Face(it2)] = 1;
}

void Plugin_ShallowWater::simplify_face(CMap2::Face f)
{
	cgogn::Dart it = f.dart;
	if (face_type_[f] == 0)
		it = map2_->phi<12>(oldest_dart(f)); // put it in the central face
	cgogn::Dart next = map2_->phi1(it);
	map2_->merge_incident_faces(CMap2::Edge(it));
	it = next;
	next = map2_->phi1(it);
	map2_->merge_incident_faces(CMap2::Edge(it));
	it = next;
	map2_->merge_incident_faces(CMap2::Edge(it));
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
