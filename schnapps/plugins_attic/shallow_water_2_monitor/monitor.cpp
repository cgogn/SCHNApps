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

#include <schnapps/plugins/shallow_water_2_monitor/monitor.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/camera.h>

namespace schnapps
{


namespace plugin_shallow_water_2_monitor
{

Plugin_Shallow_Water_2_Monitor::Plugin_Shallow_Water_2_Monitor()
{
    this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_Shallow_Water_2_Monitor::plugin_name()
{
    return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_Shallow_Water_2_Monitor::enable()
{
    shallow_water_ = static_cast<plugin_shallow_water_2::Plugin_ShallowWater*>(schnapps_->enable_plugin(plugin_shallow_water_2::Plugin_ShallowWater::plugin_name()));
    render_ = static_cast<plugin_surface_render::Plugin_SurfaceRender*>(schnapps_->enable_plugin(plugin_surface_render::Plugin_SurfaceRender::plugin_name()));
    render_scalar_ = static_cast<plugin_surface_render_scalar::Plugin_SurfaceRenderScalar*>(schnapps_->enable_plugin(plugin_surface_render_scalar::Plugin_SurfaceRenderScalar::plugin_name()));

    v_ = schnapps_->get_selected_view();

//    v_->get_current_camera()->from_string("-7.11056 -27.1433 125.902 0.263332 -0.165049 -0.0993567 0.945274");
    v_->get_current_camera()->from_string("-18.9002 -57.5899 96.6947 0.390173 -0.186984 -0.154336 0.888247");

    v_->link_plugin(render_);
    v_->link_plugin(render_scalar_);

    shallow_water_action = schnapps_->add_menu_action("Simulation;Shallow Water;Run script", "run shallow water simulation script");
    connect(shallow_water_action, SIGNAL(triggered()), this, SLOT(run_script()));

    check_simu_timer_ = new QTimer(this);
    connect(check_simu_timer_, SIGNAL(timeout()), this, SLOT(check_simu_state()));

////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     MAILLAGES FIXES DE NIVEAU 0 1 2 3 4 5
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//0
//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(0);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

//1
//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(1);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

//2
//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(2);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

//3
//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(3);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

//4
//    f_.push_back([&] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//        shallow_water_->set_max_depth(4);
//        shallow_water_->set_adaptive_mesh(false);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });

//5
//    f_.push_back([=] () {
//                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                shallow_water_->set_max_depth(5);
//                shallow_water_->set_adaptive_mesh(false);
//                shallow_water_->init();

//                render_scalar_->update_min_max(v_, mesh, true);

//                shallow_water_->start();
//            });
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     CRITERE  SPATIAL SUR H
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//        std::vector<SCALAR> seuils_seuil_sub_h = { 0.5};//, 1, 2, 2.5 };//{0.4,0.3,0.2,0.1,0.09,0.08,0.07,0.06};
//        SCALAR smph=0.05;//0.1;
//        for (SCALAR s : seuils_seuil_sub_h)
//        {
//            f_.push_back([=] () {
//                MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//                shallow_water_->set_max_depth(4);
//                shallow_water_->set_adaptive_mesh(true);
//                shallow_water_->set_seuil_sub_h(s);
//                shallow_water_->set_seuil_sub_V(s);
//                shallow_water_->set_seuil_simp_h(smph);
//                shallow_water_->set_seuil_simp_V(smph);

//                shallow_water_->set_criteria(plugin_shallow_water_2::H_spatial);
//                shallow_water_->init();

//                render_scalar_->update_min_max(v_, mesh, true);

//                shallow_water_->start();
//            });
//        }



////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     CRITERE SPATIAL SUR LA VITESSE
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//                std::vector<SCALAR> seuils_seuil_sub_v = {4.};//,3.,2.,1.,0.9,0.8};
//                for (SCALAR s : seuils_seuil_sub_v)
//                {
//                    f_.push_back([=] () {
//                        MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//                        shallow_water_->set_max_depth(5);
//                        shallow_water_->set_adaptive_mesh(true);
//                        shallow_water_->set_seuil_sub_V(s);//3
//                        shallow_water_->set_seuil_simp_V(0.6);//1.2
//                        shallow_water_->set_criteria(plugin_shallow_water_2::Q_R_tempo);
//                        shallow_water_->init();

//                        render_scalar_->update_min_max(v_, mesh, true);

//                        shallow_water_->start();
//                    });
//                }

////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     CRITERE SPATIAL MIXTE
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//            std::vector<SCALAR> seuils_sub_h = {1.};//{ 0.5, 1, 2, 2.5 };
//            for (SCALAR s : seuils_sub_h)
//            {
//                f_.push_back([=] () {
//                    MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//                    shallow_water_->set_max_depth(4);
//                    shallow_water_->set_adaptive_mesh(true);
//                    shallow_water_->set_seuil_sub_h(s);
//                    shallow_water_->set_seuil_sub_V(3.);//3
//                    shallow_water_->set_seuil_simp_h(s/2.);
//                    shallow_water_->set_seuil_simp_V(1.5);

//                    shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_spatial);
//                    shallow_water_->init();

//                    render_scalar_->update_min_max(v_, mesh, true);

//                    shallow_water_->start();
//                });
//            }



////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     CRITERE TEMPOREL SUR H
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//    f_.push_back([=] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//        shallow_water_->set_max_depth(3);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_criteria(plugin_shallow_water_2::H_tempo);
//        shallow_water_->set_iteradapt(1);
//        shallow_water_->set_seuil_sub_h(10);
//        shallow_water_->set_seuil_simp_h(5);

//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });

////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     CRITERE TEMPOREL SUR V
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


//    f_.push_back([=] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//        shallow_water_->set_max_depth(3);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_criteria(plugin_shallow_water_2::Q_R_tempo);
//        shallow_water_->set_iteradapt(1);
//        shallow_water_->set_seuil_sub_V(10);
//        shallow_water_->set_seuil_simp_V(5);

//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });


////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////                     CRITERE TEMPOREL MIXTE
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    f_.push_back([=] () {
        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
        shallow_water_->set_max_depth(3);
        shallow_water_->set_adaptive_mesh(true);
        shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_tempo);
        shallow_water_->set_iteradapt(1);
        shallow_water_->set_seuil_sub_h_V(10, 10);
        shallow_water_->set_seuil_simp_h_V(5,5);

        shallow_water_->set_export_frames(true);
        shallow_water_->set_export_frames_step(0.5);
        shallow_water_->set_export_frames_dir("/home/dahik/Data/905_Dambreak_Complexe_grossier/Output/frames/");

        shallow_water_->init();

        render_scalar_->update_min_max(v_, mesh, true);

        shallow_water_->start();
    });


////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
////~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



    return true;
}

void Plugin_Shallow_Water_2_Monitor::disable()
{
    disconnect(shallow_water_action, SIGNAL(triggered()), this, SLOT(run_script()));
    schnapps_->remove_menu_action(shallow_water_action);

    delete check_simu_timer_;
}

void Plugin_Shallow_Water_2_Monitor::run_script()
{
    current_f_ = 0;
    f_[current_f_]();
    check_simu_timer_->start(1000);
}

void Plugin_Shallow_Water_2_Monitor::check_simu_state()
{
    std::future_status status = shallow_water_->simu_future()->wait_for(std::chrono::milliseconds(1));
    if (status == std::future_status::ready)
    {
        ++current_f_;
        if (current_f_ < f_.size())
            f_[current_f_]();
        else
            check_simu_timer_->stop();
    }
}

MapHandlerGen* Plugin_Shallow_Water_2_Monitor::load(const QString& dir)
{
    shallow_water_->load_project(dir);
    MapHandlerGen* mesh = schnapps_->get_map("shallow_water_2");

    mesh->set_bb_vertex_attribute("water_position");

    v_->link_map(mesh);

    cgogn::rendering::VBO* water_position_vbo = mesh->create_vbo("water_position");
    cgogn::rendering::VBO* h_vbo = mesh->create_vbo("scalar_value_h");

    render_->set_position_vbo(v_, mesh, water_position_vbo, true);
    render_->set_render_edges(v_, mesh, true, true);
    render_->set_render_faces(v_, mesh, false, true);

    render_scalar_->set_position_vbo(v_, mesh, water_position_vbo, true);
    render_scalar_->set_scalar_vbo(v_, mesh, h_vbo, true);
    render_scalar_->set_auto_update_min_max(v_, mesh, false, true);
    render_scalar_->set_scalar_min(v_, mesh, .0, true);

    return mesh;
}

} // namespace plugin_shallow_water_2_monitor

} // namespace schnapps
