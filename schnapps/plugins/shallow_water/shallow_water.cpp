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

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_dock_tab()));

	update_dock_tab();

	return true;
}

void Plugin_ShallowWater::disable()
{
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

void Plugin_ShallowWater::draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv)
{

}

void Plugin_ShallowWater::view_linked(View*)
{
	update_dock_tab();
}

void Plugin_ShallowWater::view_unlinked(View*)
{
	update_dock_tab();
}

void Plugin_ShallowWater::init()
{
	const unsigned int nbc = 200u;

	map_ = static_cast<MapHandler<CMap2>*>(schnapps_->add_map("shallow_water", 2));
	CMap2* map2 = static_cast<CMap2*>(map_->get_map());

	position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position");

	water_height_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("water_height");

	h_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("hauteur");
	h_tmp_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("hauteur_tmp");
	q_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("debit");
	q_tmp_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("debit_tmp");
	centroid_ = map_->add_attribute<VEC3, CMap2::Face::ORBIT>("centroid");
	length_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("length");
	phi_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("phi");

	f1_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f1");
	f2_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f2");
	s0L_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s0L");
	s0R_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s0R");

	cgogn::modeling::SquareGrid<CMap2> grid(*map2, nbc, 1);
	grid.embed_into_grid(position_, float(nbc), 5.0f, 0.0f);
	boundaryL_ = CMap2::Edge(grid.vertex_table_[nbc+1].dart);
	boundaryR_ = CMap2::Edge(grid.vertex_table_[nbc].dart);

	cgogn::geometry::compute_centroid<VEC3, CMap2::Face>(*map2, position_, centroid_);

	map2->parallel_foreach_cell([&] (CMap2::Face f, uint32)
	{
		CMap2::Edge e1(f.dart);
		CMap2::Edge e2(map2->phi1(f.dart));
		if (map2->is_incident_to_boundary(e1))
		{
			length_[f] = cgogn::geometry::length<VEC3>(*map2, e1, position_);
			phi_[f] = cgogn::geometry::length<VEC3>(*map2, e2, position_);
		}
		else
		{
			phi_[f] = cgogn::geometry::length<VEC3>(*map2, e1, position_);
			length_[f] = cgogn::geometry::length<VEC3>(*map2, e2, position_);
		}

		// left half of the cells has initial water height of 10
		if (centroid_[f][0] < 0)
			h_[f] = 10.;
		// right half of the cells has initial water height of 1
		else
			h_[f] = 1.;

		// initial water flow is 0 for all cells
		q_[f] = 0.;
	});

	map2->parallel_foreach_cell([&] (CMap2::Vertex v, uint32)
	{
		SCALAR h = 0;
		uint32 nbf = 0;
		map2->foreach_incident_face(v, [&] (CMap2::Face f) { h += h_[f]; ++nbf; });
		h /= nbf;
		water_height_[v] = h;
	});

	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_height");
}

void Plugin_ShallowWater::start()
{
	CMap2* map2 = static_cast<CMap2*>(map_->get_map());

	SCALAR dt = 0.01;
	SCALAR t = 0.;

	while (t < 5.)
	{
		f1_[boundaryL_] = 0.;
		f2_[boundaryL_] = 5e-1 * 9.81 * phi_[0] * (h_[0] * h_[0]);
		s0L_[boundaryL_] = 0.;
		s0R_[boundaryL_] = 0.;

		map2->parallel_foreach_cell(
			[&] (CMap2::Edge e, uint32)
			{
				CMap2::Face fL, fR;
				if (position_[CMap2::Vertex(e.dart)][0] < position_[CMap2::Vertex(map2->phi_1(e.dart))][0])
				{
					fL = CMap2::Face(map2->phi2(e.dart));
					fR = CMap2::Face(e.dart);
				}
				else
				{
					fL = CMap2::Face(e.dart);
					fR = CMap2::Face(map2->phi2(e.dart));
				}
				struct Flux F = Solv_HLL(0., 0., phi_[fL], phi_[fR], h_[fL], h_[fR], q_[fL], q_[fR], 1e-3, 9.81);
				f1_[e] = F.F1;
				f2_[e] = F.F2;
				s0L_[e] = F.S0L;
				s0R_[e] = F.S0R;
			},
			[&] (CMap2::Edge e) { return !map2->is_incident_to_boundary(e); }
		);

		f1_[boundaryR_] = f1_[CMap2::Edge(map2->phi1(map2->phi1(boundaryR_.dart)))];
		f2_[boundaryR_] = f2_[CMap2::Edge(map2->phi1(map2->phi1(boundaryR_.dart)))];
		s0L_[boundaryR_] = 0.;
		s0R_[boundaryR_] = 0.;

		map2->parallel_foreach_cell(
			[&] (CMap2::Face f, uint32)
			{
				CMap2::Edge eL, eR;
				CMap2::Edge e(f.dart);
				if (map2->is_incident_to_boundary(e))
				{
					if (map2->same_cell(e, boundaryL_))
					{
						eL = e;
						eR = CMap2::Edge(map2->phi1(map2->phi1(eL.dart)));
					}
					else if (map2->same_cell(e, boundaryR_))
					{
						eR = e;
						eL = CMap2::Edge(map2->phi1(map2->phi1(eR.dart)));
					}
					else
					{
						if (position_[CMap2::Vertex(f.dart)][0] < position_[CMap2::Vertex(map2->phi1(f.dart))][0])
						{
							eL = CMap2::Edge(map2->phi_1(f.dart));
							eR = CMap2::Edge(map2->phi1(f.dart));
						}
						else
						{
							eL = CMap2::Edge(map2->phi1(f.dart));
							eR = CMap2::Edge(map2->phi_1(f.dart));
						}
					}
				}
				else
				{
					if (position_[CMap2::Vertex(f.dart)][0] < position_[CMap2::Vertex(map2->phi_1(f.dart))][0])
					{
						eL = CMap2::Edge(f.dart);
						eR = CMap2::Edge(map2->phi1(map2->phi1(f.dart)));
					}
					else
					{
						eL = CMap2::Edge(map2->phi1(map2->phi1(f.dart)));
						eR = CMap2::Edge(f.dart);
					}
				}

				h_tmp_[f] = h_[f] + (dt / (length_[f] * phi_[f])) * (f1_[eL] - f1_[eR]);
				q_tmp_[f] = q_[f] + (dt / (length_[f] * phi_[f])) * (f2_[eL] - s0L_[eL] - (f2_[eR] - s0R_[eR]));
			}
		);

		map2->swap_attributes(h_, h_tmp_);
		map2->swap_attributes(q_, q_tmp_);

		t += dt;

		map2->parallel_foreach_cell([&] (CMap2::Vertex v, uint32)
		{
			SCALAR h = 0;
			uint32 nbf = 0;
			map2->foreach_incident_face(v, [&] (CMap2::Face f) { h += h_[f]; ++nbf; });
			h /= nbf;
			water_height_[v] = h;
		});

		map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_height");

		for (View* v : this->get_linked_views())
			v->update();
	}
}

void Plugin_ShallowWater::update_dock_tab()
{
	View* view = schnapps_->get_selected_view();
	if (view->is_linked_to_plugin(this))
		schnapps_->enable_plugin_tab_widgets(this);
	else
		schnapps_->disable_plugin_tab_widgets(this);
}

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
		SCALAR L1L;
		SCALAR L1R;
		SCALAR L2L;
		SCALAR L2R;
		SCALAR L1LR;
		SCALAR L2LR;
		SCALAR PhiLR;
		SCALAR zL;
		SCALAR zR;

		L1L = qL/std::max(hmin,hL) - sqrt(g * hL);
		L1R = qR/std::max(hmin,hR) - sqrt(g * hR);
		L2L = qL/std::max(hmin,hL) + sqrt(g * hL);
		L2R = qR/std::max(hmin,hR) + sqrt(g * hR);
		L1LR = std::min(0e1,std::min(L1L,L1R));
		L2LR = std::max(0e1,std::max(L2L,L2R));
		zL = hL + zbL;
		zR = hR + zbR;
		PhiLR = std::min(PhiL, PhiR);

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
