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

#include <QDebug> // affichage dans la console

namespace schnapps
{

namespace plugin_shallow_water
{

bool Plugin_ShallowWater::enable()
{
	dock_tab_ = new ShallowWater_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Shallow Water");

    float size = parameters();
    const unsigned int nbc = nbr_cell_;

	map_ = static_cast<CMap2Handler*>(schnapps_->add_map("shallow_water", 2));
	map2_ = static_cast<CMap2*>(map_->get_map());
	qtrav_ = cgogn::make_unique<CMap2::QuickTraversor>(*map2_);

	position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position");
    water_position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("water_position");
    flow_velocity_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("flow_velocity");

    scalar_value_water_position_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_water_position");
    scalar_value_flow_velocity_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_flow_velocity");

	h_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("hauteur");
	h_tmp_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("hauteur_tmp");
	q_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("debit");
	q_tmp_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("debit_tmp");

	centroid_ = map_->add_attribute<VEC3, CMap2::Face::ORBIT>("centroid");
    length_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("length");
    phi_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("phi");
    error_h_ = map_->add_attribute<SCALAR,CMap2::Face::ORBIT>("error_h");
    error_u_ = map_->add_attribute<SCALAR,CMap2::Face::ORBIT>("error_u");

	subd_code_ = map_->add_attribute<uint32, CMap2::Face::ORBIT>("subdivision_code");

	f1_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f1");
	f2_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f2");
	s0L_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s0L");
	s0R_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s0R");

    cgogn::modeling::SquareGrid<CMap2> grid(*map2_, nbc, 1);
    grid.embed_into_grid(position_, size, 25.0f, 0.0f);

    map2_->copy_attribute(water_position_, position_);
    map2_->copy_attribute(flow_velocity_, position_);

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
           /*if (centroid_[f][0] < -75.)
                h_[f] = 10.;
            else if (centroid_[f][0] < -50.)
                h_[f] = 3.;
            else if (centroid_[f][0] < -25.)
                h_[f] = 8.;
            else if (centroid_[f][0] < 0.)
                h_[f] = 1.;
            else if (centroid_[f][0] < 25.)
                h_[f] = 10.;
            else if (centroid_[f][0] < 50.)
                h_[f] = 1.;
            else if (centroid_[f][0] < 75.)
                h_[f] = 6.;
            else
                h_[f] = 1.;
        //h_[f] = initial_right_flow_velocity_ + initial_left_water_position_*exp(-pow(centroid_[f][0],2.));
           q_[f] = 0.;*/

            // rupture de barrage sur fond mouillé
            if(centroid_[f][0] < 0.)
            {
                h_[f] = initial_left_water_position_;
                q_[f] = initial_left_flow_velocity_*initial_left_water_position_;
            }
            else
            {
                h_[f] = initial_right_water_position_;
                q_[f] = initial_right_flow_velocity_*initial_right_water_position_;
            }
        },
        *qtrav_
    );

    // constantes pour la solution exacte
    exact_solution_constant_calcul();

    t_ = 0.;    
    dt_max_ = (initial_left_water_position_-initial_right_water_position_)/100.;
    nbr_time_step_ = 0;

    map2_->parallel_foreach_cell(
        [&] (CMap2::Vertex v, uint32)
        {
            SCALAR h = 0;
            SCALAR q = 0;
            uint32 nbf = 0;
            map2_->foreach_incident_face(v, [&] (CMap2::Face f) { h += h_[f]; q+= q_[f]; ++nbf; });
            h /= nbf;
            q /= nbf;
            water_position_[v][2] = h;
            scalar_value_water_position_[v] = h;
            flow_velocity_[v][2] = q/h;
            scalar_value_flow_velocity_[v] = q/h;
        },
        *qtrav_
    );

    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_water_position");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_flow_velocity");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "flow_velocity");

}

void Plugin_ShallowWater::start()
{
	if (!timer_->isActive())
	{
		timer_->start(0);
		schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
        t_begin_ = clock();
    }
}

void Plugin_ShallowWater::stop()
{
	if (timer_->isActive())
	{
		timer_->stop();
        schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
        t_end_ = clock();
        qDebug() << (float)(t_end_-t_begin_)/CLOCKS_PER_SEC;
    }
}

bool Plugin_ShallowWater::is_running()
{
	return timer_->isActive();
}

void Plugin_ShallowWater::update_time_step()
{
	std::vector<SCALAR> min_dt_per_thread(cgogn::nb_threads());
    for (SCALAR& d : min_dt_per_thread) d = dt_max_;

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
    dt_ = std::min(1.-t_,dt_); // on s'arrête exactement à t = 1 pour comparer les erreurs, quitte à avoir un pas de temps plus petit, mais pour une seule itération
}

void Plugin_ShallowWater::execute_time_step()
{
    connectivity_changed_ = false;

    update_time_step();

    // resolution espace
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

            struct Flux F = Solv_HLL(0., 0., phi_[fL], phi_[fR], h_[fL], h_[fR], q_[fL], q_[fR], 1e-3, 9.81);
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

    // resolution temps
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
    //difference_measure();

    t_ += dt_;
    nbr_time_step_++;

    // calcul de l'erreur locale
    map2_->parallel_foreach_cell(
        [&] (CMap2::Face f, uint32)
        {
            SCALAR h,u;
            exact_solution(centroid_[f][0],h,u);
            error_h_[f] = std::abs(h_[f] - h);
            error_u_[f] = std::abs(q_[f]/h_[f]-u);
        },
        *qtrav_
    );

    /* calcul de l'erreur globale
     * pour la norme L2 on multiplie par la taille de la maille pour que l'erreur des petites mailles n'ait pas le même poids que l'erreur des grosses mailles
     * on ne mesure pas l'erreur autour du choc
     */
    std::vector<SCALAR> error_max_h_per_thread(cgogn::nb_threads());
    for (SCALAR& d : error_max_h_per_thread) d = 0.;
    std::vector<SCALAR> error_max_u_per_thread(cgogn::nb_threads());
    for (SCALAR& d : error_max_u_per_thread) d = 0.;
    std::vector<SCALAR> error_2_h_per_thread(cgogn::nb_threads());
    for (SCALAR& d : error_2_h_per_thread) d = 0.;
    std::vector<SCALAR> error_2_u_per_thread(cgogn::nb_threads());
    for (SCALAR& d : error_2_u_per_thread) d = 0.;
    SCALAR shock_abscissa = h_exact_solution_*u_exact_solution_*t_/(h_exact_solution_-initial_right_water_position_);
    map2_->parallel_foreach_cell(
        [&] (CMap2::Face f, uint32 idx)
        {
            if((centroid_[f][0] > shock_abscissa-3.5 && centroid_[f][0] < shock_abscissa+3.5) || (centroid_[f][0] < -80))
                return;


            error_max_h_per_thread[idx] = std::max(error_max_h_per_thread[idx], error_h_[f]);
            error_max_u_per_thread[idx] = std::max(error_max_u_per_thread[idx], error_u_[f]);
            error_2_h_per_thread[idx] += pow(error_h_[f],2.)*length_[f];
            error_2_u_per_thread[idx] += pow(error_u_[f],2.)*length_[f];
        },
        *qtrav_
    );
    error_h_max_ = *(std::max_element(error_max_h_per_thread.begin(), error_max_h_per_thread.end()));
    error_u_max_ = *(std::max_element(error_max_u_per_thread.begin(), error_max_u_per_thread.end()));
    error_h_2_ = 0.;
    error_u_2_ = 0.;
    for (SCALAR& d : error_2_h_per_thread)
        error_h_2_ += d;
    for (SCALAR& d : error_2_u_per_thread)
        error_u_2_ += d;
    error_h_2_ = sqrt(error_h_2_);
    error_u_2_ = sqrt(error_u_2_);

    map2_->parallel_foreach_cell(
        [&] (CMap2::Vertex v, uint32)
        {
            SCALAR h = 0;
            SCALAR q = 0;
            uint32 nbf = 0;
            map2_->foreach_incident_face(v, [&] (CMap2::Face f)
            {
                h += h_[f];
                q += q_[f];
                ++nbf;
            });

            h /= nbf;
            q /= nbf;
            water_position_[v][2] = h;
            flow_velocity_[v][2] = q/h; // u
            scalar_value_water_position_[v] = h;
            scalar_value_flow_velocity_[v] = q/h; // u
        },
        *qtrav_
    );

    if (connectivity_changed_)
        map_->notify_connectivity_change();

    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "position");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_water_position");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_flow_velocity");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "flow_velocity");

    if(t_ >= 1.)
    //if(nbr_time_step_ == 1)
    {        
        qDebug() << "nbr iterations temps : " << nbr_time_step_;
        qDebug() << "dt : " << dt_;
        qDebug() << "error_h_2_ : " <<  error_h_2_;
        qDebug() << "error_q_2_ : " << error_u_2_;
        qDebug() << "error_h_max_ : " << error_h_max_;
        qDebug() << "error_q_max_ : " << error_u_max_;
        qDebug() << "nombre de mailles : " << nbr_cell_;
        stop();

        /*std::ofstream file ("dt_nbr_maillles.txt",std::ios::app);
        file << nbr_cell_ << "\t" << error_h_2_ << "\t" << error_u_2_ << "\t" << error_h_max_ << "\t" << error_u_max_ << std::endl;*/

        /*QFile file("plot.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream flux(&file);
        flux.setCodec("UTF-8");
        SCALAR h,u;
        map2_->foreach_cell([&] (CMap2::Face f)
        {
            exact_solution(centroid_[f][0],h,u);
            SCALAR u_f = q_[f]/h_[f]; // si hR = 0, q_[f]/h_[f] peut être nan
            if(u_f != u_f)
                u_f = u;
            flux << centroid_[f][0] << "\t" << h_[f] << "\t" << h << "\t" << u_f << "\t" << u << endl;
        }, *qtrav_);*/
    }
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
                /* seulement 4  cas possibles
                 * pour h seul OU q seul
                 * en fonction du sens de la vague
                 * et des hauteurs relatives d'eau
                 * avec les différences de valeurs positives ou négatives
                 * car l'onde se deplace de au plus une cellule à chaque pas de temps
                 */
                CMap2::Face fR(map2_->phi2(eR.dart));
                SCALAR diff_h_right = h_[f] - h_[fR];
                SCALAR diff_q_right = q_[f] - q_[fR];

                CMap2::Face fL(map2_->phi1(map2_->phi1(map2_->phi2(eL.dart))));
                SCALAR diff_h_left = h_[f] - h_[fL];
                SCALAR diff_q_left = q_[f] - q_[fL];

                if((diff_h_right > h_difference_/50. || diff_q_right > q_difference_/50. ) && subd_code_[f] < (1 << 4) && subd_code_[fL] < (1 << 4))
                {
                    subdivide_face(f);
                    subdivide_face(fL);
                    connectivity_changed_ = true;
                }
                else if((diff_h_left < -h_difference_/50. || diff_q_left < -q_difference_/50. ) && subd_code_[f] < (1 << 4) && subd_code_[fR] < (1 << 4))
                {
                    subdivide_face(f);
                    subdivide_face(fR);
                    connectivity_changed_ = true;
                }
                else if((diff_h_right < -h_difference_/50. || diff_q_right < -q_difference_/50. ) && subd_code_[f] < (1 << 4) && subd_code_[fL] < (1 << 4))
                {
                    subdivide_face(f);
                    subdivide_face(fL);
                    connectivity_changed_ = true;
                }
                else if((diff_h_left > h_difference_/50. || diff_q_left > q_difference_/50. ) && subd_code_[f] < (1 << 4) && subd_code_[fR] < (1 << 4))
                {
                    subdivide_face(f);
                    subdivide_face(fR);
                    connectivity_changed_ = true;
                }
            }
            else if(map2_->is_incident_to_boundary(eL))
            {

                CMap2::Face fR(map2_->phi2(eR.dart));
                SCALAR diff_h_right = h_[f] - h_[fR];
                SCALAR diff_q_right = q_[f] - q_[fR];
                if((diff_h_right > h_difference_/50. || diff_q_right > q_difference_/50. ) && subd_code_[f] < (1 << 4))
                {
                    subdivide_face(f);
                    connectivity_changed_ = true;
                }
                else if((diff_h_right < -h_difference_/50. || diff_q_right < -q_difference_/50. ) && subd_code_[f] < (1 << 4))
                {
                    subdivide_face(f);
                    connectivity_changed_ = true;
                }

            }
            else if(map2_->is_incident_to_boundary(eR))
            {
                CMap2::Face fL(map2_->phi1(map2_->phi1(map2_->phi2(eL.dart))));
                SCALAR diff_h_left = h_[f] - h_[fL];
                SCALAR diff_q_left = q_[f] - q_[fL];
                if((diff_h_left < -h_difference_/50. || diff_q_left < -q_difference_/50. ) && subd_code_[f] < (1 << 5))
                {
                    subdivide_face(f);
                    connectivity_changed_ = true;
                }
                else if((diff_h_left > h_difference_/50. || diff_q_left > q_difference_/50. ) && subd_code_[f] < (1 << 5))
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

				CMap2::Face f2(map2_->phi2(eR.dart));
                if (subd_code_[f2] == subd_code_[f] + 1)
				{
                    SCALAR diff_h = std::abs(h_[f] - h_[f2]);
                    SCALAR diff_q = std::abs(q_[f] - q_[f2]);
                    if (diff_h < 0.7 && diff_q < 0.1)
					{
						remove_edge(eR);
                        connectivity_changed_ = true;
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
    flow_velocity_[v1] = position_[v1];
    flow_velocity_[v2] = position_[v2];

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

    nbr_cell_++;
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

    SCALAR old_l_1 = length_[f1];
    SCALAR old_l_2 = length_[f2];

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

    h_[f] = (old_l_1*old_h_1+old_l_2*old_h_2)/(old_l_1+old_l_2);
    q_[f] = (old_l_1*old_q_1+old_l_2*old_q_2)/(old_l_1+old_l_2);

	centroid_[f] = cgogn::geometry::centroid<VEC3>(*map2_, f, position_);
	length_[f] = cgogn::geometry::length<VEC3>(*map2_, CMap2::Edge(d1), position_);
	phi_[f] = cgogn::geometry::length<VEC3>(*map2_, CMap2::Edge(map2_->phi_1(d1)), position_);

    nbr_cell_--;
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

void Plugin_ShallowWater::exact_solution_constant_calcul()
{
    // methode de Newton
    SCALAR L = initial_left_flow_velocity_+2.*sqrt(9.81*initial_left_water_position_);
    if(initial_right_water_position_ == 0)
        h_exact_solution_ = 0.;
    else
    {
        int i = 0;
        SCALAR hPrec = 0.;
        SCALAR h = (initial_left_water_position_+initial_right_water_position_)/2.;
        SCALAR den,num;
        SCALAR R = initial_right_water_position_*initial_right_flow_velocity_*initial_right_flow_velocity_+9.81*initial_right_water_position_*initial_right_water_position_/2.;
        while(i < 100 && std::abs(h-hPrec) > 1e-12)
        {
            i++;
            hPrec = h;
            num = L*L*h*h+4.*9.81*h*h*h-4.*L*sqrt(9.81)*pow(h,5./2.)-2.*initial_right_water_position_*initial_right_flow_velocity_*L*h+4.*sqrt(9.81)*initial_right_water_position_*initial_right_flow_velocity_*pow(h,3./2.)+initial_right_water_position_*initial_right_water_position_*initial_right_flow_velocity_*initial_right_flow_velocity_ - (h-initial_right_water_position_)*(h*L*L+9.*9.81*h*h/2.-4.*L*sqrt(9.81)*pow(h,3./2.)-R);
            den = 2.*L*L*h+12.*9.81*h*h-10*L*sqrt(9.81)*pow(h,3./2.)-2.*initial_right_water_position_*initial_right_flow_velocity_*L+4.*initial_right_water_position_*initial_right_flow_velocity_*sqrt(9.81*h) - (h*L*L+9.*9.81*h*h/2.-4.*sqrt(9.81)*L*pow(h,3./2.)-R) - (h-initial_right_water_position_)*(L*L+9.*9.81*h-4.*L*sqrt(9.81*h));
            h = h - num/den;
        }
        h_exact_solution_ = h;
    }
    assert(h_exact_solution_ <= initial_left_water_position_ && h_exact_solution_ >= initial_right_water_position_);
    u_exact_solution_ = L-2.*sqrt(9.81*h_exact_solution_);
}

void Plugin_ShallowWater::exact_solution(SCALAR x, SCALAR& h, SCALAR& u)
{
    if(x <= (initial_left_flow_velocity_-sqrt(9.81*initial_left_water_position_))*t_)
    {
        h = initial_left_water_position_;
        u = initial_left_flow_velocity_;
    }
    else if (x <= (u_exact_solution_ - sqrt(9.81*h_exact_solution_))*t_)
    {
        h = pow(x/t_-2.*sqrt(9.81*initial_left_water_position_),2.)/9./9.81;
        u = 2.*(x/t_+sqrt(9.81*initial_left_water_position_))/3.;
    }
    else if(x <= (initial_left_water_position_*initial_left_flow_velocity_+u_exact_solution_*h_exact_solution_)/(h_exact_solution_-initial_right_water_position_)*t_)
    {
        h = h_exact_solution_;
        u =u_exact_solution_;
    }
    else
    {
        h = initial_right_water_position_;
        u = initial_right_flow_velocity_;
    }
}

void Plugin_ShallowWater::difference_measure()
{
    map2_->foreach_cell(
        [&] (CMap2::Face f)
        {
            CMap2::Edge eL(f.dart);
            CMap2::Edge eR(map2_->phi1(map2_->phi1(f.dart)));

            if (!map2_->is_incident_to_boundary(eL) && !map2_->is_incident_to_boundary(eR))
            {
                CMap2::Face f2(map2_->phi2(eR.dart));
                SCALAR diff_h = std::abs(h_[f] - h_[f2]);
                SCALAR diff_q = std::abs(q_[f] - q_[f2]);
                h_difference_ = std::max(h_difference_,diff_h);
                q_difference_ = std::max(q_difference_,diff_q);
            }
        },
        *qtrav_
    );
}

float Plugin_ShallowWater::parameters()
{
    float size;
    SCALAR c;
    std::ifstream file("parametres.txt",std::ios::in);
    file >> initial_left_water_position_;
    file >> initial_right_water_position_;
    file >> initial_left_flow_velocity_;
    file >> initial_right_flow_velocity_;
    file >> h_difference_;
    file >> q_difference_;
    file >> size;
    file >> c;
    file.close();
    nbr_cell_ = c*(initial_left_water_position_-initial_right_water_position_)*size/2;
    return size;
}

} // namespace plugin_shallow_water

} // namespace schnapps
