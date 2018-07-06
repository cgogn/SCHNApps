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

    v_->link_plugin(render_);
    v_->link_plugin(render_scalar_);

	shallow_water_action = schnapps_->add_menu_action("Simulation;Shallow Water;Run script", "run shallow water simulation script");
	connect(shallow_water_action, SIGNAL(triggered()), this, SLOT(run_script()));

	check_simu_timer_ = new QTimer(this);
	connect(check_simu_timer_, SIGNAL(timeout()), this, SLOT(check_simu_state()));

    //0
        f_.push_back([&] () {
            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

            shallow_water_->set_max_depth(0);
            shallow_water_->set_adaptive_mesh(false);
            shallow_water_->init();

            render_scalar_->update_min_max(v_, mesh, true);

            shallow_water_->start();
        });

    //1
        f_.push_back([&] () {
            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

            shallow_water_->set_max_depth(1);
            shallow_water_->set_adaptive_mesh(false);
            shallow_water_->init();

            render_scalar_->update_min_max(v_, mesh, true);

            shallow_water_->start();
        });

//    //2
//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(2);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

//    ////3
//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(3);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

        ////4
        //    f_.push_back([&] () {
        //        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

        //        shallow_water_->set_max_depth(4);
        //        shallow_water_->set_adaptive_mesh(false);
        //        shallow_water_->init();

        //        render_scalar_->update_min_max(v_, mesh, true);

        //        shallow_water_->start();
        //    });

    ////5
    //    f_.push_back([=] () {
    //                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
    //                shallow_water_->set_max_depth(5);
    //                shallow_water_->set_adaptive_mesh(false);
    //                shallow_water_->init();

    //                render_scalar_->update_min_max(v_, mesh, true);

    //                shallow_water_->start();
    //            });




//6
//    f_.push_back([&] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//        shallow_water_->set_max_depth(4);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });
//7
//    f_.push_back([&] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//        shallow_water_->set_max_depth(1);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });
//8
//    f_.push_back([&] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//        shallow_water_->set_max_depth(4);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_iteradapt(2);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });
//9
//    f_.push_back([&] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//        shallow_water_->set_max_depth(4);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_iteradapt(10);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });

//10

//    f_.push_back([&] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//        shallow_water_->set_max_depth(4);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_sigma_sub(0.9);
//        shallow_water_->set_sigma_simp(0.001);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });

// chifaa test de seuil


//    std::vector<SCALAR> seuils_sigma_sub_h = { 0.5, 1, 2, 2.5 };

//    for (SCALAR s : seuils_sigma_sub_h)
//    {
//        f_.push_back([=] () {
//            MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(4);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_sigma_sub_h(s);
//            shallow_water_->set_sigma_sub_vitesse(s);
//            shallow_water_->set_sigma_simp_h(s/2.0);
//            shallow_water_->set_sigma_simp_vitesse(s/2.0);

//            shallow_water_->set_criteria(plugin_shallow_water_2::H);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    }


//		std::vector<SCALAR> seuils_sigma_sub_h = {1.};//{ 0.5, 1, 2, 2.5 };
//		for (SCALAR s : seuils_sigma_sub_h)
//		{
//			f_.push_back([=] () {
//				MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//				shallow_water_->set_max_depth(4);
//				shallow_water_->set_adaptive_mesh(true);
//				shallow_water_->set_sigma_sub_h(s);
//				shallow_water_->set_sigma_sub_vitesse(3.);//3
//				shallow_water_->set_sigma_simp_h(s/2.);
//				shallow_water_->set_sigma_simp_vitesse(1.5);

//				shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R);
//				shallow_water_->init();

//				render_scalar_->update_min_max(v_, mesh, true);

//				shallow_water_->start();
//			});
//		}

//    f_.push_back([=] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//        shallow_water_->set_max_depth(2);
//        shallow_water_->set_adaptive_mesh(false);

//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });


//    f_.push_back([=] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//        shallow_water_->set_max_depth(2);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });


//    f_.push_back([=] () {
//        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//        shallow_water_->set_max_depth(3);
//        shallow_water_->set_adaptive_mesh(true);
//        shallow_water_->set_criteria(plugin_shallow_water_2::H_old);
//        shallow_water_->set_seuil_simp_h_old(0.0015);
//        shallow_water_->set_seuil_sub_h_old(0.008);

//        shallow_water_->init();

//        render_scalar_->update_min_max(v_, mesh, true);

//        shallow_water_->start();
//    });


//        std::vector<SCALAR> seuils_angle = {0.020,0.015,0.01};
//        for (SCALAR s : seuils_angle)
//       {
//        f_.push_back([=] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//            shallow_water_->set_max_depth(5);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_criteria(plugin_shallow_water_2::H_angle_norm_V);
//            shallow_water_->set_seuil_simp_angle_norm(0.0015,0.002,0.);
//            shallow_water_->set_seuil_sub_angle_norm(0.008,s,0.); //0.01

//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    }

// ############### critere spatial sur l'angle
//    std::vector<SCALAR> seuils_angle = {0.3}; // 0.4 0.3 0.2
//    for (SCALAR s : seuils_angle)
//    {
//        f_.push_back([=] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//            shallow_water_->set_max_depth(5);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_criteria(plugin_shallow_water_2::angle_V);
//            shallow_water_->set_seuil_simp_angle(0.08);
//            shallow_water_->set_seuil_sub_angle(s);// 0.3

//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    }

//    // ############### critere spatial sur Q et R
//            std::vector<SCALAR> seuils_sigma_sub_v = {4.,3.,2.,1.,0.9,0.8};
//            for (SCALAR s : seuils_sigma_sub_v)
//            {
//                f_.push_back([=] () {
//                    MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//                    shallow_water_->set_max_depth(5);
//                    shallow_water_->set_adaptive_mesh(true);
//                    shallow_water_->set_sigma_sub_vitesse(s);//3
//                    shallow_water_->set_sigma_simp_vitesse(0.6);//1.2
//                    shallow_water_->set_criteria(plugin_shallow_water_2::Q_R);
//                    shallow_water_->init();

//                    render_scalar_->update_min_max(v_, mesh, true);

//                    shallow_water_->start();
//                });
//            }




////        // ############### critere temporel sur Q et R
//                std::vector<SCALAR> seuils_sigma_sub_v ={0.06,0.05,0.04,0.03}; //{0.12,0.11,0.1,0.09,0.08,0.07};
//                for (SCALAR s : seuils_sigma_sub_v)
//                {
//                    f_.push_back([=] () {
//                        MapHandlerGen* mesh = this->load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//                        shallow_water_->set_max_depth(5);
//                        shallow_water_->set_adaptive_mesh(true);
//                        shallow_water_->set_seuil_sub_q_r_old(s,s);
//                        shallow_water_->set_seuil_simp_q_r_old(0.01,0.01);
//                        shallow_water_->set_criteria(plugin_shallow_water_2::Q_R_old);
//                        shallow_water_->init();

//                        render_scalar_->update_min_max(v_, mesh, true);

//                        shallow_water_->start();
//                    });
//                }



            // ##################### critere vitesse variation spatiale
//        f_.push_back([=] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//            shallow_water_->set_max_depth(3);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_criteria(plugin_shallow_water_2::H_angle_norm_V);
//            shallow_water_->set_seuil_simp_angle_norm(0., 0.002,0.);
//            shallow_water_->set_seuil_sub_angle_norm(0., 0.02, 0.);

//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    std::vector<SCALAR> seuils_hqr_old = {0.9};//{0.8,0.7};
//    for (SCALAR s : seuils_hqr_old)
//    {
//        f_.push_back([=] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//            shallow_water_->set_max_depth(3);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//            shallow_water_->set_iteradapt(1);
//            shallow_water_->set_seuil_sub_h_q_r_old(0.009, s, s);
//            shallow_water_->set_seuil_simp_h_q_r_old(0.0015, 0.15, 0.15);

//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    }
//#### comparer erreur temporelle
// 1
//    f_.push_back([=] () {
//                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                shallow_water_->set_max_depth(5);
//                shallow_water_->set_adaptive_mesh(true);
//                shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//                shallow_water_->set_iteradapt(1);
//                shallow_water_->set_seuil_sub_h_q_r_old(0.004, 0.9, 0.9);
//                shallow_water_->set_seuil_simp_h_q_r_old(0.0015, 0.15, 0.15);
//                shallow_water_->init();

//                render_scalar_->update_min_max(v_, mesh, true);

//                shallow_water_->start();
//            });

//2
//    f_.push_back([=] () {
//                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                shallow_water_->set_max_depth(5);
//                shallow_water_->set_adaptive_mesh(true);
//                shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//                shallow_water_->set_iteradapt(1);
//                shallow_water_->set_seuil_sub_h_q_r_old(0.03, 0.9, 0.9);
//                shallow_water_->set_seuil_simp_h_q_r_old(0.0015, 0.15, 0.15);
//                shallow_water_->init();

//                render_scalar_->update_min_max(v_, mesh, true);

//                shallow_water_->start();
//            });





//############################################## seuil subd h
    //std::vector<SCALAR> seuils_subd_h_hqr_old = {0.03,0.02,0.01,0.009,0.008,0.007,0.006};
//    std::vector<SCALAR> seuils_subd_h_hqr_old = {0.004,0.003,0.001};
//    SCALAR smph=0.0015;
//    SCALAR sbqr=0.9;
//    SCALAR smpqr=0.15;
//    //std::vector<SCALAR> seuils_simp_h_hqr_old = {0.005,0.004,0.003,0.002,0.001};
//    //std::vector<SCALAR> seuils_subd_qr_hqr_old = {2.,1.7,1.3,1.,0.9,0.8,0.7};
//    //std::vector<SCALAR> seuils_simp_qr_hqr_old = {0.5,0.4,0.3,0.15,0.1,0.05};

//    for (SCALAR sbh : seuils_subd_h_hqr_old)
//    {
//    //    for (SCALAR smph : seuils_simp_h_hqr_old)
//    //    {

//    //        for (SCALAR sbqr : seuils_subd_qr_hqr_old)
//    //        {
//    //            for (SCALAR smpqr : seuils_simp_qr_hqr_old)

//    //            {
//                        f_.push_back([=] () {
//                            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                            shallow_water_->set_max_depth(5);
//                            shallow_water_->set_adaptive_mesh(true);
//                            shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//                            shallow_water_->set_iteradapt(1);
//                            shallow_water_->set_seuil_sub_h_q_r_old(sbh, sbqr, sbqr);
//                            shallow_water_->set_seuil_simp_h_q_r_old(smph, smpqr, smpqr);

//                            shallow_water_->init();

//                            render_scalar_->update_min_max(v_, mesh, true);

//                            shallow_water_->start();
//                        });

//    //            }
//    //        }
//    //    }
//    }
//############################################## seuil simp h
//    std::vector<SCALAR> seuils_simp_h_hqr_old = {0.0025,0.002,0.0015,0.001,0.0007,0.0006};
//    SCALAR sbh=0.004;
//    SCALAR sbqr=0.9;
//    SCALAR smpqr=0.15;

//    for (SCALAR smph : seuils_simp_h_hqr_old)
//    {

//                        f_.push_back([=] () {
//                            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                            shallow_water_->set_max_depth(5);
//                            shallow_water_->set_adaptive_mesh(true);
//                            shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//                            shallow_water_->set_iteradapt(1);
//                            shallow_water_->set_seuil_sub_h_q_r_old(sbh, sbqr, sbqr);
//                            shallow_water_->set_seuil_simp_h_q_r_old(smph, smpqr, smpqr);

//                            shallow_water_->init();

//                            render_scalar_->update_min_max(v_, mesh, true);

//                            shallow_water_->start();
//                        });

//    }

    //############################################## seuil subd qr
        //std::vector<SCALAR> seuils_subd_qr_hqr_old = {1.5,1.,0.9,0.8,0.7,0.6};
//        std::vector<SCALAR> seuils_subd_qr_hqr_old = {0.5,0.4,0.3,0.2};
//        SCALAR sbh=0.004;
//        SCALAR smph=0.002;
//        SCALAR smpqr=0.15;


//        for (SCALAR sbqr : seuils_subd_qr_hqr_old)
//        {

//                            f_.push_back([=] () {
//                                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                                shallow_water_->set_max_depth(5);
//                                shallow_water_->set_adaptive_mesh(true);
//                                shallow_water_->set_criteria(plugin_shallow_water_2::H_Q_R_old);
//                                shallow_water_->set_iteradapt(1);
//                                shallow_water_->set_seuil_sub_h_q_r_old(sbh, sbqr, sbqr);
//                                shallow_water_->set_seuil_simp_h_q_r_old(smph, smpqr, smpqr);

//                                shallow_water_->init();

//                                render_scalar_->update_min_max(v_, mesh, true);

//                                shallow_water_->start();
//                            });

//        }
//######################################## h seul ::H critere spatial
//        //std::vector<SCALAR> seuils_subd_h = {0.4,0.3,0.2,0.1,0.09,0.08,0.07,0.06};
//        //std::vector<SCALAR> seuils_subd_h = {0.1,0.07};
//        std::vector<SCALAR> seuils_subd_h= {0.4, 0.4};
//        SCALAR smph=0.05;//0.1;
//        for (SCALAR sbh : seuils_subd_h)
//        {
//                            f_.push_back([=] () {
//                                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                                shallow_water_->set_max_depth(5);
//                                shallow_water_->set_adaptive_mesh(true);
//                                shallow_water_->set_criteria(plugin_shallow_water_2::H);
//                                shallow_water_->set_iteradapt(1);
//                                shallow_water_->set_sigma_sub_h(sbh);
//                                shallow_water_->set_sigma_simp_h(smph);

//                                shallow_water_->init();

//                                render_scalar_->update_min_max(v_, mesh, true);

//                                shallow_water_->start();
//                            });
//        }


//########################################## h seul ::H_old critere temporel

////    std::vector<SCALAR> seuils_subd_h_hqr_old = {0.0084};//0.009,0.008,0.007,0.006,0.0086,0.004,0.003
////      std::vector<SCALAR> seuils_subd_h_hqr_old = {0.013,0.012,0.011,0.010,0.009,0.008,0.007,0.006,0.005,0.004};
////        std::vector<SCALAR> seuils_subd_h_hqr_old={0.003};
//    //    std::vector<SCALAR> seuils_subd_h_hqr_old ={0.013,0.013};//{0.013,0.012,0.011,0.010}; //{0.011,0.010,0.009,0.008};//{0.007,0.006,0.005,0.004,0.003,0.002,0.001,0.0009};
//    SCALAR smph=0.0008;//0.0015
//    for (SCALAR sbh : seuils_subd_h_hqr_old)
//    {
//                        f_.push_back([=] () {
//                            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//                            shallow_water_->set_max_depth(5);
//                            shallow_water_->set_adaptive_mesh(true);
//                            shallow_water_->set_criteria(plugin_shallow_water_2::H_old);
//                            shallow_water_->set_iteradapt(1);
//                            shallow_water_->set_seuil_sub_h_old(sbh);
//                            shallow_water_->set_seuil_simp_h_old(smph);

//                            shallow_water_->init();

//                            render_scalar_->update_min_max(v_, mesh, true);

//                            shallow_water_->start();
//                        });
//    }




//    f_.push_back([=] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
//            shallow_water_->set_max_depth(5);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_criteria(plugin_shallow_water_2::H);
//            shallow_water_->set_iteradapt(1);
//            shallow_water_->set_sigma_sub_h(0.1);
//            shallow_water_->set_sigma_simp_h(0.05);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });

//        f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(5);
//            shallow_water_->set_adaptive_mesh(false);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//==========================================================================================================

/*    f_.push_back([=] () {
        MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");
        shallow_water_->set_max_depth(4);
        shallow_water_->set_adaptive_mesh(true);
        shallow_water_->set_criteria(plugin_shallow_water_2::entropy);
        shallow_water_->set_iteradapt(1);

        shallow_water_->init();

        render_scalar_->update_min_max(v_, mesh, true);

        shallow_water_->start();
    })*/;


//    for(std::vector<SCALAR>::iterator it=seuils_sigma_sub_h.begin();it!=seuils_sigma_sub_h.end();it++)
//    {   std::cout << "it chifaa seuils " << *it << std::endl;
//        f_.push_back([&] () {
//                MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//                shallow_water_->set_max_depth(2);
//                shallow_water_->set_adaptive_mesh(true);
//                shallow_water_->set_sigma_sub_h(*it);
//                shallow_water_->set_sigma_sub_vitesse(1);
//                shallow_water_->set_sigma_simp_h(0.1);
//                shallow_water_->set_sigma_simp_vitesse(0.5);

//                //shallow_water_->set_subd_criteria(H_Q_R);
//                shallow_water_->init();

//                render_scalar_->update_min_max(v_, mesh, true);

//                shallow_water_->start();
//            });
//    }

//    f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(4);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_sigma_sub_h(2);
//            //shallow_water_->set_subd_criteria(H_Q_R);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(4);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_sigma_sub_h(1);
//            //shallow_water_->set_subd_criteria(H_Q_R);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });


//    f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(4);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_sigma_sub_h(0.3);
//            shallow_water_->set_sigma_sub_vitesse(1);
//            shallow_water_->set_sigma_simp_h(0.15);
//            shallow_water_->set_sigma_simp_vitesse(0.1);

//            //shallow_water_->set_subd_criteria(H_Q_R);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(4);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_sigma_sub_h(0.5);
//            shallow_water_->set_sigma_sub_vitesse(1);
//            //shallow_water_->set_subd_criteria(H_Q_R);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });
//    f_.push_back([&] () {
//            MapHandlerGen* mesh = load("/home/dahik/Data/905_Dambreak_Complexe_grossier/Input");

//            shallow_water_->set_max_depth(4);
//            shallow_water_->set_adaptive_mesh(true);
//            shallow_water_->set_sigma_sub_h(0.5);
//            shallow_water_->set_sigma_sub_vitesse(0.5);
//            //shallow_water_->set_subd_criteria(H_Q_R);
//            shallow_water_->init();

//            render_scalar_->update_min_max(v_, mesh, true);

//            shallow_water_->start();
//        });







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
