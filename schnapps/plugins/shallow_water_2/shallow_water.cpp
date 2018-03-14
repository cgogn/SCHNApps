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

#include <schnapps/plugins/shallow_water_2/shallow_water.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>
#include <schnapps/core/camera.h>

#include <cgogn/modeling/tiling/triangular_grid.h>
#include <cgogn/modeling/tiling/square_grid.h>

#include <cgogn/geometry/algos/centroid.h>
#include <cgogn/geometry/algos/length.h>
#include <cgogn/geometry/algos/area.h>

#include <cgogn/io/io_utils.h>
#include <cgogn/io/map_import.h>

#include <cgogn/core/utils/thread.h>

#include <QFileInfo>

namespace schnapps
{

namespace plugin_shallow_water_2
{

Plugin_ShallowWater::Plugin_ShallowWater() :
	map_(nullptr),
	map2_(nullptr),
	atq_map_(nullptr),
	qtrav_(nullptr),
	edge_left_side_(nullptr),
	max_depth_(0),
	hmin_(1e-3), // Valeur minimale du niveau d'eau pour laquelle une maille est considérée comme non vide
	small_(1e-35) // Valeur minimale en deça de laquelle les valeurs sont considérées comme nulles
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_ShallowWater::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_ShallowWater::enable()
{
	shallow_water_dialog_ = new ShallowWater_Dialog(this->schnapps_, this);

	shallow_water_action = schnapps_->add_menu_action("Simulation;Shallow Water;Open dialog", "shallow water simulation");
	connect(shallow_water_action, SIGNAL(triggered()), this, SLOT(open_dialog()));

	draw_timer_ = new QTimer(this);
	connect(draw_timer_, SIGNAL(timeout()), this, SLOT(update_draw_data()));

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	open_dialog();

	return true;
}

void Plugin_ShallowWater::disable()
{
	disconnect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(schnapps_closing()));

	disconnect(shallow_water_action, SIGNAL(triggered()), this, SLOT(open_dialog()));
	schnapps_->remove_menu_action(shallow_water_action);

	delete draw_timer_;
	delete shallow_water_dialog_;
}

void Plugin_ShallowWater::clean_map()
{
	delete qtrav_;
	qtrav_ = nullptr;
	delete edge_left_side_;
	edge_left_side_ = nullptr;
	delete atq_map_;
	atq_map_ = nullptr;
	schnapps_->remove_map("shallow_water_2");
	map_ = nullptr;
	map2_ = nullptr;
}

void Plugin_ShallowWater::load_input_file(const QString& filename)
{
	std::ifstream input_file(filename.toStdString(), std::ios::in); // open input file
	std::string str;

	cgogn::io::getline_safe(input_file, str); // read line "Input parameters for 2D simulation-version_2"

	cgogn::io::getline_safe(input_file, str); // read line "==== Files & Folders"

	cgogn::io::getline_safe(input_file, str, ':'); // read "1D Mesh (2dm):"
	cgogn::io::getline_safe(input_file, str, ' '); // read space after :
	cgogn::io::getline_safe(input_file, str); // read name of the 1D mesh file
	mesh_1D_filename_ = QString::fromStdString(str);

	cgogn::io::getline_safe(input_file, str, ':'); // read "2D Mesh (2dm):"
	cgogn::io::getline_safe(input_file, str, ' '); // read space after :
	cgogn::io::getline_safe(input_file, str); // read name of the 2D mesh file
	mesh_2D_filename_ = QString::fromStdString(str);

	cgogn::io::getline_safe(input_file, str); // read line "==== Physical Phenomenon"

	cgogn::io::getline_safe(input_file, str, '\t'); // read "Friction"
	cgogn::io::getline_safe(input_file, str); // read value of friction
	friction_ = std::stoi(str);

	cgogn::io::getline_safe(input_file, str); // read line "==== Numerical Parameters"

	cgogn::io::getline_safe(input_file, str, '\t'); // read "Solver"
	cgogn::io::getline_safe(input_file, str); // read value of solver
	solver_ = std::stoi(str);

	cgogn::io::getline_safe(input_file, str); // read line "Parallel	0"

	cgogn::io::getline_safe(input_file, str); // read line "T0	Tmax	Dtmax	DtStore	NbStepMax"

	input_file >> t_; // read value of T0
	input_file >> t_max_; // read value of Tmax
	input_file >> dt_max_; // read value of Dtmax
	cgogn::io::getline_safe(input_file, str); // read the end of the line

	cgogn::io::getline_safe(input_file, str); // read line "Eps"
	cgogn::io::getline_safe(input_file, str); // read the value of Eps

	cgogn::io::getline_safe(input_file, str); // read line "==== Hydraulic Parameters"

	cgogn::io::getline_safe(input_file, str); // read line "Vmax	Frmax"
	input_file >> v_max_; // read value of Vmax
	input_file >> Fr_max_; // read value of Frmax

	input_file.close(); // close input file
}

void Plugin_ShallowWater::load_hydrau_param_file(const QString& filename)
{
	std::ifstream file_hydrau(filename.toStdString(), std::ios::in);
	std::string str;

	cgogn::io::getline_safe(file_hydrau, str); // read line "Unif	1"
	cgogn::io::getline_safe(file_hydrau, str); // read line "==== Default Param"
	cgogn::io::getline_safe(file_hydrau, str); // read line "Phi	Kx	Ky	Alpha"
	file_hydrau >> phi_default_; // read the value of Phi
	file_hydrau >> kx_; // read the value of Kx
	file_hydrau >> ky_; // read the value of Ky
	file_hydrau >> alphaK_; // read the value of Alpha

	file_hydrau.close(); // close file "Hydrau_Param_2D.txt"
}

void Plugin_ShallowWater::load_1D_constrained_edges()
{
	// open 1D mesh file to read the NS part
	QString mesh_1D_filepath = project_dir_ + "/" + mesh_1D_filename_;
	std::ifstream file_mesh_1D(mesh_1D_filepath.toStdString(), std::ios::in);
	std::string str, tag;

	do
	{
		file_mesh_1D >> tag;
		cgogn::io::getline_safe(file_mesh_1D, str);
	} while (tag != std::string("NS") && (!file_mesh_1D.eof()));

	if (tag == std::string("NS"))
	{
		do
		{
			if (tag == std::string("NS")) // lecture d'une face
			{
				std::stringstream oss(str);
				uint32 vertex_id_int = 0;
				std::vector<uint32> vertex_id_vect;
				while (oss >> vertex_id_int)
					vertex_id_vect.push_back(vertex_id_int);

				uint32 ns_num = vertex_id_vect.back();
				vertex_id_vect.pop_back();
				vertex_id_vect.back() *= -1;

				for (uint32 i = 0; i < vertex_id_vect.size() - 1; ++i)
				{
					CMap2::Vertex v = qtrav_->cell_from_index<CMap2::Vertex>(vertex_id_vect[i] + nb_vertices_2d_ - 1);
					bool ns_edge_found = false;
					map2_->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) -> bool
					{
						if (map2_->embedding(av) == vertex_id_vect[i+1] + nb_vertices_2d_ - 1)
						{
							NS_[CMap2::Edge(av.dart)] = ns_num;
							ns_edge_found = true;
						}
						return !ns_edge_found;
					});
					if (!ns_edge_found)
						std::cout << "WARNING: NS edge not found 1D " << vertex_id_vect[i] << "\t" << vertex_id_vect[i+1] << std::endl;
				}
			}

			file_mesh_1D >> tag;
			cgogn::io::getline_safe(file_mesh_1D, str);
		} while (!file_mesh_1D.eof());
	}
	file_mesh_1D.close(); // close 1D mesh file
}

void Plugin_ShallowWater::load_2D_constrained_edges()
{
	// open 2D mesh file to read the NS part
	QString mesh_2D_filepath = project_dir_ + "/" + mesh_2D_filename_;
	std::ifstream file_mesh_2D(mesh_2D_filepath.toStdString(), std::ios::in);
	std::string str, tag;

	do
	{
		file_mesh_2D >> tag;
		cgogn::io::getline_safe(file_mesh_2D, str);
	} while (tag != std::string("NS") && (!file_mesh_2D.eof()));

	if (tag == std::string("NS"))
	{
		do
		{
			if (tag == std::string("NS"))
			{
				std::stringstream oss(str);
				uint32 vertex_id_int = 0;
				std::vector<uint32> vertex_id_vect;
				while (oss >> vertex_id_int)
					vertex_id_vect.push_back(vertex_id_int);

				uint32 ns_num = vertex_id_vect.back();
				vertex_id_vect.pop_back();
				vertex_id_vect.back() *= -1;

				for (uint32 i = 0; i < vertex_id_vect.size() - 1; ++i)
				{
					CMap2::Vertex v = qtrav_->cell_from_index<CMap2::Vertex>(vertex_id_vect[i] - 1);
					bool ns_edge_found = false;
					map2_->foreach_adjacent_vertex_through_edge(v, [&] (CMap2::Vertex av) -> bool
					{
						if (map2_->embedding(av) == vertex_id_vect[i+1] - 1)
						{
							NS_[CMap2::Edge(av.dart)] = ns_num;
							ns_edge_found = true;
						}
						return !ns_edge_found;
					});
					if (!ns_edge_found)
						std::cout << "WARNING: NS edge not found 2D " << vertex_id_vect[i] << "\t" << vertex_id_vect[i+1] << std::endl;
				}
			}

			file_mesh_2D >> tag;
			cgogn::io::getline_safe(file_mesh_2D, str);
		} while (!file_mesh_2D.eof());
	}
	file_mesh_2D.close(); // close 2D mesh file
}

void Plugin_ShallowWater::load_2D_initial_cond_file(const QString& filename)
{
	std::ifstream initial_cond_file(filename.toStdString(), std::ios::in); // open initial cond file
	std::string str;

	cgogn::io::getline_safe(initial_cond_file, str); // read line "Unif	0"
	cgogn::io::getline_safe(initial_cond_file, str); // read line "==== Default Param"
	cgogn::io::getline_safe(initial_cond_file, str); // read line "z  u	v"

	SCALAR z;
	initial_cond_file >> z; // read default param z
	h_.set_all_values(z);

	SCALAR u;
	initial_cond_file >> u; // read default param u
	q_.set_all_values(z * u);

	SCALAR v;
	initial_cond_file >> v;  // read default param v
	r_.set_all_values(z * v);

	cgogn::io::getline_safe(initial_cond_file, str); // read the end line character

	cgogn::io::getline_safe(initial_cond_file, str); // read line "==== Distrib"

	uint32 line = 1; // line number
	// each face has a face_id that matches the line number of the file
	cgogn::io::getline_safe(initial_cond_file, str);
	do
	{
		std::stringstream oss(str);
		CMap2::Face f = qtrav_->cell_from_index<CMap2::Face>(line - 1);
		oss >> h_[f];
		oss >> q_[f];
		oss >> r_[f];
		cgogn::io::getline_safe(initial_cond_file, str);
		++line;
	} while (!initial_cond_file.eof());

	initial_cond_file.close(); // close initial cond file
}

void Plugin_ShallowWater::load_1D_initial_cond_file(const QString& filename)
{
	std::ifstream initial_cond_file(filename.toStdString(), std::ios::in); // open initial cond file
	std::string str;

	cgogn::io::getline_safe(initial_cond_file, str); // read line "Unif	0"
	cgogn::io::getline_safe(initial_cond_file, str); // read line "==== Default Param"
	cgogn::io::getline_safe(initial_cond_file, str); // read line "z  u	v"

	cgogn::io::getline_safe(initial_cond_file, str); // read (and skip) default param values
	cgogn::io::getline_safe(initial_cond_file, str); // read line "==== Distrib"

	uint32 line = nb_faces_2d_; // line number
	// each face has a face_id that matches the line number of the file
	cgogn::io::getline_safe(initial_cond_file, str);
	do
	{
		std::stringstream oss(str);
		CMap2::Face f = qtrav_->cell_from_index<CMap2::Face>(line - 1);
		oss >> h_[f];
		oss >> q_[f];
		oss >> r_[f];
		cgogn::io::getline_safe(initial_cond_file, str);
		++line;
	} while (!initial_cond_file.eof());

	initial_cond_file.close(); // close initial cond file
}

void Plugin_ShallowWater::load_2D_boundary_cond_file(const QString& filename)
{
	std::ifstream boundary_cond_file(filename.toStdString(), std::ios::in); // open boundary cond file
	std::string str;

	cgogn::io::getline_safe(boundary_cond_file, str); // read line "2D BC time series"
	cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read "number_of_BC"
	uint32 number_of_BC;
	boundary_cond_file >> number_of_BC; // read the number of boundary conditions
	cgogn::io::getline_safe(boundary_cond_file, str); // read the end line character

	cgogn::io::getline_safe(boundary_cond_file, str); // read line "Number_of_values_in_the_time_series" (only the first time serie is used)
	cgogn::io::getline_safe(boundary_cond_file, str); // read line "========================="

	// for NS, Type and Value (only one value instead of Number_of_values_in_the_time_series values),
	// there is number_of_BC int, char or SCALAR to store
	// the last column is not read like the others because it ends with a new line instead of a tab
	if (number_of_BC > 0)
	{
		std::vector<uint32> tab_ns(number_of_BC);
		cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read "NS"

		for (uint32 i = 0; i < number_of_BC - 1; ++i)
		{
			cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read the less than 0 nodestring
			tab_ns[i] = - std::stoi(str); // convert string to int and change sign
		}
		// last column
		cgogn::io::getline_safe(boundary_cond_file, str);
		tab_ns[number_of_BC - 1] = - std::stoi(str);

		std::vector<std::string> tab_type(number_of_BC);
		cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read "Type"
		for (uint32 i = 0; i < number_of_BC - 1; ++i)
		{
			cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read char
			tab_type[i] = str;
		}
		// last column
		cgogn::io::getline_safe(boundary_cond_file, str);
		tab_type[number_of_BC - 1] = str;

		cgogn::io::getline_safe(boundary_cond_file, str); // read line "========================="
		cgogn::io::getline_safe(boundary_cond_file, str); // read line "Time	Value"

		std::vector<SCALAR> tab_value(number_of_BC);
		cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read time
		for (uint32 i = 0; i < number_of_BC - 1; ++i)
		{
			cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read value
			tab_value[i] = std::stod(str); // convert string to SCALAR
		}
		// last column
		cgogn::io::getline_safe(boundary_cond_file, str); // read value
		tab_value[number_of_BC - 1] = std::stod(str); // convert string to SCALAR

		map2_->parallel_foreach_cell(
			[&] (CMap2::Edge e)
			{
				if (!map2_->is_incident_to_boundary(e))
					return;

				if (NS_[e] == 0)
				{
					typ_bc_[e] = "q";
					val_bc_[e] = 0.;
				}
				else
				{
					typ_bc_[e] = "m2"; // if the edge has a nodestring without value, it will be merged with 1D mesh
					val_bc_[e] = 0.;
					for (uint32 i = 0; i < number_of_BC; ++i)
					{
						if (NS_[e] == tab_ns[i])
						{
							typ_bc_[e] = tab_type[i];
							val_bc_[e] = tab_value[i];
						}
					}
				}
			},
			*qtrav_
		);
	}
	else
	{
		map2_->parallel_foreach_cell(
			[&] (CMap2::Edge e)
			{
				if (!map2_->is_incident_to_boundary(e))
					return;

				if (NS_[e] == 0)
				{
					typ_bc_[e] = "q";
					val_bc_[e] = 0.;
				}
				else
				{
					typ_bc_[e] = "m2"; // if the edge has a nodestring without value, it will be merged with 1D mesh
					val_bc_[e] = 0.;
				}
			},
			*qtrav_
		);
	}
	boundary_cond_file.close(); // close boundary cond file
}

void Plugin_ShallowWater::load_1D_boundary_cond_file(const QString& filename)
{
	std::ifstream boundary_cond_file(filename.toStdString(), std::ios::in); // open boundary cond file
	std::string str;

	cgogn::io::getline_safe(boundary_cond_file, str); // read line "2D BC time series"
	cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read "number_of_BC"
	uint32 number_of_BC;
	boundary_cond_file >> number_of_BC; // read the number of boundary conditions

	cgogn::io::getline_safe(boundary_cond_file, str); // read line "Number_of_values_in_the_time_series" (only the first time serie is used)
	cgogn::io::getline_safe(boundary_cond_file, str); // read line "========================="

	// for NS, Type and Value (only one value instead of Number_of_values_in_the_time_series values),
	// there is number_of_BC int, char or SCALAR to store
	// the last column is not read like the others because it ends with a new line instead of a tab
	if (number_of_BC > 0)
	{
		std::vector<uint32> tab_ns(number_of_BC);
		cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read "NS"

		for (uint32 i = 0; i < number_of_BC - 1; ++i)
		{
			cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read the less than 0 nodestring
			tab_ns[i] = - std::stoi(str); // convert string to int and change sign
		}
		// last column
		cgogn::io::getline_safe(boundary_cond_file, str);
		tab_ns[number_of_BC - 1] = - std::stoi(str);

		std::vector<std::string> tab_type(number_of_BC);
		cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read "Type"
		for (uint32 i = 0; i < number_of_BC - 1; ++i)
		{
			cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read char
			tab_type[i] = str;
		}
		// last column
		cgogn::io::getline_safe(boundary_cond_file, str);
		tab_type[number_of_BC - 1] = str;

		cgogn::io::getline_safe(boundary_cond_file, str); // read line "========================="
		cgogn::io::getline_safe(boundary_cond_file, str); // read line "Time	Value"

		std::vector<SCALAR> tab_value(number_of_BC);
		cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read time
		for (uint32 i = 0; i < number_of_BC - 1; ++i)
		{
			cgogn::io::getline_safe(boundary_cond_file, str, '\t'); // read value
			tab_value[i] = std::stod(str.c_str()); // convert string to SCALAR
		}
		// last column
		cgogn::io::getline_safe(boundary_cond_file, str); // read value
		tab_value[number_of_BC - 1] = std::stod(str); // convert string to SCALAR

		map2_->parallel_foreach_cell(
			[&] (CMap2::Edge e)
			{
				if (!map2_->is_incident_to_boundary(e))
					return;

				if (dimension_[CMap2::Face(e.dart)] == 2)
					return;

				if (NS_[e] == 0)
				{
					typ_bc_[e] = "q";
					val_bc_[e] = 0.;
				}
				else
				{
					typ_bc_[e] = "m1"; // if the edge has a nodestring without value, it will be merged with 2D mesh
					val_bc_[e] = 0.;
					for (uint32 i = 0; i < number_of_BC; ++i)
					{
						if (NS_[e] == tab_ns[i])
						{
							typ_bc_[e] = tab_type[i];
							val_bc_[e] = tab_value[i];
						}
					}
				}
			},
			*qtrav_
		);
	}
	else
	{
		map2_->parallel_foreach_cell(
			[&] (CMap2::Edge e)
			{
				if (!map2_->is_incident_to_boundary(e))
					return;

				if (dimension_[CMap2::Face(e.dart)] == 2)
					return;

				if (NS_[e] == 0)
				{
					typ_bc_[e] = "q";
					val_bc_[e] = 0.;
				}
				else
				{
					typ_bc_[e] = "m1"; // if the edge has a nodestring without value, it will be merged with 2D mesh
					val_bc_[e] = 0.;
				}
			},
			*qtrav_
		);
	}
	boundary_cond_file.close(); // close boundary cond file
}

void Plugin_ShallowWater::sew_1D_2D_meshes()
{
	map2_->foreach_cell(
		[&] (CMap2::Edge e1)
		{
			if (typ_bc_[e1].compare("m1") != 0)
				return;

			map2_->foreach_cell([&] (CMap2::Edge e2) -> bool
			{
				if (typ_bc_[e2].compare("m2") != 0)
					return true;
				return sew_faces_recursive(e1, e2);
			});
		}
	);
}

void Plugin_ShallowWater::load_project(const QString& dir)
{
	// clean
	if (map_)
		clean_map();

	project_dir_ = dir;

	load_input_file(dir + "/2d.in");
	load_hydrau_param_file(dir + "/Hydrau_Param_2D.txt");

	if (mesh_1D_filename_ == "None")
		dim_ = 2;
	else
		dim_ = 12;

	// create map
	map_ = static_cast<CMap2Handler*>(schnapps_->add_map("shallow_water_2", 2));
	map2_ = static_cast<CMap2*>(map_->get_map());
	qtrav_ = new CMap2::QuickTraversor(*map2_);
	edge_left_side_ = new CMap2::DartMarker(*map2_);

	// import 2D mesh
	QString mesh_2D_filepath = dir + "/" + mesh_2D_filename_;
	cgogn::io::import_surface<VEC3>(*map2_, mesh_2D_filepath.toStdString());

	dimension_ = map_->add_attribute<uint8, CMap2::Face::ORBIT>("dimension");
	dimension_.set_all_values(2);

	nb_faces_2d_ = map2_->nb_cells<CMap2::Face>();
	nb_vertices_2d_ = map2_->nb_cells<CMap2::Vertex>();

	if (dim_ == 12)
	{
		// import 1D mesh
		CMap2 map2_tmp;
		QString mesh_1D_filepath = dir + "/" + mesh_1D_filename_;
		cgogn::io::import_surface<VEC3>(map2_tmp, mesh_1D_filepath.toStdString());
		map2_tmp.enforce_unique_orbit_embedding<CMap2::Vertex::ORBIT>();
		CMap2::DartMarker dm(*map2_);
		map2_->merge(map2_tmp, dm);
		map2_->parallel_foreach_cell([&] (CMap2::Face f)
		{
			if (dimension_[f] != 2)
				dimension_[f] = 1;
		});
	}

	// build quick traversors
	qtrav_->build<CMap2::Vertex>();
	qtrav_->build<CMap2::Edge>();
	qtrav_->build<CMap2::Face>();

	// create map attributes
	position_ = map_->get_attribute<VEC3, CMap2::Vertex::ORBIT>("position");
	scalar_value_h_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_h");
	scalar_value_u_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_u");
	scalar_value_v_ = map_->add_attribute<SCALAR, CMap2::Vertex::ORBIT>("scalar_value_v");
	water_position_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("water_position");
	flow_velocity_ = map_->add_attribute<VEC3, CMap2::Vertex::ORBIT>("flow_velocity");

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
	val_bc_ = map_->add_attribute<SCALAR, CMap2::Edge::ORBIT>("val_bc");
	typ_bc_ = map_->add_attribute<std::string, CMap2::Edge::ORBIT>("typ_bc_");

	NS_ = map_->add_attribute<uint32, CMap2::Edge::ORBIT>("NS");
	NS_.set_all_values(0.);

	load_2D_constrained_edges();
	if (dim_ == 12)
		load_1D_constrained_edges();

	load_2D_initial_cond_file(dir + "/Initial_Cond_2D.txt");
	if (dim_ == 12)
		load_1D_initial_cond_file(dir + "/Initial_Cond_1D.txt");

	load_2D_boundary_cond_file(dir + "/BC_2D.lim");
	if (dim_ == 12)
		load_1D_boundary_cond_file(dir + "/BC_1D.lim");

	if (dim_ == 12)
		sew_1D_2D_meshes();

	// create adaptive tri/quad handler
	atq_map_ = new cgogn::AdaptiveTriQuadCMap2(*map2_);
	atq_map_->init();

	// init zb & h
	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			uint32 nbv = 0;
			map2_->foreach_incident_vertex(f, [&] (CMap2::Vertex v)
			{
				zb_[f] += position_[v][2];
				nbv++;
			});
			zb_[f] /= nbv;
			h_[f] -= zb_[f];
			// z is read in initial_cond file, not h

			phi_[f] = phi_default_;
		},
		*qtrav_
	);

	map2_->copy_attribute(water_position_, position_);

	cgogn::geometry::compute_centroid<CMap2::Face>(*map2_, position_, centroid_);
	cgogn::geometry::compute_area<CMap2::Face>(*map2_, position_, area_);

	// compute edge properties
	map2_->parallel_foreach_cell(
		[&] (CMap2::Edge e) { compute_edge_length_and_normal(e); },
		*qtrav_
	);
}

void Plugin_ShallowWater::init()
{
	simu_running_ = false;
	nb_iter_ = 0;
	t_ = 0.;

	min_h_per_thread_.resize(cgogn::thread_pool()->nb_workers());
	max_h_per_thread_.resize(cgogn::thread_pool()->nb_workers());
	min_q_per_thread_.resize(cgogn::thread_pool()->nb_workers());
	max_q_per_thread_.resize(cgogn::thread_pool()->nb_workers());
	min_r_per_thread_.resize(cgogn::thread_pool()->nb_workers());
	max_r_per_thread_.resize(cgogn::thread_pool()->nb_workers());

	h_min_ = std::numeric_limits<SCALAR>::max();
	h_max_ = 0.;
	q_min_ = std::numeric_limits<SCALAR>::max();
	q_max_ = 0.;
	r_min_ = std::numeric_limits<SCALAR>::max();
	r_max_ = 0.;
	map2_->foreach_cell(
		[&] (CMap2::Face f) {
			h_min_ = std::min(h_min_, h_[f]);
			h_max_ = std::max(h_max_, h_[f]);
			q_min_ = std::min(q_min_, q_[f]);
			q_max_ = std::max(q_max_, q_[f]);
			r_min_ = std::min(r_min_, r_[f]);
			q_max_ = std::max(r_max_, r_[f]);
		},
		*qtrav_
	);

	for (uint32 i = 0; i < max_depth_; ++i)
		try_subdivision();

	update_draw_data();
}

void Plugin_ShallowWater::start()
{
	start_time_ = std::chrono::high_resolution_clock::now();

	schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	draw_timer_->start(67);
	simu_running_ = true;
	shallow_water_dialog_->simu_running_state_changed();

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
	std::cout << "elapsed time -> " << duration << std::endl;
	std::cout << "t -> " << t_ << std::endl;
	std::cout << "dt -> " << dt_ << std::endl;

	simu_running_ = false;
	schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
	shallow_water_dialog_->simu_running_state_changed();
}

void Plugin_ShallowWater::step()
{
	if (simu_running_)
		stop();
	schnapps_->get_selected_view()->get_current_camera()->disable_views_bb_fitting();
	execute_time_step();
	std::cout << "t -> " << t_ << std::endl;
	std::cout << "dt -> " << dt_ << std::endl;
	update_draw_data();
	schnapps_->get_selected_view()->get_current_camera()->enable_views_bb_fitting();
}

void Plugin_ShallowWater::schnapps_closing()
{
	shallow_water_dialog_->close();
}

void Plugin_ShallowWater::open_dialog()
{
	shallow_water_dialog_->show();
}

void Plugin_ShallowWater::update_draw_data()
{
	if (!simu_running_)
		draw_timer_->stop();

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
				h += h_[f];
//				h += (h_[f] + zb_[f]);
				q += q_[f];
				r += r_[f];
				++nbf;
			});
			h /= nbf;
			q /= nbf;
			r /= nbf;
			scalar_value_h_[v] = h;
			scalar_value_u_[v] = q / h;
			scalar_value_v_[v] = r / h;
			water_position_[v][2] = h;
			flow_velocity_[v] = VEC3(q / h, r / h, 0.);
		},
		*qtrav_
	);

	map_->notify_connectivity_change();
	map_->init_primitives(cgogn::rendering::POINTS);
	map_->init_primitives(cgogn::rendering::LINES);
	map_->init_primitives(cgogn::rendering::TRIANGLES);
	map_->init_primitives(cgogn::rendering::BOUNDARY);

	// notify attribute changes
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_h");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_u");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "scalar_value_v");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "position");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "water_position");
	map_->notify_attribute_change(CMap2::Vertex::ORBIT, "flow_velocity");

	simu_data_access_.unlock();
	map_->unlock_topo_access();
}

void Plugin_ShallowWater::update_time_step()
{
	discharge_.set_all_values(0.);
	swept_.set_all_values(0.);

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			uint32 fidx = map2_->embedding(f);
			map2_->foreach_incident_edge(f, [&] (CMap2::Edge ie)
			{
				uint32 ieidx = map2_->embedding(ie);
				SCALAR le = length_[ieidx];
				SCALAR f1e = f1_[ieidx];
				SCALAR lambda = 0.;
				SCALAR h = h_[fidx];
				if (h > hmin_)
					lambda = fabs(q_[fidx]*normX_[ieidx] + r_[fidx]*normY_[ieidx]) / std::max(h, hmin_) + sqrt(9.81*h);
				swept_[fidx] += le * lambda;
				if (edge_left_side_->is_marked(ie.dart))
					discharge_[fidx] -= le * f1e;
				else
					discharge_[fidx] += le * f1e;
			});
		},
		*qtrav_
	);

	std::vector<SCALAR> min_dt_per_thread(cgogn::thread_pool()->nb_workers());
	for(SCALAR& d : min_dt_per_thread) d = std::min(dt_max_, t_max_ - t_); // Timestep for ending simulation

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			uint32 fidx = map2_->embedding(f);
			uint32 threadidx = cgogn::current_thread_index();
			// Ensure CFL condition
			SCALAR cfl = area_[fidx] / std::max(swept_[fidx], small_);
			min_dt_per_thread[threadidx] = std::min(min_dt_per_thread[threadidx], cfl);
			// Ensure overdry condition
			if (area_[fidx]*phi_[fidx]*(h_[fidx]+zb_[fidx]) < (-discharge_[fidx]*min_dt_per_thread[threadidx]))
				min_dt_per_thread[threadidx] = - area_[fidx]*phi_[fidx]*(h_[fidx]+zb_[fidx]) / discharge_[fidx];
		},
		*qtrav_
	);

	dt_ = *(std::min_element(min_dt_per_thread.begin(), min_dt_per_thread.end()));
}

void Plugin_ShallowWater::execute_time_step()
{
//	auto start = std::chrono::high_resolution_clock::now();

	map2_->parallel_foreach_cell(
		[&] (CMap2::Edge e)
		{
			uint32 eidx = map2_->embedding(e);

			// solve flux on edge
			Str_Riemann_Flux riemann_flux;

			if (map2_->is_incident_to_boundary(e)) // border conditions
			{
				CMap2::Face f(e.dart);
				uint32 fidx = map2_->embedding(f);
				if (phi_[f] > small_)
					riemann_flux = border_condition(typ_bc_[eidx], val_bc_[eidx], normX_[eidx], normY_[eidx], q_[fidx], r_[fidx], h_[fidx]+zb_[fidx], zb_[fidx], 9.81, hmin_, small_);
			}
			else // Inner cell: use the lateralised Riemann solver
			{
				CMap2::Face f1(e.dart),
							f2(map2_->phi2(e.dart));

				uint32 f1idx = map2_->embedding(f1);
				uint32 f2idx = map2_->embedding(f2);

				SCALAR phiL = phi_[f1idx];
				SCALAR phiR = phi_[f2idx];
				SCALAR zbL = zb_[f1idx];
				SCALAR zbR = zb_[f2idx];
				if (h_[f1idx] > hmin_ || h_[f2idx] > hmin_)
				{
					SCALAR hL = h_[f1idx];
					SCALAR hR = h_[f2idx];
					SCALAR qL = q_[f1idx]*normX_[eidx] + r_[f1idx]*normY_[eidx];
					SCALAR qR = q_[f2idx]*normX_[eidx] + r_[f2idx]*normY_[eidx];
					SCALAR rL = -q_[f1idx]*normY_[eidx] + r_[f1idx]*normX_[eidx];
					SCALAR rR = -q_[f2idx]*normY_[eidx] + r_[f2idx]*normX_[eidx];

					if (solver_ == 2)
						riemann_flux = Solv_HLLC(9.81, hmin_, small_, zbL, zbR, phiL, phiR, hL, qL, rL, hR, qR, rR);
					else if (solver_ == 4)
						riemann_flux = Solv_PorAS(9.81, hmin_, small_, zbL, zbR, phiL, phiR, hL, qL, rL, hR, qR, rR);
				}
			}

			f1_[eidx] = riemann_flux.F1;
			f2_[eidx] = riemann_flux.F2;
			f3_[eidx] = riemann_flux.F3;
			s2L_[eidx] = riemann_flux.s2L;
			s2R_[eidx] = riemann_flux.s2R;
		},
		*qtrav_
	);

	update_time_step();

	simu_data_access_.lock();

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			uint32 fidx = map2_->embedding(f);
			map2_->foreach_incident_edge(f, [&] (CMap2::Edge ie)
			{
				uint32 ieidx = map2_->embedding(ie);
				SCALAR fact = dt_ * length_[ieidx];
				SCALAR factF = 0.;
				if (phi_[fidx] > small_)
					factF = fact / area_[fidx] * phi_[fidx];
				if (edge_left_side_->is_marked(ie.dart))
				{
					h_[fidx] -= factF * f1_[ieidx];
					q_[fidx] -= factF * ((f2_[ieidx] + s2L_[ieidx])*normX_[ieidx] - f3_[ieidx]*normY_[ieidx]);
					r_[fidx] -= factF * (f3_[ieidx]*normX_[ieidx] + (f2_[ieidx] + s2L_[ieidx])*normY_[ieidx]);
				}
				else
				{
					h_[fidx] += factF * f1_[ieidx];
					q_[fidx] += factF * (( f2_[ieidx] + s2R_[ieidx])*normX_[ieidx] - f3_[ieidx]*normY_[ieidx]);
					r_[fidx] += factF * ( f3_[ieidx]*normX_[ieidx] + ( f2_[ieidx]+s2R_[ieidx])*normY_[ieidx]);
				}
			});
		},
		*qtrav_
	);

	for(uint32 i = 0; i < cgogn::thread_pool()->nb_workers(); ++i)
	{
		min_h_per_thread_[i] = std::numeric_limits<SCALAR>::max();
		max_h_per_thread_[i] = 0.;
		min_q_per_thread_[i] = std::numeric_limits<SCALAR>::max();
		max_q_per_thread_[i] = 0.;
		min_r_per_thread_[i] = std::numeric_limits<SCALAR>::max();
		max_r_per_thread_[i] = 0.;
	}

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			uint32 fidx = map2_->embedding(f);

			// friction
			if (friction_ != 0)
			{
				SCALAR qx = q_[fidx]*cos(alphaK_) + r_[fidx]*sin(alphaK_);
				SCALAR qy = - q_[fidx]*sin(alphaK_) + r_[fidx]*cos(alphaK_);
				if (h_[fidx] > hmin_)
				{
					qx = qx * exp(-(9.81 * sqrt(qx*qx+qy*qy) / (std::max(kx_*kx_,small_*small_) * pow(h_[fidx],7./3.))) * dt_);
					qy = qy * exp(-(9.81 * sqrt(qx*qx+qy*qy) / (std::max(kx_*kx_,small_*small_) * pow(h_[fidx],7./3.))) * dt_);
				}
				else
				{
					qx = 0.;
					qy = 0.;
				}
				q_[fidx] = qx*cos(alphaK_) - qy*sin(alphaK_);
				r_[fidx] = qx*sin(alphaK_) + qy*cos(alphaK_);
			}

			// optional correction
			// Negative water depth
			if (h_[fidx] < 0.)
			{
				h_[fidx] = 0.;
				q_[fidx] = 0.;
				r_[fidx] = 0.;
			}

			// Abnormal large velocity => Correction of q and r to respect Vmax and Frmax
			if (h_[fidx] > hmin_)
			{
				SCALAR v = sqrt(q_[fidx]*q_[fidx]+r_[fidx]*r_[fidx]) / std::max(h_[fidx], small_);
				SCALAR c = sqrt(9.81 * std::max(h_[fidx], small_));
				SCALAR Fr = v / c;
				SCALAR Fact = std::max({ 1e0, v / v_max_, Fr / Fr_max_ });
				q_[fidx] /= Fact;
				r_[fidx] /= Fact;
			}
			else // Quasi-zero
			{
				q_[fidx] = 0.;
				r_[fidx] = 0.;
			}

			// min and max of each variables for subdivision and simplification
			uint32 idx = cgogn::current_thread_index();
			min_h_per_thread_[idx] = std::min(min_h_per_thread_[idx], h_[fidx]);
			max_h_per_thread_[idx] = std::max(max_h_per_thread_[idx], h_[fidx]);
			min_q_per_thread_[idx] = std::min(min_q_per_thread_[idx], q_[fidx]);
			max_q_per_thread_[idx] = std::max(max_q_per_thread_[idx], q_[fidx]);
			min_r_per_thread_[idx] = std::min(min_r_per_thread_[idx], r_[fidx]);
			max_r_per_thread_[idx] = std::max(max_r_per_thread_[idx], r_[fidx]);

		},
		*qtrav_
	);

	h_min_ = *(std::min_element(min_h_per_thread_.begin(), min_h_per_thread_.end()));
	h_max_ = *(std::max_element(max_h_per_thread_.begin(), max_h_per_thread_.end()));
	q_min_ = *(std::min_element(min_q_per_thread_.begin(), min_q_per_thread_.end()));
	q_max_ = *(std::max_element(max_q_per_thread_.begin(), max_q_per_thread_.end()));
	r_min_ = *(std::min_element(min_r_per_thread_.begin(), min_r_per_thread_.end()));
	r_max_ = *(std::max_element(max_r_per_thread_.begin(), max_r_per_thread_.end()));

	simu_data_access_.unlock();

	if (nb_iter_ % 5 == 0)
	{
		map_->lock_topo_access();
		try_simplification();
		try_subdivision();
		map_->unlock_topo_access();
	}

	t_ += dt_;
	nb_iter_++;

	if (t_ == t_max_)
		stop();

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

	std::vector<std::vector<CMap2::Face>*> faces_to_subdivide_per_thread(cgogn::thread_pool()->nb_workers());
	for (auto& fv : faces_to_subdivide_per_thread)
		fv = cgogn::dart_buffers()->cell_buffer<CMap2::Face>();

	map2_->parallel_foreach_cell(
		[&] (CMap2::Face f)
		{
			if (atq_map_->face_level(f) >= max_depth_)
				return;

			uint32 fidx = map2_->embedding(f);

			SCALAR max_diff_h = 0.;
			SCALAR max_diff_q = 0.;
			SCALAR max_diff_r = 0.;
			map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
			{
				uint32 afidx = map2_->embedding(af);
				max_diff_h = std::max(max_diff_h, fabs(h_[fidx] - h_[afidx]));
				max_diff_q = std::max(max_diff_q, fabs(q_[fidx] - q_[afidx]));
				max_diff_r = std::max(max_diff_r, fabs(r_[fidx] - r_[afidx]));
			});

			if (max_diff_h > 0.05 * (h_max_ - h_min_) ||
				max_diff_q > 0.05 * (q_max_ - q_min_) ||
				max_diff_r > 0.05 * (r_max_ - r_min_))
			{
				faces_to_subdivide_per_thread[cgogn::current_thread_index()]->push_back(f);
			}
		},
		*qtrav_
	);

	for (auto& fv : faces_to_subdivide_per_thread)
	{
		for (CMap2::Face f : *fv)
		{
			if (!subdivided.is_marked(f))
			{
				SCALAR old_h;
				SCALAR old_q;
				SCALAR old_r;
				SCALAR old_phi;
				VEC3& old_centroid = centroid_[f];

				atq_map_->subdivide_face(f,
					[&] (CMap2::Vertex v)
					{
						if (map2_->is_incident_to_boundary(CMap2::Edge(v.dart)))
						{
							typ_bc_[CMap2::Edge(v.dart)] = typ_bc_[CMap2::Edge(map2_->phi_1(v.dart))];
							val_bc_[CMap2::Edge(v.dart)] = val_bc_[CMap2::Edge(map2_->phi_1(v.dart))];
						}
						position_[v] = (position_[CMap2::Vertex(map2_->phi_1(v.dart))] + position_[CMap2::Vertex(map2_->phi1(v.dart))]) / 2.;
						water_position_[v] = position_[v];
						compute_edge_length_and_normal(CMap2::Edge(v.dart));
						compute_edge_length_and_normal(CMap2::Edge(map2_->phi_1(v.dart)));
						qtrav_->update(v);
						qtrav_->update(CMap2::Edge(v.dart));
						qtrav_->update(CMap2::Edge(map2_->phi_1(v.dart)));
					},
					[&] (CMap2::Face f)
					{
						uint32 fidx = map2_->embedding(f);
						old_h = h_[fidx];
						old_q = q_[fidx];
						old_r = r_[fidx];
						old_phi = phi_[fidx];
						old_centroid = centroid_[fidx];
					},
					[&] (CMap2::Face f)
					{
						if (atq_map_->is_triangle_face(f))
						{
							cgogn::Dart cfd = map2_->phi<12>(f.dart);
							CMap2::Face cf(cfd);
							uint32 cfidx = map2_->embedding(cf);

							qtrav_->update(cf);
							subdivided.mark(cf);
							h_[cfidx] = old_h;
							q_[cfidx] = old_q;
							r_[cfidx] = old_r;
							phi_[cfidx] = old_phi;
							SCALAR zb = 0.;
							uint32 nbv = 0;
							map2_->foreach_incident_vertex(cf, [&] (CMap2::Vertex iv)
							{
								zb += position_[iv][2];
								nbv++;
							});
							zb_[cfidx] = zb / nbv;
							centroid_[cfidx] = cgogn::geometry::centroid(*map2_, cf, position_);
							area_[cfidx] = cgogn::geometry::area(*map2_, cf, position_);
							dimension_[cfidx] = 2;

							map2_->foreach_incident_edge(cf, [&] (CMap2::Edge ie)
							{
								qtrav_->update(ie);
								compute_edge_length_and_normal(ie);
							});

							map2_->foreach_adjacent_face_through_edge(cf, [&] (CMap2::Face af)
							{
								uint32 afidx = map2_->embedding(af);
								qtrav_->update(af);
								subdivided.mark(af);
								h_[afidx] = old_h;
								q_[afidx] = old_q;
								r_[afidx] = old_r;
								phi_[afidx] = old_phi;
								SCALAR zb = 0.;
								uint32 nbv = 0;
								map2_->foreach_incident_vertex(af, [&] (CMap2::Vertex iv)
								{
									zb += position_[iv][2];
									nbv++;
								});
								zb_[afidx] = zb / nbv;
								centroid_[afidx] = cgogn::geometry::centroid(*map2_, af, position_);
								area_[afidx] = cgogn::geometry::area(*map2_, af, position_);
								dimension_[afidx] = 2;
							});
						}
						else
						{
							cgogn::Dart cvd = map2_->phi<12>(f.dart);
							CMap2::Vertex cv(cvd);

							position_[cv] = old_centroid;
							water_position_[cv] = position_[cv];
							qtrav_->update(cv);

							map2_->foreach_incident_edge(cv, [&] (CMap2::Edge ie)
							{
								qtrav_->update(ie);
								compute_edge_length_and_normal(ie);
							});

							map2_->foreach_incident_face(cv, [&] (CMap2::Face af)
							{
								uint32 afidx = map2_->embedding(af);
								qtrav_->update(af);
								subdivided.mark(af);
								h_[afidx] = old_h;
								q_[afidx] = old_q;
								r_[afidx] = old_r;
								phi_[afidx] = old_phi;
								zb_[afidx] = 0.;
								SCALAR zb = 0.;
								uint32 nbv = 0;
								map2_->foreach_incident_vertex(af, [&] (CMap2::Vertex iv)
								{
									zb += position_[iv][2];
									nbv++;
								});
								zb_[afidx] = zb / nbv;
								centroid_[afidx] = cgogn::geometry::centroid(*map2_, af, position_);
								area_[afidx] = cgogn::geometry::area(*map2_, af, position_);
								dimension_[afidx] = 2;
							});
						}
					}
				);
			}
		}
	}

	for (auto& fv : faces_to_subdivide_per_thread)
		cgogn::dart_buffers()->release_cell_buffer<CMap2::Face>(fv);
}

void Plugin_ShallowWater::try_simplification()
{
	std::vector<CMap2::Face>* to_simplify = cgogn::dart_buffers()->cell_buffer<CMap2::Face>();
	CMap2::CellMarker<CMap2::Face::ORBIT> simplified(*map2_);

	map2_->foreach_cell(
		[&] (CMap2::Face f)
		{
			if (simplified.is_marked(f))
				return;

			if (atq_map_->is_simplifiable(f))
			{
				std::vector<CMap2::Face>* subfaces = cgogn::dart_buffers()->cell_buffer<CMap2::Face>();

				SCALAR max_diff_h = 0.;
				SCALAR max_diff_q = 0.;
				SCALAR max_diff_r = 0.;

				switch (atq_map_->face_type(f))
				{
					case cgogn::AdaptiveTriQuadCMap2::TRI_CORNER: {
						CMap2::Face cf(map2_->phi<12>(atq_map_->oldest_dart(f))); // central face
						subfaces->push_back(cf);
						map2_->foreach_adjacent_face_through_edge(cf, [&] (CMap2::Face af)
						{
							subfaces->push_back(af);
							max_diff_h = std::max(max_diff_h, fabs(h_[f] - h_[af]));
							max_diff_q = std::max(max_diff_q, fabs(q_[f] - q_[af]));
							max_diff_r = std::max(max_diff_r, fabs(r_[f] - r_[af]));
						});
						break;
					}
					case cgogn::AdaptiveTriQuadCMap2::TRI_CENTRAL: {
						subfaces->push_back(f);
						map2_->foreach_adjacent_face_through_edge(f, [&] (CMap2::Face af)
						{
							subfaces->push_back(af);
							max_diff_h = std::max(max_diff_h, fabs(h_[f] - h_[af]));
							max_diff_q = std::max(max_diff_q, fabs(q_[f] - q_[af]));
							max_diff_r = std::max(max_diff_r, fabs(r_[f] - r_[af]));
						});
						break;
					}
					case cgogn::AdaptiveTriQuadCMap2::QUAD: {
						cgogn::Dart cv = map2_->phi<12>(atq_map_->oldest_dart(f));
						map2_->foreach_incident_face(CMap2::Vertex(cv), [&] (CMap2::Face iface)
						{
							subfaces->push_back(iface);
							max_diff_h = std::max(max_diff_h, fabs(h_[f] - h_[iface]));
							max_diff_q = std::max(max_diff_q, fabs(q_[f] - q_[iface]));
							max_diff_r = std::max(max_diff_r, fabs(r_[f] - r_[iface]));
						});
						break;
					}
				}

				if (max_diff_h < 0.02 * (h_max_ - h_min_) &&
					max_diff_q < 0.02 * (q_max_ - q_min_) &&
					max_diff_r < 0.02 * (r_max_ - r_min_))
				{
					to_simplify->push_back(f);
					for (CMap2::Face sf : *subfaces)
						simplified.mark(sf);
				}

				cgogn::dart_buffers()->release_cell_buffer<CMap2::Face>(subfaces);
			}
		},
		*qtrav_
	);

	std::vector<SCALAR> old_h; old_h.reserve(4);
	std::vector<SCALAR> old_q; old_q.reserve(4);
	std::vector<SCALAR> old_r; old_r.reserve(4);
	std::vector<SCALAR> old_phi; old_phi.reserve(4);
	std::vector<SCALAR> old_area; old_area.reserve(4);

	for (CMap2::Face f : *to_simplify)
	{
		atq_map_->simplify_face(f,
			[&] (CMap2::Face f)
			{
				old_h.clear();
				old_q.clear();
				old_r.clear();
				old_phi.clear();
				old_area.clear();

				if (atq_map_->is_triangle_face(f))
				{
					CMap2::Face cf(f.dart);
					if (atq_map_->face_type(f) == cgogn::AdaptiveTriQuadCMap2::TRI_CORNER)
						cf.dart = map2_->phi<12>(atq_map_->oldest_dart(f));
					uint32 cfidx = map2_->embedding(cf);
					old_h.push_back(h_[cfidx]);
					old_q.push_back(q_[cfidx]);
					old_r.push_back(r_[cfidx]);
					old_phi.push_back(phi_[cfidx]);
					old_area.push_back(area_[cfidx]);
					map2_->foreach_adjacent_face_through_edge(cf, [&] (CMap2::Face af)
					{
						uint32 afidx = map2_->embedding(af);
						old_h.push_back(h_[afidx]);
						old_q.push_back(q_[afidx]);
						old_r.push_back(r_[afidx]);
						old_phi.push_back(phi_[afidx]);
						old_area.push_back(area_[afidx]);
					});
				}
				else
				{
					CMap2::Vertex cv(map2_->phi<12>(atq_map_->oldest_dart(f)));
					map2_->foreach_incident_face(cv, [&] (CMap2::Face iface)
					{
						uint32 ifaceidx = map2_->embedding(iface);
						old_h.push_back(h_[ifaceidx]);
						old_q.push_back(q_[ifaceidx]);
						old_r.push_back(r_[ifaceidx]);
						old_phi.push_back(phi_[ifaceidx]);
						old_area.push_back(area_[ifaceidx]);
					});
				}
			},
			[&] (CMap2::Face f)
			{
				uint32 fidx = map2_->embedding(f);

				qtrav_->update(f);

				uint32 nb = 0;
				VEC3 centroid(0,0,0);
				cgogn::Dart it = f.dart;
				do
				{
					cgogn::Dart next = map2_->phi<11>(it);
					centroid += position_[CMap2::Vertex(it)];
					++nb;
					it = next;
				} while (it != f.dart);
				centroid_[fidx] = centroid / SCALAR(nb);

				SCALAR h = 0.;
				SCALAR q = 0.;
				SCALAR r = 0.;
				SCALAR phi = 0.;
				SCALAR area = 0.;
				for(int i = 0; i < old_area.size(); i++)
				{
					h += old_h[i] * old_area[i];
					q += old_q[i] * old_area[i];
					r += old_r[i] * old_area[i];
					phi += old_phi[i] * old_area[i];
					area += old_area[i];
				}
				h_[fidx] = h / area;
				q_[fidx] = q / area;
				r_[fidx] = r / area;
				phi_[fidx] = phi / area;
				area_[fidx] = cgogn::geometry::area(*map2_, f, position_);

				SCALAR zb = 0.;
				nb = 0;
				map2_->foreach_incident_vertex(f, [&] (CMap2::Vertex iv)
				{
					zb += position_[iv][2];
					nb++;
				});
				zb_[f] = zb / nb;
			},
			[&] (CMap2::Edge e)
			{
				qtrav_->update(e);
				compute_edge_length_and_normal(e);
			}
		);
	}

	cgogn::dart_buffers()->release_cell_buffer<CMap2::Face>(to_simplify);
}

Plugin_ShallowWater::Str_Riemann_Flux Plugin_ShallowWater::Solv_HLLC(
	SCALAR g, SCALAR hmin, SCALAR smalll,
	SCALAR zbL,SCALAR zbR,
	SCALAR PhiL,SCALAR PhiR,
	SCALAR hL, SCALAR qL, SCALAR rL, SCALAR hR, SCALAR qR, SCALAR rR
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
		SCALAR L1L = qL / std::max(hL, smalll) - sqrt(g * std::max(hL, smalll));
		SCALAR L3L = qL / std::max(hL, smalll) + sqrt(g * std::max(hL, smalll));
		SCALAR L1R = qR / std::max(hR, smalll) - sqrt(g * std::max(hR, smalll));
		SCALAR L3R = qR / std::max(hR, smalll) + sqrt(g * std::max(hR, smalll));
		SCALAR L1LR = std::min({ L1L, L1R, 0e0 });
		SCALAR L3LR = std::max({ L3L, L3R, 0e0 });
		//========================
		SCALAR PhiLR = std::min(PhiL, PhiR);
		//------compute F1--------
		F1 = L3LR * qL - L1LR * qR + L1LR * L3LR * (zR - zL);
		F1 = F1 * PhiLR / std::max(L3LR - L1LR, smalll);
		//========================
		//-----compute F2---------
		SCALAR F2L = (qL * qL) / std::max(hL, smalll) + 5e-1 * g * hL * hL;
		SCALAR F2R = (qR * qR) / std::max(hR, smalll) + 5e-1 * g * hR * hR;
		F2 = (L3LR * PhiL * F2L - L1LR * PhiR * F2R + L1LR * L3LR * (PhiR * qR - PhiL * qL))
				/ std::max(L3LR-L1LR, smalll);
		//==========================
		//-----Compute S2L and S2R---
		SCALAR Fact = 0.5 * PhiLR * (hL + hR);
		s2L = 0.5 * (PhiL * hL * hL - PhiR * hR * hR)
				- Fact * (zL - zR);
		s2L = g * L1LR * s2L / std::max(L3LR - L1LR, smalll);
		s2R = 0.5 * (PhiR * hR * hR - PhiL * hL * hL)
				- Fact * (zR - zL);
		s2R = g * L3LR * s2R / std::max(L3LR - L1LR, smalll);
		//============================
		//------Compute F3------------
		if (F1 > 0)
			F3 = F1 * rL / std::max(hL, smalll);
		else
			F3 = F1 * rR / std::max(hR, smalll);
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

Plugin_ShallowWater::Str_Riemann_Flux Plugin_ShallowWater::Solv_PorAS(
	SCALAR g, SCALAR hmin, SCALAR smalll,
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
			uL = qL / std::max(hL, smalll);
		else
			uL = 0;
		SCALAR uR;                          // velocity on the right side
		if (hR > hmin)
			uR = qR / std::max(hR, smalll);
		else
			uR = 0;

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
		SCALAR L1LR = std::min(L1L,L1R);    // 1st wave speed at the interface
		SCALAR L2LR = std::max(L2L,L2R);    // 2nd wave speed at the interface

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
			zS = zR;
		else
			zS = zL;
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
			if (fabs(hI - hL) > hmin)
				L1L = (uI * hI - uL * hL) / (hI - hL);
			/** @todo remplacer @a hmin par @a smalll ??? **/
			if (fabs(uI * hI - uL * hL) > hmin)
				L1L = (pow(uI, 2) * hI - pow(uL, 2) * hL + 5e-1 * g * pow(hI, 2) - 5e-1 * g * pow(hL, 2)) / (uI * hI - uL * hL);
			L1I = L1L;
		}
		// 2ème onde (u+c)
		// 2nd wave (u+c)
		// L3R<L3I => Shock for the 2nd wave
		if (L2R < L2I)
		{
			// ???? PFG : pourquoi dans cet ordre ????
			if (fabs(hI - hR) > hmin)
				L2R = (uI * hI - uR * hR) / (hI - hR);
			/** @todo remplacer @a hmin par @a smalll ??? **/
			if (fabs(uI * hI - uR * hR) > hmin)
				L2R = (pow(uR, 2) * hR - pow(uI, 2) * hI + 5e-1 * g * pow(hR, 2) - 5e-1 * g * pow(hI, 2)) / (uR * hR - uI * hI);
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
			Riemann_Flux.F1 = (L2LR * F1L - L1LR * F1R - L1LR * L2LR * PhiLR * (zL - zR)) / std::max(L2LR - L1LR, smalll);
			Riemann_Flux.F2 = (L2LR * F2L - L1LR * F2R - L1LR * L2LR * (F1L - F1R)) / std::max(L2LR - L1LR, smalll);
		}
		else if ((L1I <0) && (L2I >= 0))
		{
			// Ecoulement fluvial entre les mailles L et R
			// Calcul des flux dans la zone intermédiaire d'état constant
			// Subcritical flow between the L-cell and R-cell
			// Flux computation in the constant intermediate zone
			Riemann_Flux.F1 = ((F2L - F2R - L1LR * F1L + L2LR * F1R) + (Phi_Term - Bot_Term)) / std::max(L2LR - L1LR, smalll);
			Riemann_Flux.F2 = (L2LR * F2L - L1LR * F2R - L1LR * L2LR * (F1L - F1R)) / std::max(L2LR - L1LR, smalll);
		}
		else if ((L2I < 0) && (L2R >= 0))
		{
			// Ecoulement critique de la maille R vers la maille L
			// Critical flow from R-cell to L-cell
			SCALAR PhiLR = PhiR;
			Riemann_Flux.F1 = (L2LR * F1L - L1LR * F1R - L1LR * L2LR * PhiLR * (zL - zR)) / std::max(L2LR - L1LR, smalll);
			Riemann_Flux.F2 = (L2LR * F2L - L1LR * F2R - L1LR * L2LR * (F1L - F1R)) / std::max(L2LR - L1LR, smalll);
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
			Riemann_Flux.F3 = Riemann_Flux.F1 * rL / std::max(hL, smalll);
		else
			Riemann_Flux.F3 = Riemann_Flux.F1 * rR / std::max(hR, smalll);
		// Upwind computation of the source term s2L and s2R
		Riemann_Flux.s2L = - L1LR * (-Bot_Term + Phi_Term) / std::max(L2LR - L1LR, smalll);
		Riemann_Flux.s2R = L2LR * (-Bot_Term + Phi_Term) / std::max(L2LR - L1LR, smalll);
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

Plugin_ShallowWater::Str_Riemann_Flux Plugin_ShallowWater::border_condition(
	std::string typBC, SCALAR ValBC,
	SCALAR NormX, SCALAR NormY,
	SCALAR q, SCALAR r, SCALAR z, SCALAR zb,
	SCALAR g, SCALAR hmin, SCALAR smalll)
{
	Str_Riemann_Flux Flux;

	//-----------initialization------------
	//   h1,q1,r1;h,q,r within the domain
	SCALAR q1 = q * NormX + r * NormY;
	SCALAR r1 =-q * NormY + r * NormX;
	SCALAR h1 = z - zb;

	q1 = -q1;
	r1 = -r1;

	if (h1 < hmin)
	{
		h1 = 0.;
		q1 = 0.;
		r1 = 0.;
	}

	// Characteristic variables
	SCALAR c1 = sqrt(g * h1);
	SCALAR u1 = q1 / std::max(h1, smalll);
	SCALAR v1 = r1 / std::max(h1, smalll);
	SCALAR L1 = std::max(u1 + c1, 0.);
	//===================================================================

	SCALAR F1 = 0.;
	SCALAR F2 = 0.;
	SCALAR F3 = 0.;
	//-----Boundary conditions-------------------
	//-----Free Outflow-------
	if (typBC.compare("f") == 0 || typBC.compare("F") == 0)
	{
		//----message------
		F1 = q1;
		F2 = q1 * u1 + 0.5 * g * h1 * h1;
	}
	//=========================
	//-------Critical Section----
	else if (typBC.compare("c") == 0 || typBC.compare("C") == 0)
	{
		//-----message------
		SCALAR c=(u1-2*c1)/(ValBC-2);
		c=std::max(c,0.);
		SCALAR u=-ValBC*c;
		SCALAR h=c*c/g;
		F1=h*u;
		F2=h*u*u+0.5*g*h*h;
	}
	//============================
	//-------Prescribed h---------
	else if (typBC.compare("h") == 0 || typBC.compare("H") == 0)
	{
		//------message----------
		SCALAR h = std::max(ValBC, 0.);
		SCALAR u = 0;

		if(L1 < 0)
		{
			/* torrentiel sortant*/
			h = h1;
			u = u1;
		}
		else
		{
			SCALAR cmin = std::max({ sqrt(g * h), (2 * c1 - u1) / 3.0, 0.0 });
			h = std::max(h, (cmin * cmin) / g);
			SCALAR c = sqrt(g * h);
			u = u1 + 2 * (c - c1);
		}
		F1 = h * u;
		F2 = h * u * u+ 0.5 * g * h * h;

	}
	//==============================
	//-------Prescribed z-----------
	else if (typBC.compare("z") == 0 || typBC.compare("Z") == 0)
	{
		//------message-----
		SCALAR h=std::max(ValBC - zb,0.);
		SCALAR c=sqrt(g*h);
		SCALAR u=u1+2*(c-c1);
		F1=h*u;
		F2=h*u*u+0.5*g*h*h;

		/** @todo Utilité ??? **/
		h=std::max(ValBC-zb,0.);//why is part need
		if (L1 < 0)
		{
			/* torrentiel sortant*/
			h = h1;
			u = u1;
		}
		else
		{
			SCALAR cmin=std::max({ sqrt(g*h), (2*c1-u1)/3, 0. });
			h=std::max(h,(cmin*cmin)/g);
			c=sqrt(g*h);
			u=u1+2*(c-c1);
		}
		F1=h*u;
		F2=h*u*u+0.5*g*h*h;
	}
	//===============================
	//--------Prescribed q-----------
	else if (typBC.compare("q") == 0 || typBC.compare("Q") == 0)
	{
		//-----message-------
		F1=ValBC;
		SCALAR hc=pow(((F1*F1)/g),1/3);
		if (hc>=h1)
		{
			F2=(q1*q1)/std::max(hc,smalll)+
					0.5*g*h1*h1+
					(F1-q1)*L1;
		}
		else
		{
			F2=(q1*q1)/std::max(h1,smalll)+
					0.5*g*h1*h1+
					(F1-q1)*L1;
		}
	}
	//=================================
	//---------Weir--------------------
	else if (typBC.compare("s") == 0 || typBC.compare("S") == 0)
	{
		/**
	 ** @todo Implémenter les BC de type 's' en renseignant la cote de la pelle et non la hauteur (permet de gérer les cas avec plusieurs mailles de cote du fond diférentes attenantes au même seuil)
	 **/
		//-----message-------
		if (h1<ValBC)
		{
			//No z:weir elevation not reached
			F1=0;
			F2=(q1*q1)/std::max(h1,smalll)
					+0.5*g*h1*h1;
		}
		else
		{
			// Weir overtoped
			F1=-0.42*sqrt(2*g)*pow((h1-ValBC),3/2);
			F2=(q1*q1)/std::max(h1,smalll)
					+0.5*g*h1*h1;
		}
	}
	else
	{
		std::cout << "pbl bc" << std::endl;
		std::cout << typBC << std::endl;
	}
	F3=(F1-fabs(F1))*v1/2;

	//----output-----
	F1 = -F1;

	//--return F1,F2,F3
	Flux.F1=F1;
	Flux.F2=F2;
	Flux.F3=F3;
	Flux.s2L = 0.;
	Flux.s2R = 0.;

	return Flux;
}

void Plugin_ShallowWater::get_LR_faces(CMap2::Edge e, CMap2::Face& fl, CMap2::Face& fr)
{
	if (edge_left_side_->is_marked(e.dart))
	{
		fl.dart = e.dart;
		fr.dart = map2_->phi2(e.dart);
	}
	else
	{
		fl.dart = map2_->phi2(e.dart);
		fr.dart = e.dart;
	}
}

void Plugin_ShallowWater::compute_edge_length_and_normal(CMap2::Edge e)
{
	SCALAR l = cgogn::geometry::length(*map2_, e, position_);
	length_[e] = l;
	VEC3 vec = position_[CMap2::Vertex(map2_->phi2(e.dart))] - position_[CMap2::Vertex(e.dart)];
	normX_[e] = vec[1] / l;
	normY_[e] = -vec[0] / l;
	edge_left_side_->mark(e.dart);
	edge_left_side_->unmark(map2_->phi2(e.dart));
}

bool Plugin_ShallowWater::almost_equal(VEC3 v1, VEC3 v2)
{
	return (((v1[0]-v2[0])*(v1[0]-v2[0]) + (v1[1]-v2[1])*(v1[1]-v2[1]) + (v1[2]-v2[2])*(v1[2]-v2[2])) <= 1e-6);
}

bool Plugin_ShallowWater::are_points_aligned(VEC3 p1, VEC3 p2, VEC3 p3)
{
	SCALAR a = (p3[1] - p2[1])*(p1[2] - p2[2]) - (p3[2] - p2[2])*(p1[1] - p2[1]);
	SCALAR b = (p3[2] - p2[2])*(p1[0] - p2[0]) - (p3[0] - p2[0])*(p1[2] - p2[2]);
	SCALAR c = (p3[0] - p2[0])*(p1[1] - p2[1]) - (p3[1] - p2[1])*(p1[0] - p2[0]);
	return ((a < 1e-3) && (b < 1e-3) && (c < 1e-3));
}

bool Plugin_ShallowWater::is_point_in_segment(VEC3 A, VEC3 B, VEC3 C)
{
	SCALAR ABAB = (B[0]-A[0])*(B[0]-A[0]) + (B[1]-A[1])*(B[1]-A[1]) + (B[2]-A[2])*(B[2]-A[2]);
	SCALAR ACAB = (B[0]-A[0])*(C[0]-A[0]) + (B[1]-A[1])*(C[1]-A[1]) + (B[2]-A[2])*(C[2]-A[2]);
	return ACAB >= 0 && ACAB <= ABAB;
}

bool Plugin_ShallowWater::sew_faces_recursive(CMap2::Edge e1, CMap2::Edge e2)
// e1 belongs to a 1D face
// e2 belongs to a 2D face
{
	CMap2::Vertex v1(e1.dart);
	CMap2::Vertex v2(map2_->phi2(e1.dart));
	CMap2::Vertex v3(e2.dart);
	CMap2::Vertex v4(map2_->phi2(e2.dart));

	bool tmp;

	if (!are_points_aligned(position_[v3], position_[v1], position_[v2]) || !are_points_aligned(position_[v4], position_[v1], position_[v2]))
		// the 4 vertices are not aligned
		return true;

	else if (almost_equal(position_[v1], position_[v4]) && almost_equal(position_[v2], position_[v3]))
	{
		typ_bc_[e2] = "q";
		map2_->sew_faces(e2, e1);
		return false;
	}

	else if (almost_equal(position_[v2], position_[v3]) && is_point_in_segment(position_[v3], position_[v4], position_[v1]))
	{
		CMap2::Vertex v = map2_->cut_edge(e2);
		position_[v] = position_[v1];
		qtrav_->update(v);
		qtrav_->update(CMap2::Edge(map2_->phi1(e2.dart)));
		typ_bc_[CMap2::Edge(map2_->phi1(e2.dart))] = "m2";
		typ_bc_[e2] = "q";
		CMap2::Edge e1_tmp(map2_->phi2(map2_->phi1(map2_->phi2(e1.dart))));
		map2_->sew_faces(e2,e1);
		tmp = sew_faces_recursive(e1_tmp, CMap2::Edge(map2_->phi1(e2.dart)));
		return false;
	}

	else if (almost_equal(position_[v1], position_[v4]) && is_point_in_segment(position_[v3], position_[v4], position_[v2]))
	{
		CMap2::Vertex v = map2_->cut_edge(e2);
		position_[v] = position_[v2];
		qtrav_->update(v);
		qtrav_->update(CMap2::Edge(map2_->phi1(e2.dart)));
		typ_bc_[CMap2::Edge(map2_->phi1(e2.dart))] = "q";
		CMap2::Edge e1_tmp(map2_->phi2(map2_->phi_1(map2_->phi2(e1.dart))));
		map2_->sew_faces(CMap2::Edge(map2_->phi1(e2.dart)),e1);
		tmp = sew_faces_recursive(e1_tmp, e2);
		return false;
	}

	else if (almost_equal(position_[v2], position_[v3]) && is_point_in_segment(position_[v1], position_[v2], position_[v4]))
	{
		CMap2::Vertex v = map2_->cut_edge(e1);
		position_[v] = position_[v4];
		qtrav_->update(v);
		qtrav_->update(CMap2::Edge(map2_->phi1(e1.dart)));
		typ_bc_[e2] = "q";
		CMap2::Edge e2_tmp(map2_->phi2(map2_->phi_1(map2_->phi2(e2.dart))));
		map2_->sew_faces(e2,CMap2::Edge(map2_->phi1(e1.dart)));
		tmp = sew_faces_recursive(e1, e2_tmp);
		return false;
	}

	else if (almost_equal(position_[v1], position_[v4]) && is_point_in_segment(position_[v1], position_[v2], position_[v3]))
	{
		CMap2::Vertex v = map2_->cut_edge(e1);
		position_[v] = position_[v3];
		qtrav_->update(v);
		qtrav_->update(CMap2::Edge(map2_->phi1(e1.dart)));
		typ_bc_[CMap2::Edge(map2_->phi1(e1.dart))] = "m1";
		typ_bc_[e2] = "q";
		CMap2::Edge e2_tmp(map2_->phi2(map2_->phi1(map2_->phi2(e2.dart))));
		map2_->sew_faces(e2,e1);
		tmp = sew_faces_recursive(CMap2::Edge(map2_->phi1(e1.dart)), e2_tmp);
		return false;
	}

	else
		return true;
}

} // namespace plugin_shallow_water_2

} // namespace schnapps
