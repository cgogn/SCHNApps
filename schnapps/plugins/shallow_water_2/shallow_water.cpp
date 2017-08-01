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
#include <cgogn/modeling/tiling/square_grid.h>

#include <cgogn/geometry/algos/centroid.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/area.h>

#include <cgogn/io/map_import.h>

namespace schnapps
{

namespace plugin_shallow_water_2
{

bool Plugin_ShallowWater::enable()
{
    dock_tab_ = new ShallowWater_DockTab(this->schnapps_, this);
    schnapps_->add_plugin_dock_tab(this, dock_tab_, "Shallow Water 2");

    const unsigned int nbc = 100u;

    map_ = static_cast<CMap2Handler*>(schnapps_->add_map("shallow_water_2", 2));
    map2_ = static_cast<CMap2*>(map_->get_map());
    qtrav_ = cgogn::make_unique<CMap2::QuickTraversor>(*map2_);

    /*cgogn::io::import_surface<VEC3>(*map2_, "/home/bloch/Dev/maillages/dambreak.2dm");
    position_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("position");*/

    position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("position");
    cgogn::modeling::SquareGrid<CMap2> grid(*map2_, nbc, nbc);
    grid.embed_into_grid(position_, 200.0f, 200.0f, 0.0f);

    /*SCALAR max_x = 0.;
    SCALAR max_y = 0.;
    SCALAR max_z = 0.;
    SCALAR min_x = 1000.;
    SCALAR min_y = 1000.;
    SCALAR min_z = 1000.;
    for(VEC3& p : position_)
    {
        max_x = std::max(max_x,p[0]);
        min_x = std::min(min_x,p[0]);
        max_y = std::max(max_y,p[1]);
        min_y = std::min(min_y,p[1]);
        max_z = std::max(max_z,p[2]);
        min_z = std::min(min_z,p[2]);
    }
    std:: cout << "max" << "_t" << max_x << "\t" << max_y << "\t" << max_z << std::endl;
    std:: cout << "min" << "_t" << min_x << "\t" << min_y << "\t" << min_z << std::endl;*/

    // depression
    /*for (VEC3& p : position_)
    {

        p[2] -= 7.4518;
        p[2] *= 100.;
    }*/

    dart_level_ = map_->add_attribute<uint8, CMap2::CDart::ORBIT>("dart_level");
    face_subd_id_ = map_->add_attribute<uint32, CMap2::Face::ORBIT>("face_subdivision_id");
    tri_face_ = map_->add_attribute<bool, CMap2::Face::ORBIT>("tri_face");
    phi_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("phi");
    zb_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("zb");
    h_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("h");
    q_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("q");
    r_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("r");
    centroid_ = map_->add_attribute<VEC3, CMap2::Face::ORBIT>("centroid");
    area_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("area");
    swept_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("swept");
    discharge_ = map_->add_attribute<SCALAR, CMap2::Face::ORBIT>("discharge");

    f1_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f1");
    f2_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f2");
    f3_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("f3");
    s2L_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s2L");
    s2R_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("s2R");
    normX_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("normX");
    normY_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("normY");
    length_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("length");

    scalar_value_h_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_h");
    scalar_value_u_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_u");
    scalar_value_v_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_v");
    water_position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("water_position");
    flow_velocity_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("flow_velocity");

    map2_->copy_attribute(water_position_, position_);

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

    cgogn::geometry::compute_centroid<VEC3, CMap2::Face>(*map2_, position_, centroid_);
    cgogn::geometry::compute_area<VEC3, CMap2::Face>(*map2_, position_, area_);

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
    system("rm -rf graphiques/plot*");

    t_ = 0.;
    dt_ = 0.001;
    dt_max_ = 1.;
    t_max_ = 110.;
    nbr_iter_ = 0;

    solver_ = 2;
    assert(solver_ == 2 || solver_ == 4);
    /*
     * solver_ = 2 : solveur HLLC
     * solver_ = 4 : solveur PorAS
     */

    geometry_ = 1;
    assert(geometry_ == 0 || geometry_ == 1 || geometry_ == 2 || geometry_ == 3 || geometry_ == 4);
    /*
     * geometry_ = 0 : dambreak
     * geometry_ = 1 : square grid
     * geometry_ = 2 : dambreak coude
     * geometry_ = 3 : depression
     * geometry_ = 4 : triangular grid
     */

    hmin_ = 1e-3; // Valeur minimale du niveau d'eau pour laquelle une maille est considérée comme non vide
    small_ = 1e-35; // Valeur minimale en deça de laquelle les valeurs sont considérées comme nulles
    v_max_ = 10.;
    Fr_max_ = 5.;
    alphaK_ = 0.;
    kx_ = 33.33;

    friction_ = 0;
    /*
     * friction_ = 0 : no friction
     * friction_ = 1 : friction
     */

    map2_->parallel_foreach_cell(
        [&] (CMap2::Face f)
        {
            phi_[f] = 1.;
            q_[f] = 0.;
            r_[f] = 0.;

            // ------------- dambreak
            if(geometry_ == 0)
            {
                zb_[f] = 0.;
                if(centroid_[f][1] > 65)
                    h_[f] = 10;
                else
                    h_[f] = 5.;
            }
            // ============= dambreak

            // ------------- square grid
            else if(geometry_ == 1)
            {
                zb_[f] == 0.;
                if(centroid_[f][1] > 0.)
                    h_[f] = 10;
                else
                    h_[f] = 5.;
            }
            // ============= square grid

            // -------------  dambreak coude
            else if(geometry_ == 2)
            {
                if(centroid_[f][0] < 2.39)
                {
                    zb_[f] = 0.;
                    h_[f] = 0.58;
                }
                else
                {
                    zb_[f] = 0.33;
                    h_[f] = 0.;
                }
            }
            // =============  dambreak coude

            // ------------- depression
            else if(geometry_ == 3)
            {
                zb_[f] = 0.;
                int nbv = 0.;
                map2_->foreach_incident_vertex(f, [&] (CMap2::Vertex v)
                {
                    zb_[f] += position_[v][2];
                    nbv++;
                });
                zb_[f] /= nbv;
                h_[f] = 1.;
            }
            // ============= depression

            // -------------  triangular grid
            else if(geometry_ == 4)
            {
                zb_[f] == 0.;
                if(centroid_[f][1] > 86.)
                    h_[f] = 10;
                else
                    h_[f] = 6.;
            }
            // =============  triangular grid
        },
        *qtrav_
    );

    map2_->parallel_foreach_cell(
        [&] (CMap2::Vertex v)
        {
            SCALAR h = 0;
            SCALAR q = 0;
            SCALAR r = 0;
            uint32 nbf = 0;
            map2_->foreach_incident_face(v, [&] (CMap2::Face f)
            {
                //h += h_[f];
                h += (h_[f] + zb_[f]);
                q += q_[f];
                r += r_[f];
                ++nbf;
            });
            h /= nbf;
            q /= nbf;
            r /= nbf;
            scalar_value_h_[v] = h;
            scalar_value_u_[v] = q/h;
            scalar_value_v_[v] = r/h;
            water_position_[v][2] = h;
            flow_velocity_[v][0] = q/h;
            flow_velocity_[v][1] = r/h;
            flow_velocity_[v][2] = 0.;
        },
        *qtrav_
    );

    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_h");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_u");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_v");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "flow_velocity");
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
    {
        draw_timer_->stop();
    }

    map_->lock_topo_access();
    simu_data_access_.lock();

    // update draw data from simu data
    map2_->parallel_foreach_cell(
        [&] (CMap2::Vertex v)
        {
            SCALAR h = 0;
            SCALAR q = 0;
            SCALAR r = 0;
            uint32 nbf = 0;
            map2_->foreach_incident_face(v, [&] (CMap2::Face f)
            {
                //h += h_[f];
                h += (h_[f] + zb_[f]);
                q += q_[f];
                r += r_[f];
                ++nbf;
            });
            h /= nbf;
            q /= nbf;
            r /= nbf;
            scalar_value_h_[v] = h;
            scalar_value_u_[v] = q/h;
            scalar_value_v_[v] = r/h;
            water_position_[v][2] = h;
            flow_velocity_[v][0] = q/h;
            flow_velocity_[v][1] = r/h;
        },
        *qtrav_
    );

    simu_data_access_.unlock();

    map_->notify_connectivity_change();
    map_->init_primitives(cgogn::rendering::POINTS);
    map_->init_primitives(cgogn::rendering::LINES);
    map_->init_primitives(cgogn::rendering::TRIANGLES);
    map_->init_primitives(cgogn::rendering::BOUNDARY);

    map_->unlock_topo_access();

    // notify attribute changes
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_h");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_u");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_v");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
    map_->notify_attribute_change(CMap2::Vertex::ORBIT, "flow_velocity");
}


void Plugin_ShallowWater::update_time_step()
{
    map2_->parallel_foreach_cell(
        [&] (CMap2::Face f)
        {
            swept_[f] = 0.;
            discharge_[f] = 0.;
        },
        *qtrav_
    );

    map2_->foreach_cell(
        [&] (CMap2::Edge e)
        {
            if(map2_->is_incident_to_boundary(e))
            {
                CMap2::Face f(e.dart);
                CMap2::Vertex v(e.dart);
                if(geometry_ == 0)
                {
                    if((position_[v][0] == 0 && position_[v][1] == 0) ||
                            (position_[v][0] == 100 && position_[v][1] == 0) ||
                            (position_[v][0] == 100 && position_[v][1] == 60) ||
                            (position_[v][0] == 90 && position_[v][1] == 60) ||
                            (position_[v][0] == 90 && position_[v][1] == 65) ||
                            (position_[v][0] == 100 && position_[v][1] == 65) ||
                            (position_[v][0] == 100 && position_[v][1] == 100) ||
                            (position_[v][0] == 0 && position_[v][1] == 100) ||
                            (position_[v][0] == 0 && position_[v][1] == 65) ||
                            (position_[v][0] == 60 && position_[v][1] == 65) ||
                            (position_[v][0] == 60 && position_[v][1] == 60) ||
                            (position_[v][0] == 0 && position_[v][1] == 60))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if((position_[v][0] > 90 && position_[v][1] == 60) ||
                            (position_[v][1] == 100) ||
                            (position_[v][0] == 0 && position_[v][1] > 65) ||
                            (position_[v][0] == 60 && position_[v][1] > 60 && position_[v][1] < 65) ||
                            (position_[v][0] < 60 && position_[v][1] == 60) ||
                            (position_[v][0] == 0 && position_[v][1] < 60)
                            ) //  left border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] -= length_[e]*f1_[e];
                    }
                    else // right border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] += length_[e]*f1_[e];
                    }
                }
                // =============== dambreak

                // ------------------- square grid
                else if(geometry_ == 1)
                {
                    if((position_[v][0] == -100 && position_[v][1] == -100) ||
                            (position_[v][0] == -100 && position_[v][1] == 100) ||
                            (position_[v][0] == 100 && position_[v][1] == -100) ||
                            (position_[v][0] == 100 && position_[v][1] == 100))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if(position_[v][1] == 100. || position_[v][0] == -100.) // left border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] -= length_[e]*f1_[e];
                    }
                    else // right border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] += length_[e]*f1_[e];
                    }
                }
                // ================ square grid

                // -------------------- dambreak coude
                else if(geometry_ == 2)
                {
                    if((position_[v][0] == 0 && position_[v][1] == 0) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 0) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 0.445) ||
                            (position_[v][0] == 6.805 && position_[v][1] == 0.445) ||
                            (position_[v][0] == 6.805 && position_[v][1] == 3.86) ||
                            (position_[v][0] == 6.31 && position_[v][1] == 3.86) ||
                            (position_[v][0] == 6.31 && position_[v][1] == 0.94) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 0.94) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 2.44) ||
                            (position_[v][0] == 0 && position_[v][1] == 2.44))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if((position_[v][0] > 6.31 && position_[v][1] == 3.86) ||
                            (position_[v][0] == 6.31 && position_[v][1] > 0.94) ||
                            (position_[v][0] > 2.39 && position_[v][0] < 6.31 && position_[v][1] == 0.94) ||
                            (position_[v][0] < 2.39 && position_[v][1] == 2.44) ||
                            (position_[v][0] == 0 && position_[v][1] < 2.44)
                            )// left border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] -= length_[e]*f1_[e];
                    }
                    else // right border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] += length_[e]*f1_[e];
                    }
                }
                // ===================== dambreak coude

                // -------------------- depression
                else if(geometry_ == 3)
                {
                    if((position_[v][0] == 0 && position_[v][1] == 0 ) ||
                            (position_[v][0] == 0 && position_[v][1] == 2000 ) ||
                            (position_[v][0] == 2000 && position_[v][1] == 0) ||
                            (position_[v][0] == 2000 && position_[v][1] == 2000))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if(position_[v][1] == 2000 || position_[v][0] == 0) // left border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] -= length_[e]*f1_[e];
                    }
                    else // right border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] += length_[e]*f1_[e];
                    }
                }
                // ======================== depression

                // ------------------- triangular grid
                else if(geometry_ == 4)
                {
                    if((position_[v][0] ==  0 && position_[v][1] == 0) ||
                            (position_[v][0] == 0 && position_[v][1] > 173) ||
                            (position_[v][0] == 200 && position_[v][1] == 0) ||
                            (position_[v][0] == 300 && position_[v][1] > 173))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if(position_[v][0] <= 100. || position_[v][1] == 100.) // left border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] -= length_[e]*f1_[e];
                    }
                    else // right border
                    {
                        SCALAR lambda = 0.;
                        if(h_[f] > hmin_)
                            lambda = fabs(q_[f]*normX_[e] + r_[f]*normY_[e]) / max_0(h_[f], hmin_) + sqrt(9.81*h_[f]);
                        swept_[f] += lambda*length_[e];
                        discharge_[f] += length_[e]*f1_[e];
                    }
                }
                // ================ triangular grid
            }

            else
            {
                CMap2::Face fL, fR;
                get_LR_faces(e,fL,fR);
                SCALAR lambda = 0.;
                if(h_[fL] > hmin_)
                    lambda = fabs(q_[fL]*normX_[e] + r_[fL]*normY_[e]) / max_0(h_[fL], hmin_) + sqrt(9.81*h_[fL]);
                swept_[fL] += lambda*length_[e];
                lambda = 0.;
                if(h_[fR] > hmin_)
                    lambda = fabs(q_[fR]*normX_[e] + r_[fR]*normY_[e]) / max_0(h_[fR], hmin_) + sqrt(9.81*h_[fR]);
                swept_[fR] += lambda*length_[e];
                discharge_[fL] -= length_[e]*f1_[e];
                discharge_[fR] += length_[e]*f1_[e];
            }
        },
        *qtrav_
    );

    std::vector<SCALAR> min_dt_per_thread(cgogn::thread_pool()->nb_workers());
    for(SCALAR& d : min_dt_per_thread) d = min_0(dt_max_, t_max_-t_); // Timestep for ending simulation
    map2_->parallel_foreach_cell(
        [&] (CMap2::Face f)
        {
            uint32 idx = cgogn::current_thread_index();
            // Ensure CFL condition
            if(area_[f]/max_0(swept_[f], small_) < min_dt_per_thread[idx])
                min_dt_per_thread[idx] = area_[f]/max_0(swept_[f], small_);
            // Ensure overdry condition
            if(area_[f]*phi_[f]*(h_[f]+zb_[f]) < (-discharge_[f]*min_dt_per_thread[idx]))
                min_dt_per_thread[idx] = - area_[f]*phi_[f]*(h_[f]+zb_[f]) / discharge_[f];
        },
        *qtrav_
    );
    dt_ = *(std::min_element(min_dt_per_thread.begin(), min_dt_per_thread.end()));
}

void Plugin_ShallowWater::execute_time_step()
{
    //auto start = std::chrono::high_resolution_clock::now();

//    std::cout << "execute time step" << std::endl;

    map2_->parallel_foreach_cell(
        [&] (CMap2::Edge e)
        {
        // calcul des composantes du vecteur normal à l'interface
            CMap2::Vertex v1(e.dart);
            CMap2::Vertex v2(map2_->phi1(e.dart));
            CMap2::Vertex vB, vH;
            if(position_[v2][1] < position_[v1][1] )
            {
                vB = v2;
                vH = v1;
            }
            else if(position_[v2][1] == position_[v1][1])
            {
                if(position_[v2][0] <= position_[v1][0])
                {
                    vB = v2;
                    vH = v1;
                }
                else
                {
                    vB = v1;
                    vH = v2;
                }
            }
            else
            {
                vB = v1;
                vH = v2;
            }

            length_[e] = cgogn::geometry::length<VEC3>(*map2_, e, position_);
            SCALAR v2x = position_[vH][0] - position_[vB][0];
            SCALAR v2y = position_[vH][1] - position_[vB][1];
            normX_[e] = v2y/length_[e];
            normY_[e] = -v2x/length_[e];
        },
        *qtrav_
    );
//    std::cout << "normale" << std::endl;

    map2_->parallel_foreach_cell(
        [&] (CMap2::Edge e)
        {
            // solve flux on edge
            // border conditions
            Str_Riemann_Flux riemann_flux;
            f1_[e] = 0.;
            f2_[e] = 0.;
            f3_[e] = 0.;
            s2L_[e] = 0.;
            s2R_[e] = 0.;
            if (map2_->is_incident_to_boundary(e))
            {
                CMap2::Vertex v(e.dart);
                CMap2::Face f(e.dart);

                /*
                     * border_condition_choice = 0 : Free Outflow
                     * border_condition_choice = 1 : Critical Section
                     * border_condition_choice = 2 : Prescribed h
                     * border_condition_choice = 3 : Prescribed z
                     * border_condition_choice = 4 : Prescribed q
                     * border_condition_choice = 5 : Weir
                     */

                if(phi_[f] > small_)
                {
                    // ------------------- dambreak
                    if(geometry_ == 0)
                    {
                        if((position_[v][0] == 0 && position_[v][1] == 0) ||
                                (position_[v][0] == 100 && position_[v][1] == 0) ||
                                (position_[v][0] == 100 && position_[v][1] == 60) ||
                                (position_[v][0] == 90 && position_[v][1] == 60) ||
                                (position_[v][0] == 90 && position_[v][1] == 65) ||
                                (position_[v][0] == 100 && position_[v][1] == 65) ||
                                (position_[v][0] == 100 && position_[v][1] == 100) ||
                                (position_[v][0] == 0 && position_[v][1] == 100) ||
                                (position_[v][0] == 0 && position_[v][1] == 65) ||
                                (position_[v][0] == 60 && position_[v][1] == 65) ||
                                (position_[v][0] == 60 && position_[v][1] == 60) ||
                                (position_[v][0] == 0 && position_[v][1] == 60))
                        {
                            // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                            CMap2::Vertex v_aux(map2_->phi1(e.dart));
                            v = v_aux;
                        }

                        if(position_[v][1] == 0)
                            riemann_flux = border_condition(2,5.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 100 && position_[v][1] < 60)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] > 90 && position_[v][1] == 60)
                            riemann_flux = border_condition(2,5.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 90 && position_[v][1] > 60 && position_[v][1] < 65)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] > 90 && position_[v][1] == 65)
                            riemann_flux = border_condition(2,10.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 100 && position_[v][1] > 65)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][1] == 100)
                            riemann_flux = border_condition(2,10.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 0 && position_[v][1] > 65)
                            riemann_flux = border_condition(4,0.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] < 60 && position_[v][1] == 65)
                            riemann_flux = border_condition(2,10.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 60 && position_[v][1] > 60 && position_[v][1] < 65)
                            riemann_flux = border_condition(4,0.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] < 60 && position_[v][1] == 60)
                            riemann_flux = border_condition(2,5.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 0 && position_[v][1] < 60)
                            riemann_flux = border_condition(4,0.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);

                    }
                    // =================== parital dambreak

                    // ------------------- square grid
                    else if(geometry_ == 1)
                    {
                        if((position_[v][0] == -100 && position_[v][1] == -100) ||
                                (position_[v][0] == -100 && position_[v][1] == 100) ||
                                (position_[v][0] == 100 && position_[v][1] == -100) ||
                                (position_[v][0] == 100 && position_[v][1] == 100))
                        {
                            // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                            CMap2::Vertex v_aux(map2_->phi1(e.dart));
                            v = v_aux;
                        }
                        if(position_[v][0] == -100)
                            riemann_flux = border_condition(4,0.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][1] == -100)
                            riemann_flux = border_condition(2,5.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 100)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][1] == 100)
                            riemann_flux = border_condition(2,10.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                    }
                    // =================== square grid

                    // ------------------- dambreak coude
                    else if(geometry_ == 2)
                    {
                        if((position_[v][0] == 0 && position_[v][1] == 0) ||
                                (position_[v][0] == 2.39 && position_[v][1] == 0) ||
                                (position_[v][0] == 2.39 && position_[v][1] == 0.445) ||
                                (position_[v][0] == 6.805 && position_[v][1] == 0.445) ||
                                (position_[v][0] == 6.805 && position_[v][1] == 3.86) ||
                                (position_[v][0] == 6.31 && position_[v][1] == 3.86) ||
                                (position_[v][0] == 6.31 && position_[v][1] == 0.94) ||
                                (position_[v][0] == 2.39 && position_[v][1] == 0.94) ||
                                (position_[v][0] == 2.39 && position_[v][1] == 2.44) ||
                                (position_[v][0] == 0 && position_[v][1] == 2.44))
                        {
                            // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                            CMap2::Vertex v_aux(map2_->phi1(e.dart));
                            v = v_aux;
                        }
                        if(position_[v][0] < 2.39 && position_[v][1] == 0)
                            riemann_flux = border_condition(2,0.58,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 2.39 && position_[v][1] < 0.445)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] > 2.39 && position_[v][1] == 0.445)
                            riemann_flux = border_condition(5,2.18,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]); // ????????
                        else if(position_[v][0] == 6.805 && position_[v][1] > 0.0445)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] > 6.31 && position_[v][1] == 3.86)
                            riemann_flux = border_condition(0,2.18,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 6.31 && position_[v][1] > 0.94)
                            riemann_flux = border_condition(4,0.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] > 2.39 && position_[v][0] < 6.31 && position_[v][1] == 0.94)
                            riemann_flux = border_condition(5,2.18,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]); // ????????
                        else if(position_[v][0] == 2.39 && position_[v][1] > 0.94)
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] < 2.39 && position_[v][1] == 2.44)
                            riemann_flux = border_condition(2,0.58,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 0 && position_[v][1] < 2.44)
                            riemann_flux = border_condition(2,0.58,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                    }
                    // =================== dambreak coude

                    // ------------------- depression
                    else if(geometry_ == 3)
                    {
                        if((position_[v][0] == 0 && position_[v][1] == 0 ) ||
                                (position_[v][0] == 0 && position_[v][1] == 2000 ) ||
                                (position_[v][0] == 2000 && position_[v][1] == 0) ||
                                (position_[v][0] == 2000 && position_[v][1] == 2000))
                        {
                            // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                            CMap2::Vertex v_aux(map2_->phi1(e.dart));
                            v = v_aux;
                        }
                        SCALAR q;
                        if(t_ >= 600. && t_ <= 5160)
                            q = 0.2;
                        else
                            q = 0.;
                        if(position_[v][1] == 0 || position_[v][0] == 2000)
                            riemann_flux = border_condition(4,q,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] == 0 || position_[v][1] == 2000)
                            riemann_flux = border_condition(4,q,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                    }
                    // =================== depression

                    // ------------------- triangular grid
                    else if(geometry_ == 4)
                    {
                        if((position_[v][0] ==  0 && position_[v][1] == 0) ||
                                (position_[v][0] == 0 && position_[v][1] > 173) ||
                                (position_[v][0] == 200 && position_[v][1] == 0) ||
                                (position_[v][0] == 300 && position_[v][1] > 173))
                        {
                            // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                            CMap2::Vertex v_aux(map2_->phi1(e.dart));
                            v = v_aux;
                        }
                        if(position_[v][1] == 0)
                            riemann_flux = border_condition(2,6.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][1] > 173)
                            riemann_flux = border_condition(2,10.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else if(position_[v][0] <= 100 )
                            riemann_flux = border_condition(4,0.,false, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                        else
                            riemann_flux = border_condition(4,0.,true, normX_[e], normY_[e], q_[f], r_[f], h_[f]+zb_[f], zb_[f]);
                    }
                    // =================== triangular grid
                } // phi_[f] > small_
            } // maps2_->is_incident_to_boundary(e)



            else //Inner cell:use the lateralised Riemann solver
            {
                CMap2::Face fL,fR;
                get_LR_faces(e,fL,fR);

                if((area_[fL] == 0 || area_[fR] == 0) && simu_running_)
                {
                    std::cout << "stop car aire nulle" << std::endl;
                    simu_running_ = false;
                }

                SCALAR phiL = phi_[fL];
                SCALAR phiR = phi_[fR];
                SCALAR zbL = zb_[fL];
                SCALAR zbR = zb_[fR];
                if(h_[fL] > hmin_ || h_[fR] > hmin_)
                {
                    SCALAR hL = h_[fL];
                    SCALAR hR = h_[fR];
                    SCALAR qL = q_[fL]*normX_[e] + r_[fL]*normY_[e];
                    SCALAR qR = q_[fR]*normX_[e] + r_[fR]*normY_[e];
                    SCALAR rL = -q_[fL]*normY_[e] + r_[fL]*normX_[e];
                    SCALAR rR = -q_[fR]*normY_[e] + r_[fR]*normX_[e];

                    if(solver_ == 2)
                        riemann_flux = Solv_HLLC(9.81,hmin_,small_, zbL, zbR, phiL, phiR, hL, qL, rL, hR, qR, rR);
                    else if(solver_ == 4)
                        riemann_flux = Solv_PorAS(9.81,hmin_,small_, zbL, zbR, phiL, phiR, hL, qL, rL, hR, qR, rR);
                }
            }
            f1_[e] = riemann_flux.F1;
            f2_[e] = riemann_flux.F2;
            f3_[e] = riemann_flux.F3;
            s2L_[e] = riemann_flux.s2L;
            s2R_[e] = riemann_flux.s2R;
        },
        *qtrav_
    );
//    std::cout << "flux" << std::endl;

    update_time_step();
//    std::cout << "update time step" << std::endl;

    map2_->foreach_cell(
        [&] (CMap2::Edge e)
        {
            // balance
            SCALAR fact = dt_*length_[e];
            if(map2_->is_incident_to_boundary(e))
                // border conditions
            {
                CMap2::Face f(e.dart);
                CMap2::Vertex v(e.dart);

                // ------------ dambreak
                if(geometry_ == 0)
                {
                    if((position_[v][0] == 0 && position_[v][1] == 0) ||
                            (position_[v][0] == 100 && position_[v][1] == 0) ||
                            (position_[v][0] == 100 && position_[v][1] == 60) ||
                            (position_[v][0] == 90 && position_[v][1] == 60) ||
                            (position_[v][0] == 90 && position_[v][1] == 65) ||
                            (position_[v][0] == 100 && position_[v][1] == 65) ||
                            (position_[v][0] == 100 && position_[v][1] == 100) ||
                            (position_[v][0] == 0 && position_[v][1] == 100) ||
                            (position_[v][0] == 0 && position_[v][1] == 65) ||
                            (position_[v][0] == 60 && position_[v][1] == 65) ||
                            (position_[v][0] == 60 && position_[v][1] == 60) ||
                            (position_[v][0] == 0 && position_[v][1] == 60))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if((position_[v][0] > 90 && position_[v][1] == 60) ||
                            (position_[v][1] == 100) ||
                            (position_[v][0] == 0 && position_[v][1] > 65) ||
                            (position_[v][0] == 60 && position_[v][1] > 60 && position_[v][1] < 65) ||
                            (position_[v][0] < 60 && position_[v][1] == 60) ||
                            (position_[v][0] == 0 && position_[v][1] < 60)
                            ) //  left border
                    {
                        SCALAR factR = fact/area_[f];
                        h_[f] = h_[f] + factR*f1_[e];
                        q_[f] = q_[f] + factR*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] + factR*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                    else // right border
                    {
                        SCALAR factL = fact/area_[f];
                        h_[f] = h_[f] - factL*f1_[e];
                        q_[f] = q_[f] - factL*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] - factL*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                }
                // =============== dambreak

                // ------------------- square grid
                else if(geometry_ == 1)
                {
                    if((position_[v][0] == -100 && position_[v][1] == -100) ||
                            (position_[v][0] == -100 && position_[v][1] == 100) ||
                            (position_[v][0] == 100 && position_[v][1] == -100) ||
                            (position_[v][0] == 100 && position_[v][1] == 100))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if(position_[v][1] == 100. || position_[v][0] == -100.) // left border
                    {
                        SCALAR factR = fact/area_[f];
                        h_[f] = h_[f] + factR*f1_[e];
                        q_[f] = q_[f] + factR*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] + factR*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                    else // right border
                    {
                        SCALAR factL = fact/area_[f];
                        h_[f] = h_[f] - factL*f1_[e];
                        q_[f] = q_[f] - factL*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] - factL*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                }
                // ================ square grid

                // -------------------- dambreak coude
                else if(geometry_ == 2)
                {
                    if((position_[v][0] == 0 && position_[v][1] == 0) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 0) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 0.445) ||
                            (position_[v][0] == 6.805 && position_[v][1] == 0.445) ||
                            (position_[v][0] == 6.805 && position_[v][1] == 3.86) ||
                            (position_[v][0] == 6.31 && position_[v][1] == 3.86) ||
                            (position_[v][0] == 6.31 && position_[v][1] == 0.94) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 0.94) ||
                            (position_[v][0] == 2.39 && position_[v][1] == 2.44) ||
                            (position_[v][0] == 0 && position_[v][1] == 2.44))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if((position_[v][0] > 6.31 && position_[v][1] == 3.86) ||
                            (position_[v][0] == 6.31 && position_[v][1] > 0.94) ||
                            (position_[v][0] > 2.39 && position_[v][0] < 6.31 && position_[v][1] == 0.94) ||
                            (position_[v][0] < 2.39 && position_[v][1] == 2.44) ||
                            (position_[v][0] == 0 && position_[v][1] < 2.44)
                            )// left border
                    {
                        SCALAR factR = fact/area_[f];
                        h_[f] = h_[f] + factR*f1_[e];
                        q_[f] = q_[f] + factR*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] + factR*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                    else // right border
                    {
                        SCALAR factL = fact/area_[f];
                        h_[f] = h_[f] - factL*f1_[e];
                        q_[f] = q_[f] - factL*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] - factL*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                }
                // ===================== dambreak coude

                // -------------------- depression
                else if(geometry_ == 3)
                {
                    if((position_[v][0] == 0 && position_[v][1] == 0 ) ||
                            (position_[v][0] == 0 && position_[v][1] == 2000 ) ||
                            (position_[v][0] == 2000 && position_[v][1] == 0) ||
                            (position_[v][0] == 2000 && position_[v][1] == 2000))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if(position_[v][1] == 2000 || position_[v][0] == 0) // left border
                    {
                        SCALAR factR = fact/area_[f];
                        h_[f] = h_[f] + factR*f1_[e];
                        q_[f] = q_[f] + factR*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] + factR*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                    else // right border
                    {
                        SCALAR factL = fact/area_[f];
                        h_[f] = h_[f] - factL*f1_[e];
                        q_[f] = q_[f] - factL*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] - factL*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                }
                // ======================== depression

                // ------------------- triangular grid
                else if(geometry_ == 4)
                {
                    if((position_[v][0] ==  0 && position_[v][1] == 0) ||
                            (position_[v][0] == 0 && position_[v][1] > 173) ||
                            (position_[v][0] == 200 && position_[v][1] == 0) ||
                            (position_[v][0] == 300 && position_[v][1] > 173))
                    {
                        // si le noeud associé à l'arrête est dans le coin, alors on prend l'autre noeud
                        CMap2::Vertex v_aux(map2_->phi1(e.dart));
                        v = v_aux;
                    }
                    if(position_[v][0] <= 100. || position_[v][1] == 100.) // left border
                    {
                        SCALAR factR = fact/area_[f];
                        h_[f] = h_[f] + factR*f1_[e];
                        q_[f] = q_[f] + factR*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] + factR*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                    else // right border
                    {
                        SCALAR factL = fact/area_[f];
                        h_[f] = h_[f] - factL*f1_[e];
                        q_[f] = q_[f] - factL*(f2_[e]*normX_[e] - f3_[e]*normY_[e]);
                        r_[f] = r_[f] - factL*(f3_[e]*normX_[e] + f2_[e]*normY_[e]);
                    }
                }
                // ================ triangular grid
            } // map2_->is_incident_to_boundary(e)

            else // inner cell
            {
                CMap2::Face fL, fR;
                get_LR_faces(e,fL,fR);
                SCALAR factL = 0.;
                SCALAR factR = 0.;
                if(phi_[fL] > small_)
                    factL = fact/area_[fL]*phi_[fL];
                if(phi_[fR] > small_)
                    factR = fact/area_[fR]*phi_[fR];
                h_[fL] = h_[fL] - factL*f1_[e];
                h_[fR] = h_[fR] + factR*f1_[e];
                q_[fL] = q_[fL] + factL * ((-f2_[e] + s2L_[e])*normX_[e] + f3_[e]*normY_[e]);
                q_[fR] = q_[fR] + factR * (( f2_[e] + s2R_[e])*normX_[e] - f3_[e]*normY_[e]);
                r_[fL] = r_[fL] + factL*(-f3_[e]*normX_[e] + (-f2_[e]+s2L_[e])*normY_[e]);
                r_[fR] = r_[fR] + factR*( f3_[e]*normX_[e] + ( f2_[e]+s2R_[e])*normY_[e]);
            }

        },
        *qtrav_
    );
//    std::cout << "balance" << std::endl;

    map2_->parallel_foreach_cell(
        [&] (CMap2::Face f)
        {
            if(area_[f] == 0)
                return;

            // friction
            if(friction_ != 0)
            {
                SCALAR qx = q_[f]*cos(alphaK_) + r_[f]*sin(alphaK_);
                SCALAR qy = - q_[f]*sin(alphaK_) + r_[f]*cos(alphaK_);
                if(h_[f] > hmin_)
                {
                    qx = qx * exp(-(9.81 * sqrt(qx*qx+qy*qy) / (max_0(kx_*kx_,small_*small_) * pow(h_[f],7./3.))) * dt_);
                    qy = qy * exp(-(9.81 * sqrt(qx*qx+qy*qy) / (max_0(kx_*kx_,small_*small_) * pow(h_[f],7./3.))) * dt_);
                }
                else
                {
                    qx = 0.;
                    qy = 0.;
                }
                q_[f] = qx*cos(alphaK_) - qy*sin(alphaK_);
                r_[f] = qx*sin(alphaK_) + qy*cos(alphaK_);
            }

            // optional correction
            // Negative water depth
            if(h_[f] < 0.)
            {
                h_[f] = 0.;
                q_[f] = 0.;
                r_[f] = 0.;
            }
            // Abnormal large velocity => Correction of q and r to respect Vmax and Frmax
            if(h_[f] > hmin_)
            {
                SCALAR v = sqrt(q_[f]*q_[f]+r_[f]*r_[f])/max_0(h_[f],small_);
                SCALAR c = sqrt(9.81*max_0(h_[f],small_));
                SCALAR Fr = v/c;
                SCALAR Fact = max_1(1e0, v/v_max_, Fr/Fr_max_);
                q_[f] /= Fact;
                r_[f] /= Fact;
            }
            // Quasi-zero
            else
            {
                q_[f] = 0.;
                r_[f] = 0.;
            }
        },
        *qtrav_
    );
//    std::cout << "optional corrections" << std::endl;

    map_->lock_topo_access();
    //try_simplification();
    std::cout << "simplification" << std::endl;
    try_subdivision();
    std::cout << "subdivision" << std::endl;
    map_->unlock_topo_access();

    t_ += dt_;

    nbr_iter_++;
//    std::cout << nbr_iter_ << std::endl;

     /*std::ofstream file ("graphiques/plot" + std::to_string(nbr_iter_) + ".txt");
    assert(!file.fail());
    map2_->foreach_cell(
        [&] (CMap2::Face f)
        {
            if(area_[f] == 0)
                return;

            file << centroid_[f][0] << "\t" << centroid_[f][1] << "\t" << h_[f] << "\t" << q_[f] << "\t" << r_[f] << std::endl;
        },
        *qtrav_
    );
    file.close();*/

    /*        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::nanoseconds sleep_duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<SCALAR>(dt_))
            - std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

        if (sleep_duration > std::chrono::nanoseconds::zero())
            std::this_thread::sleep_for(sleep_duration);*/

    std::cout << std::endl;
}

void Plugin_ShallowWater::try_subdivision()
{
    CMap2::CellMarker<CMap2::Face::ORBIT> subdivided(*map2_);

    map2_->foreach_cell(
        [&] (CMap2::Face f)
        {
            if (subdivided.is_marked(f))
                return;

            if (face_level(f) > 1)
                return;

            std::vector<SCALAR> diff_h;
            std::vector<SCALAR> diff_q;
            std::vector<SCALAR> diff_r;
            std::vector<SCALAR> diff_zb;
            map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
            {
                diff_h.push_back(fabs(h_[f]-h_[af]));
                diff_q.push_back(fabs(q_[f]-q_[af]));
                diff_r.push_back(fabs(r_[f]-r_[af]));
                diff_zb.push_back(fabs(zb_[f]-zb_[af]));
            });
            SCALAR max_diff_h = 0.;
            SCALAR max_diff_q = 0.;
            SCALAR max_diff_r = 0.;
            SCALAR max_diff_zb = 0.;
            for(int i = 0; i < diff_h.size(); i++)
            {
                max_diff_h = std::max(max_diff_h,diff_h[i]);
                max_diff_q = std::max(max_diff_q,diff_q[i]);
                max_diff_r = std::max(max_diff_r,diff_r[i]);
                max_diff_zb = std::max(max_diff_zb,diff_zb[i]);
            }

            //if ( /* a certain condition is met */ true)
            if(max_diff_h > 2.01914/50. || max_diff_q > 8.44341/50. || max_diff_r > 8.44341/50. || max_diff_zb > 0)
                subdivide_face(f, subdivided);
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

            std::vector<SCALAR> diff_h;
            std::vector<SCALAR> diff_q;
            std::vector<SCALAR> diff_r;
            map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
            {
                diff_h.push_back(fabs(h_[f]-h_[af]));
                diff_q.push_back(fabs(q_[f]-q_[af]));
                diff_r.push_back(fabs(r_[f]-r_[af]));
            });
            SCALAR max_diff_h = 0.;
            SCALAR max_diff_q = 0.;
            SCALAR max_diff_r = 0.;
            for(int i = 0; i < diff_h.size(); i++)
            {
                max_diff_h = std::max(max_diff_h,diff_h[i]);
                max_diff_q = std::max(max_diff_q,diff_q[i]);
                max_diff_r = std::max(max_diff_r,diff_r[i]);
            }

            //if ( /* a certain condition is met */ true)
            if(max_diff_h < 0.7 && max_diff_q < 0.1 && max_diff_r < 0.1)
                simplify_face(f);
        },
        *qtrav_
    );
}

void Plugin_ShallowWater::subdivide_face(CMap2::Face f, CMap2::CellMarker<CMap2::Face::ORBIT>& subdivided)
{
    f.dart = oldest_dart(f);
    uint8 fl = face_level(f);
    uint8 fid = face_subd_id_[f];
    SCALAR old_h = h_[f];
    SCALAR old_q = q_[f];
    SCALAR old_r = r_[f];
    SCALAR old_phi = phi_[f];
    VEC3 old_centroid = centroid_[f];
    uint8 dl = dart_level_[f.dart];

    // check neighbours level
    map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
    {
        if (face_level(af) < fl)
            subdivide_face(af, subdivided);
    });
    std::cout << "check neighbours level" << std::endl;

    // cut edges (if not already done)
    // the new vertex is the center of the edge
    cgogn::Dart it = f.dart;
    do
    {
        cgogn::Dart next = map2_->phi1(it);
        if (dart_level_[next] > fl) ////////////////
        {
            next = map2_->phi1(next);
        }
        else
        {
            CMap2::Vertex v1(it);
            CMap2::Vertex v2(map2_->phi1(it));
            CMap2::Vertex v = map2_->cut_edge(CMap2::Edge(it));
            position_[v] = (position_[v1] + position_[v2])/2.;
            if(map2_->is_incident_to_boundary(CMap2::Edge(it)))
            {
                dart_level_[map2_->phi1(it)] = fl+1;
            }
            else
            {
                dart_level_[map2_->phi1(it)] = fl+1;
                dart_level_[map2_->phi2(it)] = fl+1;
            }
            qtrav_->update(v);
            qtrav_->update(CMap2::Edge(it));
            qtrav_->update(CMap2::Edge(map2_->phi1(it)));
        }
        it = next;
    } while (it != f.dart);
    std::cout << "cut edge" << std::endl;

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
        h_[CMap2::Face(it)] = old_h;
        q_[CMap2::Face(it)] = old_q;
        r_[CMap2::Face(it)] = old_r;
        phi_[CMap2::Face(it)] = old_phi;
        zb_[CMap2::Face(it)] = 0.;
        int nbv = 0.;
        map2_->foreach_incident_vertex(CMap2::Face(it), [&] (CMap2::Vertex v)
        {
            zb_[CMap2::Face(it)] += position_[v][2];
            nbv++;
        });
        zb_[CMap2::Face(it)] /= nbv;
        centroid_[CMap2::Face(it)] = cgogn::geometry::centroid<VEC3>(*map2_, CMap2::Face(it), position_);
        area_[CMap2::Face(it)] = cgogn::geometry::area<VEC3>(*map2_, CMap2::Face(it), position_);
//        std::cout << "face tri 1" << std::endl;

        it = map2_->phi<11>(it2);
        e = map2_->cut_face(it, it2);
        dart_level_[e.dart] = fl+1;
        face_subd_id_[CMap2::Face(it2)] = 4*fid+2;
        tri_face_[CMap2::Face(it2)] = true;
        subdivided.mark(CMap2::Face(it2));
        qtrav_->update(e);
        qtrav_->update(CMap2::Face(it2));
        h_[CMap2::Face(it2)] = old_h;
        q_[CMap2::Face(it2)] = old_q;
        r_[CMap2::Face(it2)] = old_r;
        phi_[CMap2::Face(it2)] = old_phi;
        zb_[CMap2::Face(it2)] = 0.;
        nbv = 0.;
        map2_->foreach_incident_vertex(CMap2::Face(it2), [&] (CMap2::Vertex v)
        {
            zb_[CMap2::Face(it2)] += position_[v][2];
            nbv++;
        });
        zb_[CMap2::Face(it2)] /= nbv;
        centroid_[CMap2::Face(it2)] = cgogn::geometry::centroid<VEC3>(*map2_, CMap2::Face(it2), position_);
        area_[CMap2::Face(it2)] = cgogn::geometry::area<VEC3>(*map2_, CMap2::Face(it2), position_);
//        std::cout << "face tri 2" << std::endl;

        it2 = map2_->phi<11>(it);
//        std::cout << "it2" << std::endl;
        e = map2_->cut_face(it, it2);
//        std::cout << "cut face" << std::endl;
        dart_level_[e.dart] = fl+1;
        dart_level_[map2_->phi2(e.dart)] = fl+1;
        face_subd_id_[CMap2::Face(it)] = 4*fid+3;
        tri_face_[CMap2::Face(it)] = true;
        subdivided.mark(CMap2::Face(it));
        qtrav_->update(e);
        qtrav_->update(CMap2::Face(it));
        h_[CMap2::Face(it)] = old_h;
        q_[CMap2::Face(it)] = old_q;
        r_[CMap2::Face(it)] = old_r;
        phi_[CMap2::Face(it)] = old_phi;
        zb_[CMap2::Face(it)] = 0.;
        nbv = 0.;
        map2_->foreach_incident_vertex(CMap2::Face(it), [&] (CMap2::Vertex v)
        {
            zb_[CMap2::Face(it)] += position_[v][2];
            nbv++;
        });
        zb_[CMap2::Face(it)] /= nbv;
        centroid_[CMap2::Face(it)] = cgogn::geometry::centroid<VEC3>(*map2_, CMap2::Face(it), position_);
        area_[CMap2::Face(it)] = cgogn::geometry::area<VEC3>(*map2_, CMap2::Face(it), position_);
//        std::cout << "face tri 3" << std::endl;

        face_subd_id_[CMap2::Face(it2)] = 4*fid+4;
        tri_face_[CMap2::Face(it2)] = true;
        subdivided.mark(CMap2::Face(it2));
        qtrav_->update(CMap2::Face(it2));

        h_[CMap2::Face(it2)] = old_h;
        q_[CMap2::Face(it2)] = old_q;
        r_[CMap2::Face(it2)] = old_r;
        phi_[CMap2::Face(it2)] = old_phi;
        zb_[CMap2::Face(it2)] = 0.;
        nbv = 0.;
        map2_->foreach_incident_vertex(CMap2::Face(it2), [&] (CMap2::Vertex v)
        {
            zb_[CMap2::Face(it2)] += position_[v][2];
            nbv++;
        });
        zb_[CMap2::Face(it2)] /= nbv;
        centroid_[CMap2::Face(it2)] = cgogn::geometry::centroid<VEC3>(*map2_, CMap2::Face(it2), position_);
        area_[CMap2::Face(it2)] = cgogn::geometry::area<VEC3>(*map2_, CMap2::Face(it2), position_);
//        std::cout << "face tri 4" << std::endl;
    }

    else
    {
        // cut face into 4 quads
        it = map2_->phi1(it);
        cgogn::Dart it2 = map2_->phi<11>(it);
        CMap2::Edge e = map2_->cut_face(it, it2);
        dart_level_[e.dart] = fl+1;
        dart_level_[map2_->phi2(e.dart)] = fl+1;
        tri_face_[CMap2::Face(it2)] = false;
        subdivided.mark(CMap2::Face(it2));
        qtrav_->update(CMap2::Face(it2));
        std::cout << "add edge" << std::endl;

        CMap2::Vertex v = map2_->cut_edge(e);
        position_[v] = old_centroid;
        dart_level_[map2_->phi1(e.dart)] = fl+1;
        dart_level_[map2_->phi2(e.dart)] = fl+1;
        qtrav_->update(v);
        qtrav_->update(e);
        qtrav_->update(CMap2::Edge(map2_->phi1(e.dart)));
        std::cout << "add vertex" << std::endl;

        it = map2_->phi2(e.dart);
        it2 = map2_->phi<11>(it2);
        do
        {
            CMap2::Edge ee = map2_->cut_face(it, it2);

            CMap2::Vertex v1(ee.dart);
            CMap2::Vertex v2(map2_->phi2(ee.dart));
            std::cout << position_[v1][0] << "\t" << position_[v1][1] << std::endl;
            std::cout << position_[v2][0] << "\t" << position_[v2][1] << std::endl;

            std::cout << "ee" << std::endl;
            dart_level_[ee.dart] = fl+1;
            std::cout << "dart level ee" << std::endl;
            dart_level_[map2_->phi2(ee.dart)] = fl+1;
            std::cout << "dart level phi2 ee" << std::endl;
            tri_face_[CMap2::Face(it2)] = false;
            std::cout << "tri face" << std::endl;
            subdivided.mark(CMap2::Face(it2));
            std::cout << "mark" << std::endl;
            qtrav_->update(ee);
            std::cout << "qtrav" << std::endl;
            qtrav_->update(CMap2::Face(it2));

            it = map2_->phi2(map2_->phi_1(it));
            std::cout << "it" << std::endl;
            it2 = map2_->phi<11>(it2);
            std::cout << "it2" << std::endl;

        } while (map2_->phi1(it2) != it);
        std::cout << "boucle edges" << std::endl;

        int cmpt = 0;
        map2_->foreach_incident_face(v, [&] (CMap2::Face af)
        {
            cmpt++;
            face_subd_id_[af] = 4*fid+cmpt;
            h_[af] = old_h;
            q_[af] = old_q;
            r_[af] = old_r;
            phi_[af] = old_phi;
            centroid_[af] = cgogn::geometry::centroid<VEC3>(*map2_, af, position_);
            area_[af] = cgogn::geometry::area<VEC3>(*map2_, af, position_);
            zb_[af] = 0.;
            uint32 nbv = 0;
            map2_->foreach_incident_vertex(af, [&] (CMap2::Vertex vv)
            {
                zb_[af] += position_[vv][2];
                nbv++;
            });
            zb_[af] /= nbv;
        });
        std::cout << "maj valeurs subdivision" << std::endl;
    }
}

void Plugin_ShallowWater::simplify_face(CMap2::Face f)
{
    // if we are here, f is simplifiable (part of a group of 4 triangle or quad faces)

    uint32 fid = face_subd_id_[f];
    uint8 fl = face_level(f);

    cgogn::Dart resF;
    std::vector<SCALAR> old_h;
    std::vector<SCALAR> old_q;
    std::vector<SCALAR> old_r;
    std::vector<SCALAR> old_phi;
    std::vector<SCALAR> old_area;

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
        {
            resF = map2_->phi_1(map2_->phi2(f.dart));
            break;
        }
        }

        old_h.push_back(h_[CMap2::Face(it)]);
        old_q.push_back(q_[CMap2::Face(it)]);
        old_r.push_back(r_[CMap2::Face(it)]);
        old_phi.push_back(phi_[CMap2::Face(it)]);
        old_area.push_back(area_[CMap2::Face(it)]);
        map2_->foreach_adjacent_face_through_edge(CMap2::Face(it), [&] (CMap2::Face f)
        {
            old_h.push_back(h_[f]);
            old_q.push_back(q_[f]);
            old_r.push_back(r_[f]);
            old_phi.push_back(phi_[f]);
            old_area.push_back(area_[f]);
        });

        cgogn::Dart next = map2_->phi1(it);
        map2_->merge_incident_faces(CMap2::Edge(it));
        it = next;
        next = map2_->phi1(it);
        map2_->merge_incident_faces(CMap2::Edge(it));
        it = next;
        map2_->merge_incident_faces(CMap2::Edge(it));

        tri_face_[CMap2::Face(resF)] = true;
//        std::cout << "merge incident faces tri" << std::endl;
    }

    else
    {
        cgogn::Dart od = oldest_dart(f);
        resF = od;
        cgogn::Dart it = map2_->phi<11>(od); // central vertex

        map2_->foreach_incident_face(CMap2::Vertex(it), [&] (CMap2::Face f)
        {
            old_h.push_back(h_[f]);
            old_q.push_back(q_[f]);
            old_r.push_back(r_[f]);
            old_phi.push_back(phi_[f]);
            old_area.push_back(area_[f]);
        });

        map2_->merge_incident_faces(CMap2::Vertex(it));

        tri_face_[CMap2::Face(resF)] = false;
//        std::cout << "merge incident faces quad" << std::endl;

    }

    SCALAR area = 0.;
    h_[CMap2::Face(resF)] = 0.;
    q_[CMap2::Face(resF)] = 0.;
    r_[CMap2::Face(resF)] = 0.;
    phi_[CMap2::Face(resF)] = 0.;
    for(int i = 0; i < old_area.size(); i++)
    {
        h_[CMap2::Face(resF)] += old_h[i]*old_area[i];
        q_[CMap2::Face(resF)] += old_q[i]*old_area[i];
        r_[CMap2::Face(resF)] += old_r[i]*old_area[i];
        phi_[CMap2::Face(resF)] += old_phi[i]*old_area[i];
        area += old_area[i];
    }
    h_[CMap2::Face(resF)] /= area;
    q_[CMap2::Face(resF)] /= area;
    r_[CMap2::Face(resF)] /= area;
    phi_[CMap2::Face(resF)] /= area;
    zb_[CMap2::Face(resF)] = 0.;
    int nbv = 0;
    map2_->foreach_incident_vertex(CMap2::Face(resF), [&] (CMap2::Vertex v)
    {
        zb_[CMap2::Face(resF)] += position_[v][2];
        nbv++;
    });
    zb_[CMap2::Face(resF)] /= nbv;
    centroid_[CMap2::Face(resF)] = cgogn::geometry::centroid<VEC3>(*map2_, CMap2::Face(resF), position_);
    area_[CMap2::Face(resF)] = cgogn::geometry::area<VEC3>(*map2_, CMap2::Face(resF), position_);
    face_subd_id_[resF] = uint32((fid-1)/4);
    qtrav_->update(CMap2::Face(resF));

    if(area_[CMap2::Face(resF)] == 0)
    {
        // supprimer resF ou Face(resF)
    }
//    std::cout << "maj valeurs simplification" << std::endl;

    // simplify edges (if possible)    
    cgogn::Dart it = resF;
    do
    {
        cgogn::Dart next = map2_->phi<11>(it);
        if (map2_->is_boundary_cell(CMap2::Face(map2_->phi2(it))) || face_level(CMap2::Face(map2_->phi2(it))) == fl-1)
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
//    std::cout << "simplify edges" << std::endl;
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

struct Plugin_ShallowWater::Str_Riemann_Flux Plugin_ShallowWater::Solv_HLLC(
        SCALAR g, SCALAR hmin, SCALAR small,
        SCALAR zbL,SCALAR zbR,
        SCALAR PhiL,SCALAR PhiR,
        SCALAR hL,SCALAR qL,SCALAR rL,SCALAR hR,SCALAR qR,SCALAR rR
        )
{
    Str_Riemann_Flux Riemann_flux;

    SCALAR F1 = 0;
    SCALAR F2 = 0;
    SCALAR F3 = 0;
    SCALAR s2L = 0;
    SCALAR s2R = 0;

    SCALAR zL = zbL + hL;
    SCALAR zR = zbR + hR;

    if (((hL > hmin) && (hR > hmin)) ||
            ((hL < hmin) && (zR >= zbL + hmin) && (hR > hmin)) ||
            ((hR < hmin) && (zL >= zbR + hmin) && (hL > hmin)))
    {
        //---possible exchange--------
        //There is water in both cells or one of the cells
        //can fill the other one
        //-----wave speed--------
        SCALAR L1L = qL / max_0(hL, small) - sqrt(g * max_0(hL, small));
        SCALAR L3L = qL / max_0(hL, small) + sqrt(g * max_0(hL, small));
        SCALAR L1R = qR / max_0(hR, small) - sqrt(g * max_0(hR, small));
        SCALAR L3R = qR / max_0(hR, small) + sqrt(g * max_0(hR, small));
        SCALAR L1LR = min_1(L1L, L1R, 0e0);
        SCALAR L3LR = max_1(L3L, L3R, 0e0);
        //========================
        SCALAR PhiLR = min_0(PhiL, PhiR);
        //------compute F1--------
        F1 = L3LR * qL - L1LR * qR + L1LR * L3LR * (zR - zL);
        F1 = F1 * PhiLR / max_0(L3LR - L1LR, small);
        //========================
        //-----compute F2---------
        SCALAR F2L = (qL * qL) / max_0(hL, small) + 5e-1 * g * hL * hL;
        SCALAR F2R = (qR * qR) / max_0(hR, small) + 5e-1 * g * hR * hR;
        F2 = (L3LR * PhiL * F2L - L1LR * PhiR * F2R + L1LR * L3LR * (PhiR * qR - PhiL * qL))
                / max_0(L3LR-L1LR, small);
        //==========================
        //-----Compute S2L and S2R---
        SCALAR Fact = 0.5 * PhiLR * (hL + hR);
        s2L = 0.5 * (PhiL * hL * hL - PhiR * hR * hR)
                - Fact * (zL - zR);
        s2L = g * L1LR * s2L / max_0(L3LR - L1LR, small);
        s2R = 0.5 * (PhiR * hR * hR - PhiL * hL * hL)
                - Fact * (zR - zL);
        s2R = g * L3LR * s2R / max_0(L3LR - L1LR, small);
        //============================
        //------Compute F3------------
        if (F1 > 0)
        {
            F3 = F1 * rL / max_0(hL, small);
        }
        else
        {
            F3 = F1 * rR / max_0(hR, small);
        }
    }
    //===================================
    else if ((hL < hmin) && (zR < zbL) && (hR > hmin))
        //------impossible exchange-Cell L empty------
        //The cell L is empty and the water level in the cell R
        //is below zbL-filling is impossible
    {
        F1 = 0e0;
        F2 = 0e0;
        s2L = 0e0;
        s2R = PhiR * 5e-1 * g * hR * hR;
        F3 = 0e0;
    }
    //===============================================
    else if ((hR < hmin) && (zL < zbR) && (hL > hmin))
        //------impossible exchange-Cell R empty------
        //The cell R is empty and the water level in the cell L
        //is below zbR-filling is impossible
    {
        F1 = 0e0;
        F2 = 0e0;
        s2L = -PhiL * 0.5 * g * hL * hL;
        s2R = 0e0;
        F3 = 0e0;
    }
    //===============================================
    else
        //Both cells below hmin:exchange is impossible
    {
        F1 = 0e0;
        F2 = 0e0;
        F3 = 0e0;
        s2L = 0e0;
        s2R = 0e0;
    }
    Riemann_flux.F1 = F1;
    Riemann_flux.F2 = F2;
    Riemann_flux.F3 = F3;
    Riemann_flux.s2L = s2L;
    Riemann_flux.s2R = s2R;

    return Riemann_flux;
}

SCALAR Plugin_ShallowWater::min_0(SCALAR a, SCALAR b)
{
    if(a>b)
        return b;
    else
        return a;
}

SCALAR Plugin_ShallowWater::max_0(SCALAR a, SCALAR b)
{
    if(a>b)
        return a;
    else
        return b;
}

SCALAR Plugin_ShallowWater::min_1(SCALAR a, SCALAR b, SCALAR c)
{
    if((a<=b)&&(a<=c))
        return a;
    else if((b<=a)&&(b<=c))
        return b;
    else
        return c;
}

SCALAR Plugin_ShallowWater::max_1(SCALAR a, SCALAR b, SCALAR c)
{
    if((a>=b)&&(a>=c))
        return a;
    else if((b>=a)&&(b>=c))
        return b;
    else
        return c;
}


struct Plugin_ShallowWater::Str_Riemann_Flux Plugin_ShallowWater::Solv_PorAS
        (SCALAR g, SCALAR hmin, SCALAR small,
         SCALAR zbL, SCALAR zbR,
         SCALAR PhiL, SCALAR PhiR,
         SCALAR hL,SCALAR qL,SCALAR rL,
         SCALAR hR,SCALAR qR,SCALAR rR)
{
    Str_Riemann_Flux Riemann_Flux;

    SCALAR zL = hL + zbL;               // free surface elevation on the left side
    SCALAR zR = hR + zbR;               // free surface elevation on the right side

    if (((hL > hmin) && (hR > hmin)) ||
        ((hL > hmin) && (zR >= zbL + hmin) && (hR > hmin)) ||
        ((hR > hmin) && (zL >= zbR + hmin) && (hL > hmin)))
        {
                // Initialisation
            SCALAR cL = sqrt(g * hL);           // pressure wave speed on the left side
            SCALAR cR = sqrt(g * hR);           // pressure wave speed on the right side
            SCALAR uL;                          // velocity on the left side
            if (hL > hmin)
            {
                uL = qL / max_0(hL, small);
            }
            else
            {
                uL = 0;
            }
            SCALAR uR;                          // velocity on the right side
            if (hR > hmin)
            {
                uR = qR / max_0(hR, small);
            }
            else
            {
                uR = 0;
            }

            SCALAR F1L = PhiL * qL;             // flux of mass on the left side
            SCALAR F1R = PhiR * qR;             // flux of mass on the right side
            SCALAR F2L = PhiL * (pow(uL, 2) * hL + 5e-1 * g * pow(hL, 2));   // flux of momentum on the left side
            SCALAR F2R = PhiR * (pow(uR, 2) * hR + 5e-1 * g * pow(hR, 2));   // flux of momentum on the right side

                // Célérités d'ondes
                // Waves speed
            SCALAR L1L = uL - cL;          // 1st wave speed on the left side (u-c)
            SCALAR L1R = uR - cR;          // 1st wave speed on the right side (u-c)
            SCALAR L2L = uL + cL;          // 2nd wave speed on the left side (u+c)
            SCALAR L2R = uR + cR;          // 2nd wave speed on the right side (u+c)
            SCALAR L1LR = min_0(L1L,L1R);    // 1st wave speed at the interface
            SCALAR L2LR = max_0(L2L,L2R);    // 2nd wave speed at the interface

                // Zone intermédiaire
                // Intermediate state
                /* L'état intermédiaire est calculé en utilisant les invariants de Riemann */
                /* The intermediate state i computed using the Riemann invariants */
            SCALAR uI = ((uL + uR) / 2) + cL - cR;          // velocity in the intermediate state
            SCALAR cI = ((uL - uR) / 4) + ((cL + cR) / 2);  // pressure wave speed in the intermediate state
            SCALAR hI = pow(cI, 2) / g;                      // water depth in the intermediate state
            SCALAR L1I = uI - cI;                           // 1st wave speed in the intermediate state (u-c)
            SCALAR L2I = uI + cI;                           // 2nd wave speed in the intermediate state (u+c)
                // Discretisation des termes sources
                // Source term discretisation

            SCALAR zS;          // free surface elevation for the source term
            if (zbL > zbR)
            {
                zS = zR;
            }
            else
            {
                zS = zL;
            }
            SCALAR hS;          // water depth for the source term
            SCALAR PhiS;        // porosity for the source term
            if (PhiL > PhiR)
            {
                hS = hL;
                PhiS = PhiR;
            }
            else
            {
                hS = hR;
                PhiS = PhiL;
            }
            SCALAR Phi_Term = 5e-1 * g * (PhiR - PhiL) * pow(hS,2);                 // Porosity contribution to the source term
            SCALAR Bot_Term = g * PhiS * (zbR - zbL) * (zS - ((zbL + zbR) / 2));    // Bottom contribution to the source term

                // Identification de la nature des ondes
                // Determination of the wave type
                // 1ère onde (u-c)
                // 1st wave (u-c)
                // L1L>L1I => Shock for the 1st wave
            if (L1L > L1I)
            {
                // ???? PFG : pourquoi dans cet ordre ????
                if (fabs(hI - hL > hmin))
                {
                    L1L = (uI * hI - uL * hL) / (hI - hL);
                }
                /** @todo remplacer @a hmin par @a small ??? **/
                if (fabs(uI * hI - uL * hL) > hmin)
                {
                    L1L = (pow(uI, 2) * hI - pow(uL, 2) * hL + 5e-1 * g * pow(hI, 2) - 5e-1 * g * pow(hL, 2)) / (uI * hI - uL * hL);
                }
                L1I = L1L;
            }
            // 2ème onde (u+c)
            // 2nd wave (u+c)
            // L3R<L3I => Shock for the 2nd wave
            if (L2R < L2I)
            {
                // ???? PFG : pourquoi dans cet ordre ????
                if (fabs(hI - hR > hmin))
                {
                    L2R = (uI * hI - uR * hR) / (hI - hR);
                }
                /** @todo remplacer @a hmin par @a small ??? **/
                if (fabs(uI * hI - uR * hR) > hmin)
                {
                    L2R = (pow(uR, 2) * hR - pow(uI, 2) * hI + 5e-1 * g * pow(hR, 2) - 5e-1 * g * pow(hI, 2)) / (uR * hR - uI * hI);
                }
                L2I = L2R;
            }

            // Calcul du flux à travers l'interface
            // Flux computation through the interface
            if (L1L >= 0)
            {
                // Ecoulement torrentiel de la maille L vers la maille R
                // Supercritical flow from L-cell to R-cell
                Riemann_Flux.F1 = F1L;
                Riemann_Flux.F2 = F2L;
            }
            else if ((L1L < 0) && (L1I >= 0))
            {
                // Ecoulement critique de la maille L vers la maille R
                // Critical flow from L-cell to R-cell
                SCALAR PhiLR = PhiL;
                Riemann_Flux.F1 = (L2LR * F1L - L1LR * F1R - L1LR * L2LR * PhiLR * (zL - zR)) / max_0(L2LR - L1LR, small);
                Riemann_Flux.F2 = (L2LR * F2L - L1LR * F2R - L1LR * L2LR * (F1L - F1R)) / max_0(L2LR - L1LR, small);
            }
            else if ((L1I <0) && (L2I >= 0))
            {
                // Ecoulement fluvial entre les mailles L et R
                // Calcul des flux dans la zone intermédiaire d'état constant
                // Subcritical flow between the L-cell and R-cell
                // Flux computation in the constant intermediate zone
                Riemann_Flux.F1 = ((F2L - F2R - L1LR * F1L + L2LR * F1R) + (Phi_Term - Bot_Term)) / max_0(L2LR - L1LR, small);
                Riemann_Flux.F2 = (L2LR * F2L - L1LR * F2R - L1LR * L2LR * (F1L - F1R)) / max_0(L2LR - L1LR, small);
            }
            else if ((L2I < 0) && (L2R >= 0))
            {
                // Ecoulement critique de la maille R vers la maille L
                // Critical flow from R-cell to L-cell
                SCALAR PhiLR = PhiR;
                Riemann_Flux.F1 = (L2LR * F1L - L1LR * F1R - L1LR * L2LR * PhiLR * (zL - zR)) / max_0(L2LR - L1LR, small);
                Riemann_Flux.F2 = (L2LR * F2L - L1LR * F2R - L1LR * L2LR * (F1L - F1R)) / max_0(L2LR - L1LR, small);
            }
            else if (L2R < 0)
            {
                // Ecoulement torrentiel de la maille R vers la maille L
                // Supercritical flow from R-cell to L-cell
                Riemann_Flux.F1 = F1R;
                Riemann_Flux.F2 = F2R;
            }

            // Computation of F3
            if (Riemann_Flux.F1 > 0e0)
            {
                Riemann_Flux.F3 = Riemann_Flux.F1 * rL / max_0(hL, small);
            }
            else
            {
                Riemann_Flux.F3 = Riemann_Flux.F1 * rR / max_0(hR, small);
            }
            // Upwind computation of the source term s2L and s2R
            Riemann_Flux.s2L = - L1LR * (-Bot_Term + Phi_Term) / max_0(L2LR - L1LR, small);
            Riemann_Flux.s2R = L2LR * (-Bot_Term + Phi_Term) / max_0(L2LR - L1LR, small);
    }
    else if ((hL <= hmin) && (zR < zbL) && (hR > hmin))
    {
        // Maille R en eau mais niveau sous le fond de la maille L
        // Water in the R-cell but water is below the bottom of the L-cell
        Riemann_Flux.F1 = 0;
        Riemann_Flux.F2 = 0;
        Riemann_Flux.F3 = 0;
        Riemann_Flux.s2L = 0;
        Riemann_Flux.s2R = 5e-1 * g * PhiR * pow(hR, 2);
    }
    else if ((hR <= hmin) && (zL < zbR) && (hL > hmin))
    {
        // Maille L en eau mais niveau sous le fond de la maille R
        // Water in the L-cell but water is below the bottom of the R-cell
        Riemann_Flux.F1 = 0;
        Riemann_Flux.F2 = 0;
        Riemann_Flux.F3 = 0;
        Riemann_Flux.s2L = -5e-1 * g * PhiR * pow(hL, 2);
        Riemann_Flux.s2R = 0;
    }
    else
        //Both cells below hmin:exchange is impossible
    {
        Riemann_Flux.F1 = 0e0;
        Riemann_Flux.F2 = 0e0;
        Riemann_Flux.F3 = 0e0;
        Riemann_Flux.s2L = 0e0;
        Riemann_Flux.s2R = 0e0;
    }


    return Riemann_Flux;
}

struct Plugin_ShallowWater::Str_Riemann_Flux Plugin_ShallowWater::border_condition(
        int border_condition_choice, SCALAR val_bc, bool sideR,
        SCALAR normX, SCALAR normY,
        SCALAR q, SCALAR r, SCALAR z, SCALAR zb)
{
    // initialization
    SCALAR q1 = q*normX + r*normY;
    SCALAR r1 = -q*normY + r*normX;
    SCALAR h1 = z-zb;
    SCALAR f1,f2,f3;
    Str_Riemann_Flux riemann_flux;
    if(sideR)
    {
        q1 = -q1;
        r1 = -r1;
    }
    if(h1 < hmin_)
    {
        h1 = 0.;
        q1 = 0.;
        r1 = 0.;
    }

    // characteristic variables
    SCALAR c1 = sqrt(9.81*h1);
    SCALAR u1 = q1 / max_0(h1, small_);
    SCALAR v1 = r1 / max_0(h1, small_);
    SCALAR L1 = max_0(u1 + c1, 0e0);

    // boundary conditions
    if(border_condition_choice == 0) //-----Free Outflow-------
    {
        f1 = q1;
        f2 = q1 * u1 + 0.5 * 9.81 * h1 * h1;
    }
    else if(border_condition_choice == 1) //-------Critical Section----
    {
        SCALAR c = (u1-2.*c1)/(val_bc-2.);
        c = max_0(c,0);
        SCALAR u = -val_bc*c;
        SCALAR h = c*c/9.81;
        f1 = h*u;
        f2 = h*u*u+9.81*h*h/2.;
    }
    else if(border_condition_choice == 2) //-------Prescribed h---------
    {
        SCALAR h = max_0(val_bc,0.);
        SCALAR u = 0.;
        if(L1 < 0) // torrentiel sortant
        {
            h = h1;
            u = u1;
        }
        else
        {
            SCALAR cmin = max_1(sqrt(9.81 * h), (2 * c1 - u1) / 3.0, 0.0);
            h = max_0(h, (cmin * cmin) / 9.81);
            SCALAR c = sqrt(9.81*h);
            u = u1 + 2 * (c - c1);
        }
        f1 = h*u;
        f2 = h*u*u+9.81*h*h/2.;
    }
    else if(border_condition_choice == 3) //-------Prescribed z-----------
    {
        SCALAR h=max_0(val_bc - zb,0);
        SCALAR c = sqrt(9.81*h);
        SCALAR u=u1+2*(c-c1);
        f1 = h*u;
        f2 = h*u*u+9.81*h*h/2.;

        h = max_0(val_bc-zb,0.); //why is part need
        if(L1 < 0)
        {
            /* torrentiel sortant*/
            h = h1;
            u = u1;
        }
        else
        {
            SCALAR cmin = max_1(sqrt(9.81*h),(2.*c1-u1)/2.,0.);
            h = max_0(h, cmin*cmin/9.81);
            c = sqrt(9.81*h);
            u = u1+2.*(c-c1);
        }
        f1 = h*u;
        f2 = h*u*u+9.81*h*h/2.;
    }
    else if(border_condition_choice == 4) //--------Prescribed q-----------
    {
        f1 = val_bc;
        SCALAR hc = pow(f1*f1/9.81,1./3.);
        if(hc >= h1)
            f2 = q1*q1/max_0(hc,small_) + 9.81*h1*h1/2. + (f1-q1)*L1;
        else
            f2 = q1*q1/max_0(h1,small_) + 9.81*h1*h1/2. + (f1-q1)*L1;
    }
    else if(border_condition_choice == 5) //---------Weir--------------------
    {
        if(h1 < val_bc) // No z:weir elevation not reached
        {
            f1 = 0.;
            f2 = (q1*q1)/max_0(h1,small_) + 9.81*h1*h1/2.;
        }
        else // Weir overtoped
        {
            f1 = -0.42*sqrt(2.*9.81)*pow((h1-val_bc),3./2.);
            f2 = (q1*q1)/max_0(h1,small_) + 9.81*h1*h1/2.;
        }
    }
    f3 = (f1-fabs(f1))*v1/2.;
    if(sideR)
        f1 = -f1;

    riemann_flux.F1 = f1;
    riemann_flux.F2 = f2;
    riemann_flux.F3 = f3;
    riemann_flux.s2L = 0.;
    riemann_flux.s2R = 0.;
    return riemann_flux;
}

void Plugin_ShallowWater::get_LR_faces(CMap2::Edge e, CMap2::Face& fl, CMap2::Face& fr)
{
    CMap2::Face fR(e.dart);
    CMap2::Face fL(map2_->phi2(e.dart));
    CMap2::Vertex v1(e.dart);
    CMap2::Vertex v2(map2_->phi1(e.dart));
    CMap2::Vertex vB,vH;
    if(position_[v2][1] < position_[v1][1] )
    {
        vB = v2;
        vH = v1;
    }
    else if(position_[v2][1] == position_[v1][1])
    {
        if(position_[v2][0] <= position_[v1][0])
        {
            vB = v2;
            vH = v1;
        }
        else
        {
            vB = v1;
            vH = v2;
        }
    }
    else
    {
        vB = v1;
        vH = v2;
    }

    if(area_[fR] == 0 || area_[fL] == 0)
    {
        std::cout << "aire nulle" << std::endl;
    }

    SCALAR x1 = position_[vB][0];
    SCALAR y1 = position_[vB][1];
    SCALAR x2 = position_[vH][0];
    SCALAR y2 = position_[vH][1];
    SCALAR v2x = x2 - x1;
    SCALAR v2y = y2 - y1;
    SCALAR x3 = centroid_[fL][0];
    SCALAR y3 = centroid_[fL][1];
    SCALAR x4 = centroid_[fR][0];
    SCALAR y4 = centroid_[fR][1];
    SCALAR v1x = x4 - x1;
    SCALAR v3x = x3 - x1;
    SCALAR v1y = y4 - y1;
    SCALAR v3y = y3 - y1;
    SCALAR prod23 = v2x * v3y - v3x * v2y;
    SCALAR prod12 = v1x * v2y - v2x * v1y;
    //test if 2 gravity centres on the same side
    if(prod12*prod23 <= 0. && !(area_[fR] == 0 || area_[fL] == 0))
    {
        std::cout << "2 gravity centres on the same side" << std::endl;
    }
    //If the product is negative,exchange left and right : Normal vector from Left to right
    if(prod12 < 0)
    {
        CMap2::Face f;
        f = fL;
        fL = fR;
        fR = f;
    }
    fl = fL;
    fr = fR;
}


} // namespace plugin_shallow_water_2

} // namespace schnapps
