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

#ifndef SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_
#define SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_

#include "dll.h"
#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>

#include <shallow_water_dock_tab.h>

namespace schnapps
{

namespace plugin_shallow_water_2
{

/**
* @brief Shallow water simulation
*/
class SCHNAPPS_PLUGIN_SHALLOW_WATER_2_API Plugin_ShallowWater : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_ShallowWater() {}
	~Plugin_ShallowWater() override {}

private:

	bool enable() override;
	void disable() override;

public slots:

	void init();
	void start();
	void stop();
	bool is_simu_running();

private slots:

	void update_draw_data();
	void update_time_step();
	void execute_time_step();

private:

    struct Str_Riemann_Flux
    {
        SCALAR F1;      /**< Flux de masse à travers l'interface **/
        SCALAR F2;      /**< Flux de quantité de mouvement à travers l'interface dans la direction normale à l'interface **/
        SCALAR F3;      /**< Flux de quantité de mouvement à travers l'interface dans la direction longitudinale à l'interface **/
        SCALAR s2L;     /**< Quantité de mouvement associée well-balancing du terme source pour la maille gauche de l'interface **/
        SCALAR s2R;     /**< Quantité de mouvement associée well-balancing du terme source pour la maille droite de l'interface **/
    };

    Str_Riemann_Flux Solv_HLLC(SCALAR g, SCALAR hmin, SCALAR small,
                               SCALAR zbL,SCALAR zbR,
                               SCALAR PhiL,SCALAR PhiR,
                               SCALAR hL,SCALAR qL,SCALAR rL,SCALAR hR,SCALAR qR,SCALAR rR);
    Str_Riemann_Flux Solv_PorAS(SCALAR g, SCALAR hmin, SCALAR small,
                               SCALAR zbL,SCALAR zbR,
                               SCALAR PhiL,SCALAR PhiR,
                               SCALAR hL,SCALAR qL,SCALAR rL,SCALAR hR,SCALAR qR,SCALAR rR);

	enum FaceType: uint8
	{
		TRI_CORNER = 0,
		TRI_CENTRAL,
		QUAD
	};

	void try_subdivision();
	void try_simplification();
	void subdivide_face(CMap2::Face f, CMap2::CellMarker<CMap2::Face::ORBIT>& subdivided);
	void simplify_face(CMap2::Face f);        

	cgogn::Dart oldest_dart(CMap2::Face f);
	uint8 face_level(CMap2::Face f);
	FaceType face_type(CMap2::Face f);

    /*Str_Riemann_Flux border_condition(int border_condition_choice, SCALAR val_bc, bool sideR,
                          SCALAR normX, SCALAR normY,
                          SCALAR q, SCALAR r, SCALAR z, SCALAR zB);*/
    struct Str_Riemann_Flux border_condition(
            char* typBC,SCALAR ValBC,char* side,
                            SCALAR NormX,SCALAR NormY,
                            SCALAR q,SCALAR r,SCALAR z,SCALAR zb,
                            SCALAR g, SCALAR hmin, SCALAR small);

    void get_LR_faces(CMap2::Edge e, CMap2::Face& fl, CMap2::Face& fr);
    char boundary_side(CMap2::Edge e);

    SCALAR min_0(SCALAR a, SCALAR b);
    SCALAR max_0(SCALAR a, SCALAR b);
    SCALAR min_1(SCALAR a, SCALAR b, SCALAR c);
    SCALAR max_1(SCALAR a, SCALAR b, SCALAR c);

	ShallowWater_DockTab* dock_tab_;

	SCALAR t_;
	SCALAR dt_;
    SCALAR hmin_;
    SCALAR small_;
    uint8 solver_;
    SCALAR v_max_;
    SCALAR Fr_max_;
    SCALAR t_max_;
    SCALAR dt_max_;
    uint8 friction_;
    SCALAR alphaK_;
    SCALAR kx_;
    uint32 nbr_iter_;

	QTimer* draw_timer_;
	std::chrono::high_resolution_clock::time_point start_time_;
	std::future<void> simu_future_;
	std::atomic_bool simu_running_;
	std::mutex simu_data_access_;

	CMap2Handler* map_;
	CMap2* map2_;
	std::unique_ptr<CMap2::QuickTraversor> qtrav_;                

	CMap2::VertexAttribute<VEC3> position_; // vertices position
    CMap2::VertexAttribute<SCALAR> scalar_value_h_;
    CMap2::VertexAttribute<SCALAR> scalar_value_u_;
    CMap2::VertexAttribute<SCALAR> scalar_value_v_;
    CMap2::VertexAttribute<VEC3> water_position_;
    CMap2::VertexAttribute<VEC3> flow_velocity_;
    CMap2::VertexAttribute<uint32> NS_;
    CMap2::VertexAttribute<uint32> vertex_id_;

    CMap2::CDartAttribute<uint8> dart_level_; // dart insertion level

	CMap2::FaceAttribute<uint32> face_subd_id_; // face subdivision id
	CMap2::FaceAttribute<bool> tri_face_; // face is triangle or not
    CMap2::FaceAttribute<SCALAR> phi_; // porosité
    CMap2::FaceAttribute<SCALAR> zb_; // cote du fond
    CMap2::FaceAttribute<SCALAR> h_; // hauteur d'eau
    CMap2::FaceAttribute<SCALAR> q_; // flux de quantité de mouvement dans la direction X
    CMap2::FaceAttribute<SCALAR> r_; // flux de quantité de mouvement dans la direction Y
    CMap2::FaceAttribute<VEC3> centroid_;   // cell centroid
    CMap2::FaceAttribute<SCALAR> area_; //cell area
    CMap2::FaceAttribute<SCALAR> swept_;
    CMap2::FaceAttribute<SCALAR> discharge_;
    CMap2::FaceAttribute<uint32> face_id_;

    CMap2::EdgeAttribute<SCALAR> f1_;
    CMap2::EdgeAttribute<SCALAR> f2_;
    CMap2::EdgeAttribute<SCALAR> f3_;
    CMap2::EdgeAttribute<SCALAR> s2L_;
    CMap2::EdgeAttribute<SCALAR> s2R_;
    CMap2::EdgeAttribute<SCALAR> normX_;
    CMap2::EdgeAttribute<SCALAR> normY_;
    CMap2::EdgeAttribute<SCALAR> length_;
    CMap2::EdgeAttribute<SCALAR> val_bc_;
    CMap2::EdgeAttribute<char> typ_bc_;

};

} // namespace plugin_shallow_water_2

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_
