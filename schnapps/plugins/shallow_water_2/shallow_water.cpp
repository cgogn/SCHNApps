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

#include <cgogn/io/map_import.h>

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
	qtrav_ = cgogn::make_unique<CMap2::QuickTraversor>(*map2_);

	cgogn::io::import_surface<VEC3>(*map2_, "/home/kraemer/Desktop/2D.2dm");

	position_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("position");
	for (VEC3& p : position_)
	{
		p[2] *= 100.;
	}

	dart_level_ = map_->add_attribute<uint8, CMap2::CDart::ORBIT>("dart_level");
	face_subd_id_ = map_->add_attribute<uint32, CMap2::Face::ORBIT>("face_subdivision_id");
	tri_face_ = map_->add_attribute<bool, CMap2::Face::ORBIT>("tri_face");

//	cgogn::modeling::TriangularGrid<CMap2> grid(*map2_, nbc, nbc);
//	grid.embed_into_grid(position_, 200.0f, 200.0f, 0.0f);

	qtrav_->build<CMap2::Vertex>();
	qtrav_->build<CMap2::Edge>();
	qtrav_->build<CMap2::Face>();

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			if (map2_->codegree(f) == 3)
				tri_face_[f] = true;
			else
				tri_face_[f] = false;
			face_subd_id_[f] = 0;
			map2_->foreach_dart_of_orbit(f, [&] (cgogn::Dart d) { dart_level_[d] = 0; });
		},
		*qtrav_
	);

	init();

	draw_timer_ = new QTimer(this);
	connect(draw_timer_, SIGNAL(timeout()), this, SLOT(update_draw_data()));

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
	start_time_ = std::chrono::high_resolution_clock::now();

	schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	draw_timer_->start(20);
	simu_running_ = true;
	simu_future_ = cgogn::launch_thread([&] () -> void
	{
		while (simu_running_)
			execute_time_step();
	});
}

void Plugin_ShallowWater::stop()
{
	std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t - start_time_).count();
	std::cout << "time -> " << duration << std::endl;
	std::cout << "t -> " << t_ << std::endl;
	std::cout << "dt -> " << dt_ << std::endl;

	simu_running_ = false;
	schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
}

bool Plugin_ShallowWater::is_simu_running()
{
	return draw_timer_->isActive();
}

void Plugin_ShallowWater::update_draw_data()
{
	if (!simu_running_)
		draw_timer_->stop();

	map_->lock_topo_access();
	simu_data_access_.lock();

	// update draw data from simu data

	simu_data_access_.unlock();

	map_->notify_connectivity_change();
	map_->init_primitives(cgogn::rendering::POINTS);
	map_->init_primitives(cgogn::rendering::LINES);
	map_->init_primitives(cgogn::rendering::TRIANGLES);
	map_->init_primitives(cgogn::rendering::BOUNDARY);

	map_->unlock_topo_access();

	// notify attribute changes
}

void Plugin_ShallowWater::update_time_step()
{
	// update time step
}

void Plugin_ShallowWater::execute_time_step()
{
	//	auto start = std::chrono::high_resolution_clock::now();

	map2_->parallel_foreach_cell(
		[&] (CMap2::Edge e)
		{
			if (map2_->is_incident_to_boundary(e))
				return;

			// solve flux on edge
		},
		*qtrav_
	);

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			// update quantities on faces
		},
		*qtrav_
	);

//	map_->lock_topo_access();
//	try_simplification();
//	try_subdivision();
//	map_->unlock_topo_access();

	t_ += dt_;


	//	auto end = std::chrono::high_resolution_clock::now();

	//	std::chrono::nanoseconds sleep_duration =
	//		std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<SCALAR>(dt_))
	//		- std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

	//	if (sleep_duration > std::chrono::nanoseconds::zero())
	//		std::this_thread::sleep_for(sleep_duration);
}

void Plugin_ShallowWater::try_subdivision()
{
	CMap2::CellMarker<CMap2::Face::ORBIT> subdivided(*map2_);

	map2_->foreach_cell(
		[&] (CMap2::Face f)
		{
			if (subdivided.is_marked(f))
				return;

			if (face_level(f) > 5)
				return;

			if ( /* a certain condition is met */ true)
			{
				subdivide_face(f, subdivided);
			}
		},
		*qtrav_
	);
}

void Plugin_ShallowWater::try_simplification()
{
	map2_->foreach_cell(
		[&] (CMap2::Face f)
		{
			if (face_level(f) == 0)
				return;

			switch (face_type(f))
			{
				case TRI_CORNER: {
					if (map2_->codegree(f) != 3)
						return;
					cgogn::Dart it = oldest_dart(f);
					it = map2_->phi<12>(it); // central face
					if (map2_->codegree(CMap2::Face(it)) != 3)
						return;
					cgogn::Dart it2 = map2_->phi<12>(it); // corner face 1
					if (map2_->codegree(CMap2::Face(it2)) != 3)
						return;
					it2 = map2_->phi2(map2_->phi_1(it)); // corner face 2
					if (map2_->codegree(CMap2::Face(it2)) != 3)
						return;
					break;
				}
				case TRI_CENTRAL: {
					if (map2_->codegree(f) != 3)
						return;
					cgogn::Dart it = f.dart;
					if (map2_->codegree(CMap2::Face(map2_->phi2(it))) != 3) // corner face 1
						return;
					it = map2_->phi1(it);
					if (map2_->codegree(CMap2::Face(map2_->phi2(it))) != 3) // corner face 2
						return;
					it = map2_->phi1(it);
					if (map2_->codegree(CMap2::Face(map2_->phi2(it))) != 3) // corner face 3
						return;
					break;
				}
				case QUAD: {
					if (map2_->codegree(f) != 4)
						return;
					cgogn::Dart it = map2_->phi1(oldest_dart(f));
					it = map2_->phi<12>(it);
					if (map2_->codegree(CMap2::Face(it)) != 4)
						return;
					it = map2_->phi<12>(it);
					if (map2_->codegree(CMap2::Face(it)) != 4)
						return;
					it = map2_->phi<12>(it);
					if (map2_->codegree(CMap2::Face(it)) != 4)
						return;
					break;
				}
			}

			if ( /* a certain condition is met */ true)
			{
				simplify_face(f);
			}
		},
		*qtrav_
	);
}

void Plugin_ShallowWater::subdivide_face(CMap2::Face f, CMap2::CellMarker<CMap2::Face::ORBIT>& subdivided)
{
	f.dart = oldest_dart(f);
	uint8 fl = face_level(f);
	uint8 fid = face_subd_id_[f];

	// check neighbours level
	map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
	{
		if (face_level(af) < fl)
			subdivide_face(af, subdivided);
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
			CMap2::Vertex v = map2_->cut_edge(CMap2::Edge(it));
			dart_level_[map2_->phi1(it)] = fl+1;
			dart_level_[map2_->phi2(it)] = fl+1;
			qtrav_->update(v);
			qtrav_->update(CMap2::Edge(it));
			qtrav_->update(CMap2::Edge(map2_->phi1(it)));
		}
		it = next;
	} while (it != f.dart);

	if (tri_face_[f])
	{
		// cut face into 4 triangles
		it = map2_->phi1(it);
		cgogn::Dart it2 = map2_->phi<11>(it);
		CMap2::Edge e = map2_->cut_face(it, it2);
		dart_level_[e.dart] = fl+1;
		dart_level_[map2_->phi2(e.dart)] = fl+1;
		face_subd_id_[CMap2::Face(it)] = 4*fid+1;
		tri_face_[CMap2::Face(it)] = true;
		subdivided.mark(CMap2::Face(it));
		qtrav_->update(e);
		qtrav_->update(CMap2::Face(it));

		it = map2_->phi<11>(it2);
		e = map2_->cut_face(it, it2);
		dart_level_[e.dart] = fl+1;
		dart_level_[map2_->phi2(e.dart)] = fl+1;
		face_subd_id_[CMap2::Face(it2)] = 4*fid+2;
		tri_face_[CMap2::Face(it2)] = true;
		subdivided.mark(CMap2::Face(it2));
		qtrav_->update(e);
		qtrav_->update(CMap2::Face(it2));

		it2 = map2_->phi<11>(it);
		e = map2_->cut_face(it, it2);
		dart_level_[e.dart] = fl+1;
		dart_level_[map2_->phi2(e.dart)] = fl+1;
		face_subd_id_[CMap2::Face(it)] = 4*fid+3;
		tri_face_[CMap2::Face(it)] = true;
		subdivided.mark(CMap2::Face(it));
		qtrav_->update(e);
		qtrav_->update(CMap2::Face(it));

		face_subd_id_[CMap2::Face(it2)] = 4*fid+4;
		tri_face_[CMap2::Face(it2)] = true;
		subdivided.mark(CMap2::Face(it2));
		qtrav_->update(CMap2::Face(it2));
	}
	else
	{
		// cut face into 4 quads
		it = map2_->phi1(it);
		cgogn::Dart it2 = map2_->phi<11>(it);
		CMap2::Edge e = map2_->cut_face(it, it2);
		dart_level_[e.dart] = fl+1;
		dart_level_[map2_->phi2(e.dart)] = fl+1;
		tri_face_[CMap2::Face(it)] = false;
		subdivided.mark(CMap2::Face(it));
		qtrav_->update(CMap2::Face(it));

		CMap2::Vertex v = map2_->cut_edge(e);
		dart_level_[map2_->phi1(e.dart)] = fl+1;
		dart_level_[map2_->phi2(e.dart)] = fl+1;
		qtrav_->update(v);
		qtrav_->update(e);
		qtrav_->update(CMap2::Edge(map2_->phi1(e.dart)));

		it = map2_->phi2(e.dart);
		it2 = map2_->phi<11>(it2);
		do
		{
			CMap2::Edge ee = map2_->cut_face(it, it2);
			dart_level_[ee.dart] = fl+1;
			dart_level_[map2_->phi2(ee.dart)] = fl+1;
			tri_face_[CMap2::Face(it)] = false;
			subdivided.mark(CMap2::Face(it));
			qtrav_->update(ee);
			qtrav_->update(CMap2::Face(it));

			it = map2_->phi2(map2_->phi_1(it));
			it2 = map2_->phi<11>(it2);
		} while (map2_->phi1(it2) != it);
	}
}

void Plugin_ShallowWater::simplify_face(CMap2::Face f)
{
	// if we are here, f is simplifiable (part of a group of 4 triangle or quad faces)

	uint32 fid = face_subd_id_[f];
	uint8 fl = face_level(f);

	cgogn::Dart resF;

	if (tri_face_[f])
	{
		cgogn::Dart it = f.dart;
		switch (face_type(f))
		{
			case TRI_CORNER: {
				cgogn::Dart od = oldest_dart(f);
				it = map2_->phi<12>(od); // put 'it' in the central face
				resF = od;
				break;
			}
			case TRI_CENTRAL:
				resF = map2_->phi_1(map2_->phi2(f.dart));
				break;
		}

		cgogn::Dart next = map2_->phi1(it);
		map2_->merge_incident_faces(CMap2::Edge(it));
		it = next;
		next = map2_->phi1(it);
		map2_->merge_incident_faces(CMap2::Edge(it));
		it = next;
		map2_->merge_incident_faces(CMap2::Edge(it));

		face_subd_id_[resF] = uint32((fid-1)/4);
		qtrav_->update(CMap2::Face(resF));
	}
	else
	{
		cgogn::Dart od = oldest_dart(f);
		resF = od;
		cgogn::Dart it = map2_->phi<11>(od); // central vertex
		map2_->merge_incident_faces(CMap2::Vertex(it));

		face_subd_id_[resF] = uint32((fid-1)/4);
		qtrav_->update(CMap2::Face(resF));
	}

	// simplify edges (if possible)
	cgogn::Dart it = resF;
	do
	{
		cgogn::Dart next = map2_->phi<11>(it);
		if (face_level(CMap2::Face(map2_->phi2(it))) == fl-1)
		{
			map2_->merge_incident_edges(CMap2::Vertex(map2_->phi1(it)));
			qtrav_->update(CMap2::Edge(it));
		}
		else
		{
			qtrav_->update(CMap2::Vertex(map2_->phi1(it)));
		}
		it = next;
	} while (it != resF);
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

uint8 Plugin_ShallowWater::face_level(CMap2::Face f)
{
	uint32 id = face_subd_id_[f];
	if (id == 0) return 0;
	if (id < 5) return 1;
	if (id < 21) return 2;
	if (id < 85) return 3;
	if (id < 341) return 4;
	if (id < 1365) return 5;
	if (id < 5461) return 6;
}

Plugin_ShallowWater::FaceType Plugin_ShallowWater::face_type(CMap2::Face f)
{
	if (!tri_face_[f])
		return QUAD;
	else if (face_subd_id_[f] % 4 == 0)
		return TRI_CENTRAL;
	else
		return TRI_CORNER;
}

} // namespace plugin_shallow_water_2

} // namespace schnapps
