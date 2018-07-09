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

#include <schnapps/plugins/shallow_water_2/dll.h>

#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/topology/types/adaptive_tri_quad_cmap2.h>

#include <QAction>

namespace schnapps
{

namespace plugin_shallow_water_2
{

class ShallowWater_Dialog;

enum Criteria {
    H_Q_R,
    H,
    Q_R,
    entropy,
    H_old,
    H_Q_R_old,
    H_angle_norm_V, // angle- temporelle
    angle_V // angle spatiale
};

/**
* @brief Shallow water simulation
*/
class SCHNAPPS_PLUGIN_SHALLOW_WATER_2_API Plugin_ShallowWater : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_ShallowWater();
	inline ~Plugin_ShallowWater() override {}
	static QString plugin_name();

	void load_project(const QString& dir);

	bool is_simu_running() { return simu_running_; }
	std::future<void>* simu_future() { return &simu_future_; }

	void set_export_frames(bool b) { export_frames_ = b; }
	void set_export_frames_step(SCALAR s) { export_frames_step_ = s; }
	void set_export_frames_dir(const QString& dir) { export_frames_dir_ = dir;  }

	void set_max_depth(uint32 m) { max_depth_ = m; }
	void set_iteradapt(uint32 i){ iteradapt_ = i; }
    void set_adaptive_mesh(bool b) { adaptive_mesh_ = b; }
    void set_criteria(Criteria c) { criteria_ = c; }

	void set_sigma_sub_h(SCALAR ssb) { sigma_sub_h = ssb; }
	void set_sigma_simp_h(SCALAR ssmp) { sigma_simp_h = ssmp; }

	void set_sigma_sub_vitesse(SCALAR ssb) { sigma_sub_vitesse = ssb; }
	void set_sigma_simp_vitesse(SCALAR ssmp) { sigma_simp_vitesse = ssmp; }

	void set_seuil_sub_h_old(SCALAR ssh) { seuil_sub_h_old = ssh; }
	void set_seuil_simp_h_old(SCALAR ssh) { seuil_simp_h_old = ssh; }

	void set_seuil_sub_h_q_r_old(SCALAR ssh, SCALAR ssq, SCALAR ssr)
	{
		seuil_sub_h_old = ssh;
		seuil_sub_q_old = ssq;
		seuil_sub_r_old = ssr;
	}
	void set_seuil_simp_h_q_r_old(SCALAR ssh, SCALAR ssq, SCALAR ssr)
	{
		seuil_simp_h_old = ssh;
		seuil_simp_q_old = ssq;
		seuil_simp_r_old = ssr;
	}

    void set_seuil_sub_angle_norm(SCALAR ssh, SCALAR ssa, SCALAR ssn)
    {
        seuil_sub_h_old = ssh;
        seuil_sub_angle = ssa;
        seuil_sub_norm = ssn;
    }

    void set_seuil_simp_angle_norm(SCALAR ssh, SCALAR ssa, SCALAR ssn)
    {
        seuil_simp_h_old = ssh;
        seuil_simp_angle = ssa;
        seuil_simp_norm = ssn;
    }

    void set_seuil_sub_angle(SCALAR ssa)
    {
        seuil_sub_angle = ssa;
    }

    void set_seuil_simp_angle(SCALAR ssa)
    {
        seuil_simp_angle = ssa;
    }

    void init();

private:

	bool enable() override;
	void disable() override;

	void clean_map();
	void load_input_file(const QString& filename);
	void load_hydrau_param_file(const QString& filename);
	void load_1D_constrained_edges();
	void load_2D_constrained_edges();
	void load_1D_initial_cond_file(const QString& filename);
	void load_2D_initial_cond_file(const QString& filename);
	void load_1D_boundary_cond_file(const QString& filename);
	void load_2D_boundary_cond_file(const QString& filename);
	void sew_1D_2D_meshes();

public slots:

	void start();
	void stop();
	void step();

signals:

	void save_frame();

private slots:

	// slots called from SCHNApps signals
	void schnapps_closing();

	// slots called from action signals
	void open_dialog();

	void update_draw_data();
	void export_frame();
	void update_time_step();
	void execute_time_step();

private:

	struct Str_Riemann_Flux
	{
		SCALAR F1;  /**< Flux de masse à travers l'interface **/
		SCALAR F2;  /**< Flux de quantité de mouvement à travers l'interface dans la direction normale à l'interface **/
		SCALAR F3;  /**< Flux de quantité de mouvement à travers l'interface dans la direction longitudinale à l'interface **/
		SCALAR s2L; /**< Quantité de mouvement associée well-balancing du terme source pour la maille gauche de l'interface **/
		SCALAR s2R; /**< Quantité de mouvement associée well-balancing du terme source pour la maille droite de l'interface **/
	};

	Str_Riemann_Flux Solv_HLLC(SCALAR g, SCALAR hmin, SCALAR smalll,
							   SCALAR zbL,SCALAR zbR,
							   SCALAR PhiL,SCALAR PhiR,
							   SCALAR hL,SCALAR qL,SCALAR rL,SCALAR hR,SCALAR qR,SCALAR rR);

	Str_Riemann_Flux Solv_PorAS(SCALAR g, SCALAR hmin, SCALAR smalll,
								SCALAR zbL,SCALAR zbR,
								SCALAR PhiL,SCALAR PhiR,
								SCALAR hL,SCALAR qL,SCALAR rL,SCALAR hR,SCALAR qR,SCALAR rR);

	Str_Riemann_Flux border_condition(
			std::string typBC, SCALAR ValBC,
			SCALAR NormX, SCALAR NormY,
			SCALAR q, SCALAR r, SCALAR z, SCALAR zb,
			SCALAR g, SCALAR hmin, SCALAR smalll);

    bool subd_criteria_h_q_r(CMap2::Face f);
    bool subd_criteria_h(CMap2::Face f);

    bool simp_criteria_h_q_r(cgogn::Dart central_cell);
    bool simp_criteria_h(cgogn::Dart central_cell);

    bool subd_criteria_entropy(CMap2::Face f);
    bool simp_criteria_entropy(cgogn::Dart central_cell);

    bool subd_criteria_h_q_r_old(CMap2::Face f);
    bool subd_criteria_h_old(CMap2::Face f);

    bool simp_criteria_h_q_r_old(cgogn::Dart central_cell);
    bool simp_criteria_h_old(cgogn::Dart central_cell);

    bool subd_criteria_h_angle_norm_V(CMap2::Face f);
    bool simp_criteria_h_angle_norm_V(cgogn::Dart central_cell);

    bool subd_criteria_angle_V(CMap2::Face f);
    bool simp_criteria_angle_V(cgogn::Dart central_cell);

    bool subd_criteria_q_r(CMap2::Face f);
    bool simp_criteria_q_r(cgogn::Dart central_cell);

    //    bool verification_thrdDgreeSolve(SCALAR f1,SCALAR f2,SCALAR hL,SCALAR hR);
	SCALAR ThrdDgreeSolve(SCALAR f1, SCALAR f2, SCALAR hL, SCALAR hR);
    SCALAR sign_y(SCALAR y);

	void try_subdivision();
	void try_simplification();

	void get_LR_faces(CMap2::Edge e, CMap2::Face& fl, CMap2::Face& fr);

	bool almost_equal(VEC3 v1, VEC3 v2);
	bool are_points_aligned(VEC3 p1, VEC3 p2, VEC3 p3); // check if the point p is in the line through points l1 and l2
	bool is_point_in_segment(VEC3 A, VEC3 B, VEC3 C); // check if the point C is in the segment [A,B]
	bool sew_faces_recursive(CMap2::Edge e1, CMap2::Edge e2);

	void compute_edge_length_and_normal(CMap2::Edge e);

	ShallowWater_Dialog* shallow_water_dialog_;
	QAction* shallow_water_action;

	QString project_dir_;
	QString mesh_1D_filename_;
	QString mesh_2D_filename_;

	uint8 dim_;

	SCALAR t_;
	SCALAR t_max_;
	SCALAR dt_max_;

	SCALAR dt_;

	uint8 friction_;
	SCALAR v_max_;
	SCALAR Fr_max_;

	SCALAR phi_default_;
	SCALAR kx_;
	SCALAR ky_;
	SCALAR alphaK_;

	SCALAR hmin_;
	SCALAR small_;
	uint8 solver_;
	uint32 nb_iter_;

	uint32 max_depth_;
	uint32 iteradapt_;
    bool adaptive_mesh_;
    Criteria criteria_;

	//chifaa
    bool sup10; // un booleen pour recuper des données au premier temps t superieur a 10

    SCALAR sigma_sub_h;
    SCALAR sigma_simp_h;
    SCALAR sigma_sub_vitesse;
    SCALAR sigma_simp_vitesse;

	SCALAR seuil_sub_h_old;
    SCALAR seuil_simp_h_old;
    SCALAR seuil_sub_q_old;
    SCALAR seuil_simp_q_old;
    SCALAR seuil_sub_r_old;
    SCALAR seuil_simp_r_old;

    SCALAR seuil_sub_angle;
    SCALAR seuil_sub_norm;

    SCALAR seuil_simp_angle;
    SCALAR seuil_simp_norm;

	uint32 nbmailles;
	uint64 somme_nb_mailles;
    //end chifaa

	QTimer* draw_timer_;
	std::chrono::high_resolution_clock::time_point start_time_;
	std::future<void> simu_future_;
	std::atomic_bool simu_running_;
	std::mutex simu_data_access_;

	bool export_frames_;
	QString export_frames_dir_;
	SCALAR export_frames_step_;
	SCALAR next_frame_t_;
	uint32 frame_num_;

	CMap2Handler* map_;
	CMap2* map2_;
	cgogn::AdaptiveTriQuadCMap2* atq_map_;
	CMap2::QuickTraversor* qtrav_;
	CMap2::DartMarker* edge_left_side_;

	uint32 nb_faces_2d_;
	uint32 nb_vertices_2d_;

	CMap2::VertexAttribute<VEC3> position_; // vertices position
	CMap2::VertexAttribute<SCALAR> scalar_value_h_;
	CMap2::VertexAttribute<SCALAR> scalar_value_u_;
	CMap2::VertexAttribute<SCALAR> scalar_value_v_;
	CMap2::VertexAttribute<VEC3> water_position_;
	CMap2::VertexAttribute<VEC3> flow_velocity_;

	CMap2::FaceAttribute<SCALAR> phi_; // porosité
	CMap2::FaceAttribute<SCALAR> zb_; // cote du fond
	CMap2::FaceAttribute<SCALAR> h_; // hauteur d'eau
    CMap2::FaceAttribute<SCALAR> h_old_; // hauteur d'eau
	CMap2::FaceAttribute<SCALAR> q_; // flux de quantité de mouvement dans la direction X
    CMap2::FaceAttribute<SCALAR> q_old_; // flux de quantité de mouvement dans la direction X
    CMap2::FaceAttribute<SCALAR> r_; // flux de quantité de mouvement dans la direction Y
    CMap2::FaceAttribute<SCALAR> r_old_; // flux de quantité de mouvement dans la direction Y
    //chifaa
    CMap2::FaceAttribute<SCALAR> s_entropy_;
    CMap2::FaceAttribute<SCALAR> Snk_;
    SCALAR entropy_global_;
    SCALAR max_entropy_;
    SCALAR area_global_;
	//end chifaa
	CMap2::FaceAttribute<VEC3> centroid_; // cell centroid
	CMap2::FaceAttribute<SCALAR> area_; // cell area
	CMap2::FaceAttribute<SCALAR> swept_;
	CMap2::FaceAttribute<SCALAR> discharge_;
	CMap2::FaceAttribute<uint8> dimension_;

	CMap2::EdgeAttribute<SCALAR> f1_;
	CMap2::EdgeAttribute<SCALAR> f2_;
	CMap2::EdgeAttribute<SCALAR> f3_;
	CMap2::EdgeAttribute<SCALAR> s2L_;
	CMap2::EdgeAttribute<SCALAR> s2R_;
	CMap2::EdgeAttribute<SCALAR> normX_;
	CMap2::EdgeAttribute<SCALAR> normY_;
	CMap2::EdgeAttribute<SCALAR> length_;
	CMap2::EdgeAttribute<SCALAR> val_bc_;
	CMap2::EdgeAttribute<std::string> typ_bc_;
	CMap2::EdgeAttribute<uint32> NS_;
    //chifaa
    CMap2::EdgeAttribute<SCALAR> psi_entropy_x_;
	CMap2::EdgeAttribute<SCALAR> psi_entropy_y_;
    // end chifaa

    //chifaa
	std::vector<SCALAR> tempschifaa;
    std::vector<uint32> vect_nbmailles_chifaa;

	std::map<
		uint32, // vertex id
        std::tuple<std::vector<SCALAR>, std::vector<SCALAR>, std::vector<SCALAR>, std::vector<SCALAR>> // h, q, r, entropy values
	> logged_values_;

    std::map<
        uint32, // vertex id
        std::tuple<SCALAR, SCALAR, SCALAR, SCALAR> // h, q, r, entropy values,x,y
    > logged_values_all_;
//    std::vector<SCALAR> entropy_490chifaa;
//    std::vector<SCALAR> entropy_301chifaa;
//    std::vector<SCALAR> entropy_9chifaa;
//    std::vector<SCALAR> entropy_69chifaa;
//    std::vector<SCALAR> entropy_342chifaa;
//    std::vector<SCALAR> entropy_295chifaa;
    std::vector<SCALAR> max_entropy_tempschifaa;
    std::vector<SCALAR> global_entropy_tempschifaa;
};

} // namespace plugin_shallow_water_2

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_2_H_
