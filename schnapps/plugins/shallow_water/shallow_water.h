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

#ifndef SCHNAPPS_PLUGIN_SHALLOW_WATER_H_
#define SCHNAPPS_PLUGIN_SHALLOW_WATER_H_

#include "dll.h"
#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>

#include <shallow_water_dock_tab.h>

#include <chrono>

namespace schnapps
{

namespace plugin_shallow_water
{

/**
* @brief Shallow water simulation
*/
class SCHNAPPS_PLUGIN_SHALLOW_WATER_API Plugin_ShallowWater : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_ShallowWater() :
		simu_running_(false)
	{}
	~Plugin_ShallowWater() override {}

private:

	bool enable() override;
	void disable() override;

public slots:

	void init();
	void start();
	void stop();
	void step();
	bool is_simu_running();

private slots:

	void update_draw_data();
	void update_time_step();
	void execute_time_step();

	void try_subdivision();
	void try_simplification();
	void subdivide_face(CMap2::Face f, CMap2::CellMarker<CMap2::Face::ORBIT>& subdivided);
	void remove_edge(CMap2::Edge e);

    void exact_solution_constant_calcul();
    void exact_solution(SCALAR x, SCALAR& h, SCALAR& u);
    void difference_measure();
    float parameters();

private:

	struct Flux
	{
		SCALAR F1;
		SCALAR F2;
		SCALAR S0L;
		SCALAR S0R;
	};

	struct Flux Solv_HLL(
		SCALAR zbL, SCALAR zbR,
		SCALAR PhiL, SCALAR PhiR,
		SCALAR hL, SCALAR hR,
		SCALAR qL, SCALAR qR,
		SCALAR hmin, SCALAR g
	);    

	ShallowWater_DockTab* dock_tab_;

	SCALAR t_;
	SCALAR dt_;

	QTimer* draw_timer_;
	std::chrono::high_resolution_clock::time_point start_time_;
	std::future<void> simu_future_;
	std::atomic_bool simu_running_;
	std::mutex simu_data_access_;

    SCALAR initial_right_water_position_;
    SCALAR initial_left_water_position_;
    SCALAR initial_right_flow_velocity_;
    SCALAR initial_left_flow_velocity_;
    SCALAR error_h_2_;
    SCALAR error_u_2_;
    SCALAR error_h_max_;
    SCALAR error_u_max_;
    SCALAR h_exact_solution_;
    SCALAR u_exact_solution_;
    SCALAR h_difference_;
    SCALAR q_difference_;
    unsigned int nbr_cell_;
    unsigned int nbr_time_step_;

    SCALAR dt_max_;
    clock_t t_begin_, t_end_;

	CMap2Handler* map_;
	CMap2* map2_;
	CMap2::Edge boundaryL_, boundaryR_;
	std::unique_ptr<CMap2::QuickTraversor> qtrav_;

    CMap2::VertexAttribute<VEC3> position_; // vertices position
    CMap2::VertexAttribute<VEC3> water_position_;
    CMap2::VertexAttribute<SCALAR> scalar_value_water_position_;
    CMap2::VertexAttribute<SCALAR> scalar_value_flow_velocity_;
    CMap2::VertexAttribute<VEC3> flow_velocity_;

	CMap2::FaceAttribute<SCALAR> h_;        // water height
	CMap2::FaceAttribute<SCALAR> h_tmp_;
	CMap2::FaceAttribute<SCALAR> q_;        // water flow
	CMap2::FaceAttribute<SCALAR> q_tmp_;

	CMap2::FaceAttribute<VEC3> centroid_;   // cell centroid
    CMap2::FaceAttribute<SCALAR> length_;   // cell length
    CMap2::FaceAttribute<SCALAR> phi_;      // cell width

	CMap2::FaceAttribute<uint32> subd_code_;// subdivision code

    CMap2::FaceAttribute<SCALAR> error_h_;
    CMap2::FaceAttribute<SCALAR> error_u_;

	CMap2::EdgeAttribute<SCALAR> f1_;
	CMap2::EdgeAttribute<SCALAR> f2_;
	CMap2::EdgeAttribute<SCALAR> s0L_;
	CMap2::EdgeAttribute<SCALAR> s0R_;
};

} // namespace plugin_shallow_water

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_H_
