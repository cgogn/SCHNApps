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

#include <cgogn/modeling/tiling/square_grid.h>
#include <cgogn/geometry/algos/centroid.h>
#include <cgogn/geometry/algos/length.h>

namespace schnapps
{

namespace plugin_shallow_water
{

bool Plugin_ShallowWater::enable()
{
	dock_tab_ = new ShallowWater_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Shallow Water");

	const unsigned int nbc = 100u;

	map_ = static_cast<CMap2Handler*>(schnapps_->add_map("shallow_water", 2));
	map2_ = static_cast<CMap2*>(map_->get_map());
	qtrav_ = cgogn::make_unique<CMap2::QuickTraversor>(*map2_);

	position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position");
	water_position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("water_position");

	scalar_value_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value");

	h_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("hauteur");
	h_tmp_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("hauteur_tmp");
	q_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("debit");
	q_tmp_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("debit_tmp");

	centroid_ = map_->add_attribute<VEC3, CMap2::Face::ORBIT>("centroid");
	length_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("length");
	phi_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("phi");
	zb_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("zb");

	subd_code_ = map_->add_attribute<uint32, CMap2::Face::ORBIT>("subdivision_code");

	f1_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f1");
	f2_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f2");
	s0L_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s0L");
	s0R_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s0R");

	cgogn::modeling::SquareGrid<CMap2> grid(*map2_, nbc, 1);
	grid.embed_into_grid(position_, 200.0f, 25.0f, 0.0f);

	map2_->copy_attribute(water_position_, position_);

	boundaryL_ = CMap2::Edge(grid.vertex_table_[nbc+1].dart);
	boundaryR_ = CMap2::Edge(grid.vertex_table_[nbc].dart);

	qtrav_->build<CMap2::Vertex>();

	qtrav_->build<CMap2::Edge>([&] (CMap2::Edge e) -> cgogn::Dart {
		if (map2_->is_incident_to_boundary(e))
			return map2_->is_boundary(e.dart) ? map2_->phi2(e.dart) : e.dart;
		else
		{
			if (position_[CMap2::Vertex(e.dart)][0] < position_[CMap2::Vertex(map2_->phi_1(e.dart))][0])
				return e.dart;
			else
				return map2_->phi2(e.dart);
		}
	});

	qtrav_->build<CMap2::Face>([&] (CMap2::Face f) -> cgogn::Dart {
		CMap2::Edge e(f.dart);
		if (map2_->is_incident_to_boundary(e))
		{
			if (position_[CMap2::Vertex(f.dart)][0] < position_[CMap2::Vertex(map2_->phi1(f.dart))][0])
				return map2_->phi_1(f.dart);
			else
				return map2_->phi1(f.dart);
		}
		else
		{
			if (position_[CMap2::Vertex(f.dart)][0] < position_[CMap2::Vertex(map2_->phi_1(f.dart))][0])
				return f.dart;
			else
				return map2_->phi1(map2_->phi1(f.dart));
		}
	});

	cgogn::geometry::compute_centroid<VEC3, CMap2::Face>(*map2_, position_, centroid_);

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f, uint32)
		{
			CMap2::Edge e1(f.dart);
			CMap2::Edge e2(map2_->phi1(f.dart));
			phi_[f] = cgogn::geometry::length<VEC3>(*map2_, e1, position_);
			length_[f] = cgogn::geometry::length<VEC3>(*map2_, e2, position_);
			subd_code_[f] = 1;
		},
		*qtrav_
	);

	init();

	timer_ = new QTimer(this);
	connect(timer_, SIGNAL(timeout()), this, SLOT(execute_time_step()));

	return true;
}

void Plugin_ShallowWater::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	schnapps_->remove_map("shallow_water");
	delete dock_tab_;
}

void Plugin_ShallowWater::init()
{
	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f, uint32)
		{
			if (centroid_[f][0] < -75.)
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}
			else if (centroid_[f][0] < -50.)
			{
				h_[f] = 30.;
				zb_[f] = 0.;
			}
			else if (centroid_[f][0] < -25.)
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}
			else if (centroid_[f][0] < 0.)
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}
			else if (centroid_[f][0] < 25.)
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}
			else if (centroid_[f][0] < 50.)
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}
			else if (centroid_[f][0] < 75.)
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}
			else
			{
				h_[f] = 10.;
				zb_[f] = 0.;
			}

			// initial water flow is 0 for all cells
			q_[f] = 0.;
		},
		*qtrav_
	);

	t_ = 0.;
	dt_ = 0.1;

	map2_->parallel_foreach_cell(
		[&] (CMap2::Vertex v, uint32)
		{
			SCALAR wh = 0, bh = 0;
			uint32 nbf = 0;
			map2_->foreach_incident_face(v, [&] (CMap2::Face f)
			{
				wh += h_[f];
				bh += zb_[f];
				++nbf;
			});
			wh /= nbf;
			bh /= nbf;
			water_position_[v][2] = wh;
			position_[v][2] = bh;
			scalar_value_[v] = wh-bh;
		},
		*qtrav_
	);

	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
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

void Plugin_ShallowWater::update_time_step()
{
	std::vector<SCALAR> min_dt_per_thread(cgogn::nb_threads());
	for (SCALAR& d : min_dt_per_thread) d = 0.1;

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f, uint32 idx)
		{
			SCALAR swept = std::fabs(q_[f]) / std::max(h_[f], 1e-10) + std::sqrt(9.81 * h_[f]);
			double dt = length_[f] / std::max(swept, 1e-10);
			min_dt_per_thread[idx] = dt < min_dt_per_thread[idx] ? dt : min_dt_per_thread[idx];
		},
		*qtrav_
	);

	dt_ = *(std::min_element(min_dt_per_thread.begin(), min_dt_per_thread.end()));
}

void Plugin_ShallowWater::execute_time_step()
{
	connectivity_changed_ = false;

	update_time_step();
//	std::cout << dt_ << std::endl;

	f1_[boundaryL_] = 0.;
	f2_[boundaryL_] = 5e-1 * 9.81 * phi_[CMap2::Face(boundaryL_.dart)] * (h_[CMap2::Face(boundaryL_.dart)] * h_[CMap2::Face(boundaryL_.dart)]);
	s0L_[boundaryL_] = 0.;
	s0R_[boundaryL_] = 0.;

	map2_->parallel_foreach_cell(
		[&] (CMap2::Edge e, uint32)
		{
			if (map2_->is_incident_to_boundary(e))
				return;

			CMap2::Face fL(map2_->phi2(e.dart));
			CMap2::Face fR(e.dart);

			struct Flux F = Solv_HLL(zb_[fL], zb_[fR], phi_[fL], phi_[fR], h_[fL], h_[fR], q_[fL], q_[fR], 1e-3, 9.81);
			f1_[e] = F.F1;
			f2_[e] = F.F2;
			s0L_[e] = F.S0L;
			s0R_[e] = F.S0R;
		},
		*qtrav_
	);

	f1_[boundaryR_] = 0.;
	f2_[boundaryR_] = 5e-1 * 9.81 * phi_[CMap2::Face(boundaryR_.dart)] * (h_[CMap2::Face(boundaryR_.dart)] * h_[CMap2::Face(boundaryR_.dart)]);
	s0L_[boundaryR_] = 0.;
	s0R_[boundaryR_] = 0.;

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f, uint32)
		{
			CMap2::Edge eL(f.dart);
			CMap2::Edge eR(map2_->phi1(map2_->phi1(f.dart)));
			h_tmp_[f] = h_[f] + (dt_ / (length_[f] * phi_[f])) * (f1_[eL] - f1_[eR]);
			q_tmp_[f] = q_[f] + (dt_ / (length_[f] * phi_[f])) * ((f2_[eL] - s0L_[eL]) - (f2_[eR] - s0R_[eR]));
		},
		*qtrav_
	);

	map2_->swap_attributes(h_, h_tmp_);
	map2_->swap_attributes(q_, q_tmp_);

	try_simplification();
	try_subdivision();

	map2_->parallel_foreach_cell(
		[&] (CMap2::Vertex v, uint32)
		{
			SCALAR wh = 0, bh = 0;
			uint32 nbf = 0;
			map2_->foreach_incident_face(v, [&] (CMap2::Face f)
			{
				wh += h_[f];
				bh += zb_[f];
				++nbf;
			});
			wh /= nbf;
			bh /= nbf;
			water_position_[v][2] = wh;
			position_[v][2] = bh;
			scalar_value_[v] = wh-bh;
		},
		*qtrav_
	);

	if (connectivity_changed_)
		map_->notify_connectivity_change();
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "position");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value");

	t_ += dt_;
}

void Plugin_ShallowWater::try_subdivision()
{
	CMap2::CellCache cache(*map2_);
	cache.build<CMap2::Face>(*qtrav_);

	map2_->foreach_cell(
		[&] (CMap2::Face f)
		{
			CMap2::Edge eL(f.dart);
			CMap2::Edge eR(map2_->phi1(map2_->phi1(f.dart)));

			if (!map2_->is_incident_to_boundary(eL) && !map2_->is_incident_to_boundary(eR))
			{
				SCALAR g = std::abs(h_[CMap2::Face(map2_->phi2(eL.dart))] - h_[CMap2::Face(map2_->phi2(eR.dart))]);
				if (g > 1. && subd_code_[f] < (1 << 4))
				{
					subdivide_face(f);
					connectivity_changed_ = true;
				}
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
			if (subd_code_[f] % 2 != 0)
				return;

			CMap2::Edge eL(f.dart);
			CMap2::Edge eR(map2_->phi1(map2_->phi1(f.dart)));

			if (!map2_->is_incident_to_boundary(eR))
			{
				CMap2::Face f2(map2_->phi2(eR.dart));
				if (subd_code_[f2] == subd_code_[f] + 1)
				{
					SCALAR g = std::abs(h_[f] - h_[f2]);
					if (g < 0.05)
					{
						remove_edge(eR);
						connectivity_changed_ = true;
					}
				}
			}
		},
		*qtrav_
	);
}

void Plugin_ShallowWater::subdivide_face(CMap2::Face f)
{
	uint32 old_subd_code = subd_code_[f];

	SCALAR old_h = h_[f];
	SCALAR old_q = q_[f];
	SCALAR old_zb = zb_[f];

	CMap2::Edge eL(f.dart);
	CMap2::Edge eR(map2_->phi1(map2_->phi1(f.dart)));

	CMap2::Vertex v1 = map2_->cut_edge(CMap2::Edge(map2_->phi1(eL.dart)));
	CMap2::Vertex v2 = map2_->cut_edge(CMap2::Edge(map2_->phi1(eR.dart)));
	CMap2::Edge e = map2_->cut_face(v2.dart, v1.dart);

	qtrav_->update(v1);
	qtrav_->update(v2);

	qtrav_->update(CMap2::Edge(map2_->phi2(e.dart)));
	qtrav_->update(CMap2::Edge(map2_->phi1(e.dart)));
	qtrav_->update(CMap2::Edge(map2_->phi1(map2_->phi2(e.dart))));

	CMap2::Face fL(eL.dart);
	CMap2::Face fR(map2_->phi2(e.dart));

	qtrav_->update(fR);

	subd_code_[fL] = old_subd_code * 2;
	subd_code_[fR] = old_subd_code * 2 + 1;

	position_[v1] = 0.5 * (position_[CMap2::Vertex(eR.dart)] + position_[CMap2::Vertex(map2_->phi1(eL.dart))]);
	position_[v2] = 0.5 * (position_[CMap2::Vertex(eL.dart)] + position_[CMap2::Vertex(map2_->phi1(eR.dart))]);

	water_position_[v1] = position_[v1];
	water_position_[v2] = position_[v2];

	h_[fL] = old_h;
	h_[fR] = old_h;

	q_[fL] = old_q;
	q_[fR] = old_q;

	centroid_[fL] = cgogn::geometry::centroid<VEC3>(*map2_, fL, position_);
	centroid_[fR] = cgogn::geometry::centroid<VEC3>(*map2_, fR, position_);

	length_[fL] = cgogn::geometry::length<VEC3>(*map2_, CMap2::Edge(map2_->phi1(eL.dart)), position_);
	length_[fR] = cgogn::geometry::length<VEC3>(*map2_, CMap2::Edge(map2_->phi1(eR.dart)), position_);

	phi_[fL] = cgogn::geometry::length<VEC3>(*map2_, eL, position_);
	phi_[fR] = cgogn::geometry::length<VEC3>(*map2_, eR, position_);

	zb_[fL] = old_zb;
	zb_[fR] = old_zb;
}

void Plugin_ShallowWater::remove_edge(CMap2::Edge e)
{
	CMap2::Face f1(e.dart);
	CMap2::Face f2(map2_->phi2(e.dart));

	uint32 old_subd_code_1 = subd_code_[f1];
	uint32 old_subd_code_2 = subd_code_[f2];

	SCALAR old_h_1 = h_[f1];
	SCALAR old_h_2 = h_[f2];
	SCALAR old_q_1 = q_[f1];
	SCALAR old_q_2 = q_[f2];
	SCALAR old_zb_1 = zb_[f1];
	SCALAR old_zb_2 = zb_[f2];

	cgogn::Dart d1 = map2_->phi_1(e.dart);
	cgogn::Dart d2 = map2_->phi_1(map2_->phi2(e.dart));

	CMap2::Face f(d1);
	CMap2::Vertex v1(map2_->phi_1(d1));
	CMap2::Vertex v2(map2_->phi_1(d2));

	map2_->merge_incident_faces(e);

	CMap2::Builder builder(*map2_);
	builder.collapse_edge_topo(map2_->phi1(d1));
	builder.collapse_edge_topo(map2_->phi1(d2));
	builder.template set_orbit_embedding<CMap2::Vertex>(v1, map2_->embedding(v1));
	builder.template set_orbit_embedding<CMap2::Vertex>(v2, map2_->embedding(v2));

	subd_code_[f] = old_subd_code_1 / 2;

	h_[f] = old_h_1;
	q_[f] = old_q_1;
	zb_[f] = old_zb_1;

	centroid_[f] = cgogn::geometry::centroid<VEC3>(*map2_, f, position_);
	length_[f] = cgogn::geometry::length<VEC3>(*map2_, CMap2::Edge(d1), position_);
	phi_[f] = cgogn::geometry::length<VEC3>(*map2_, CMap2::Edge(map2_->phi_1(d1)), position_);
}

//std::pair<CMap2::Edge, CMap2::Edge> Plugin_ShallowWater::get_LR_edges(CMap2::Face f)
//{
//	CMap2::Edge eL, eR;
//	CMap2::Edge e(f.dart);
//	if (map2_->is_incident_to_boundary(e))
//	{
//		if (map2_->same_cell(e, boundaryL_))
//		{
//			eL = e;
//			eR = CMap2::Edge(map2_->phi1(map2_->phi1(eL.dart)));
//		}
//		else if (map2_->same_cell(e, boundaryR_))
//		{
//			eR = e;
//			eL = CMap2::Edge(map2_->phi1(map2_->phi1(eR.dart)));
//		}
//		else
//		{
//			if (position_[CMap2::Vertex(f.dart)][0] < position_[CMap2::Vertex(map2_->phi1(f.dart))][0])
//			{
//				eL = CMap2::Edge(map2_->phi_1(f.dart));
//				eR = CMap2::Edge(map2_->phi1(f.dart));
//			}
//			else
//			{
//				eL = CMap2::Edge(map2_->phi1(f.dart));
//				eR = CMap2::Edge(map2_->phi_1(f.dart));
//			}
//		}
//	}
//	else
//	{
//		if (position_[CMap2::Vertex(f.dart)][0] < position_[CMap2::Vertex(map2_->phi_1(f.dart))][0])
//		{
//			eL = CMap2::Edge(f.dart);
//			eR = CMap2::Edge(map2_->phi1(map2_->phi1(f.dart)));
//		}
//		else
//		{
//			eL = CMap2::Edge(map2_->phi1(map2_->phi1(f.dart)));
//			eR = CMap2::Edge(f.dart);
//		}
//	}

//	return std::make_pair(eL, eR);
//}

//std::pair<CMap2::Face, CMap2::Face> Plugin_ShallowWater::get_LR_faces(CMap2::Edge e)
//{
//	CMap2::Face fL, fR;
//	if (position_[CMap2::Vertex(e.dart)][0] < position_[CMap2::Vertex(map2_->phi_1(e.dart))][0])
//	{
//		fL = CMap2::Face(map2_->phi2(e.dart));
//		fR = CMap2::Face(e.dart);
//	}
//	else
//	{
//		fL = CMap2::Face(e.dart);
//		fR = CMap2::Face(map2_->phi2(e.dart));
//	}

//	return std::make_pair(fL, fR);
//}

struct Plugin_ShallowWater::Flux Plugin_ShallowWater::Solv_HLL(
	SCALAR zbL, SCALAR zbR,
	SCALAR PhiL, SCALAR PhiR,
	SCALAR hL, SCALAR hR,
	SCALAR qL, SCALAR qR,
	SCALAR hmin, SCALAR g
)
/* Calcul du flux à l'interface avec le solveur HLL*/
{
	struct Flux F;

	if (((hL > hmin) && (hR > hmin)) ||
		((hL <= hmin) && (zbR + hR >= zbL)) ||
		((hR <= hmin) && (zbL + hL >= zbR)))
	{
		/* There is water in both cells or one of the cells can fill the other one */
		SCALAR L1L = qL/std::max(hmin,hL) - sqrt(g * hL);
		SCALAR L1R = qR/std::max(hmin,hR) - sqrt(g * hR);
		SCALAR L2L = qL/std::max(hmin,hL) + sqrt(g * hL);
		SCALAR L2R = qR/std::max(hmin,hR) + sqrt(g * hR);
		SCALAR L1LR = std::min(0e1,std::min(L1L,L1R));
		SCALAR L2LR = std::max(0e1,std::max(L2L,L2R));
		SCALAR zL = hL + zbL;
		SCALAR zR = hR + zbR;
		SCALAR PhiLR = std::min(PhiL, PhiR);

		F.F1 = PhiLR * (L2LR * qL - L1LR * qR + L1LR * L2LR * (zR - zL)) / (L2LR - L1LR);
		F.F2 = (L2LR * PhiL * (pow(qL,2)/std::max(hmin,hL) + 5e-1 * g * pow(hL,2)) -
				L1LR * PhiR * (pow(qR,2)/std::max(hmin,hR) + 5e-1 * g * pow(hR,2)) +
				L1LR * L2LR * (PhiR * qR - PhiL * qL)) / (L2LR - L1LR);

		F.S0L = g * L1LR * (5e-1 * (PhiL * pow(hL,2) - PhiR * pow(hR,2)) - 5e-1 * PhiLR * (hL + hR) * (zL - zR)) / (L2LR - L1LR);
		F.S0R = g * L2LR * (5e-1 * (PhiR * pow(hR,2) - PhiL * pow(hL,2)) - 5e-1 * PhiLR * (hL + hR) * (zR - zL)) / (L2LR - L1LR);
	}
	else if ((hL < hmin) && (zbR + hR < zbL))
	{
		/* Impossible exchange - Cell L Empty
		 * The cell L is empty and the water level in the cell R is below zbL
		 * Filling is impossible */
		F.F1 = 0;
		F.F2 = 0;
		F.S0L = 0;
		F.S0R = 5e-1 * g * PhiR * pow(hR,2);
	}
	else if ((hR < hmin) && (zbL + hL < zbR))
	{
		/* Impossible exchange - Cell R Empty
		 * The cell R is empty and the water level in the cell L is below zbR
		 * Filling is impossible */
		F.F1 = 0;
		F.F2 = 0;
		F.S0L = - 5e-1 * g * PhiL * pow(hL,2);
		F.S0R = 0;
	 }
	 else
	 {
		/* Both cells below hmin: exchange is impossible */
		F.F1 = 0;
		F.F2 = 0;
		F.S0L = 0;
		F.S0R = 0;
	 }

	return F;
}

} // namespace plugin_shallow_water

} // namespace schnapps
