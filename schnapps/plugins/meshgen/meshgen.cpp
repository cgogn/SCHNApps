/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin MeshGen                                                               *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#include <schnapps/plugins/meshgen/meshgen.h>
#include <schnapps/plugins/meshgen/tetgen_structure_io.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>

#include <cgogn/core/utils/unique_ptr.h>
#include <cgogn/io/map_export.h>
#ifdef PLUGIN_MESHGEN_WITH_CGAL
#include <cgogn/modeling/algos/refinements.h>
#include <cgal/c3t3_import.h>
#endif // PLUGIN_MESHGEN_WITH_CGAL
#include <schnapps/plugins/image/image.h>

#include <tetgen/tetgen.h>
#include <netgen_structure_io.h>
#include <netgen/nglib/nglib.h>

namespace schnapps
{

namespace plugin_meshgen
{

TetgenParameters::TetgenParameters() :
	tetgen_command_line("-pqY")
{}

NetgenParameters::NetgenParameters() :
	uselocalh(true),
	maxh(1000),
	minh(0),
	fineness(0.5),
	grading(0.3),
	elementsperedge(2.0),
	elementspercurve(2.0),
	closeedgeenable(false),
	closeedgefact(2.0),
	minedgelenenable(false),
	minedgelen(1e-4),
	second_order(false),
	quad_dominated(false),
	meshsize_filename(nullptr),
	optsurfmeshenable(true),
	optvolmeshenable(true),
	optsteps_2d(3),
	optsteps_3d(3),
	invert_tets(false),
	invert_trigs(false),
	check_overlap(true),
	check_overlapping_boundary(true)
{}

CGALParameters::CGALParameters() :
	cell_size_(8),
	cell_radius_edge_ratio_(3),
	facet_angle_(30),
	facet_size_(15),
	facet_distance_(10),
	do_odt_(true),
	do_odt_freeze_(true),
	odt_max_iter_(0),
	odt_convergence_(0.02),
	odt_freeze_bound_(0.01),
	do_lloyd_(true),
	do_lloyd_freeze_(true),
	lloyd_max_iter_(0),
	lloyd_convergence_(0.02),
	lloyd_freeze_bound_(0.01),
	do_perturber_(true),
	perturber_sliver_bound_(0),
	do_exuder_(true),
	exuder_sliver_bound_(0)
{

}

MeshGeneratorParameters::MeshGeneratorParameters() :
	cgal(),
	netgen(),
	tetgen()
{}

Plugin_VolumeMeshFromSurface::Plugin_VolumeMeshFromSurface() :
	gen_mesh_action_(nullptr),
	plugin_image_(nullptr),
	plugin_cmap_provider_(nullptr),
	generation_parameters_(),
	dialog_(nullptr)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_VolumeMeshFromSurface::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_VolumeMeshFromSurface::enable()
{
	if (!dialog_)
		dialog_ = cgogn::make_unique<VolumeMeshFromSurfaceDialog>(schnapps_, this);

	while(dialog_->export_dialog_->comboBox_images->count() > 1)
		dialog_->export_dialog_->comboBox_images->removeItem(dialog_->export_dialog_->comboBox_images->count() -1);

	gen_mesh_action_ = schnapps_->add_menu_action("Export;Tetrahedralize", "Tetrahedralize");
	connect(gen_mesh_action_, SIGNAL(triggered()), dialog_.get(), SLOT(show_export_dialog()));

	schnapps_->foreach_object([&](Object* mhg)
	{
		dialog_->map_added(mhg);
	});

	plugin_image_ = static_cast<plugin_image::Plugin_Image*>(schnapps_->enable_plugin(plugin_image::Plugin_Image::plugin_name()));
	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));

	if (!plugin_cmap_provider_)
		return false;

	schnapps_->foreach_object([&](Object* im)
	{
			dialog_->image_added(im);
	});

	return true;
}

void Plugin_VolumeMeshFromSurface::disable()
{
	schnapps_->remove_menu_action(gen_mesh_action_);
}

void Plugin_VolumeMeshFromSurface::generate_button_netgen_pressed()
{
	MapHandler2* handler_map2 = plugin_cmap_provider_->cmap2(dialog_->get_selected_map());
	if (handler_map2)
	{
		Map2* map = handler_map2->map();
		const std::string& position_att_name = dialog_->export_dialog_->comboBoxPositionSelection->currentText().toStdString();
		auto position_att = map->template get_attribute<VEC3, Map2::Vertex::ORBIT>(position_att_name);

		if (!position_att.is_valid())
		{
			cgogn_log_info("Plugin_VolumeMeshFromSurface") << "The position attribute has to be of type VEC3.";
			return;
		}
		generate_netgen(handler_map2, position_att, generation_parameters_.netgen);
	}
}

void Plugin_VolumeMeshFromSurface::generate_button_tetgen_pressed()
{
	MapHandler2* handler_map2 = plugin_cmap_provider_->cmap2(dialog_->get_selected_map());
	if (handler_map2)
	{
		Map2* map = handler_map2->map();
		const std::string& position_att_name = dialog_->export_dialog_->comboBoxPositionSelection->currentText().toStdString();
		auto position_att = map->template get_attribute<VEC3, Map2::Vertex::ORBIT>(position_att_name);

		if (!position_att.is_valid())
		{
			cgogn_log_info("Plugin_VolumeMeshFromSurface") << "The position attribute has to be of type VEC3.";
			return;
		}
		const std::string& tetgen_command_line = generation_parameters_.tetgen.tetgen_command_line;
		generate_tetgen(handler_map2, position_att, tetgen_command_line.c_str());
	}
}

Plugin_VolumeMeshFromSurface::MapHandler3*Plugin_VolumeMeshFromSurface::generate_netgen(Plugin_VolumeMeshFromSurface::MapHandler2* mh2, CMap2::VertexAttribute<VEC3> position_att, const NetgenParameters& params)
{
	if (!mh2 || !position_att.is_valid())
		return nullptr;

	Map2* map = mh2->map();
	auto netgen_structure = export_netgen(*map, position_att);
	nglib::Ng_Meshing_Parameters* mp = setup_netgen_parameters(params);

	nglib::Ng_GenerateVolumeMesh (netgen_structure.get(), mp);
	delete mp;

	MapHandler3* handler_map3 = plugin_cmap_provider_->add_cmap3("netgen_export");
	NetgenStructureVolumeImport netgen_import(netgen_structure.get(), *handler_map3->map());
	netgen_import.create_map();

	handler_map3->attribute_added(CMap3::Vertex::ORBIT, "position");
	handler_map3->set_bb_vertex_attribute("position");
	handler_map3->create_vbo("position");

	return handler_map3;
}

Plugin_VolumeMeshFromSurface::MapHandler3* Plugin_VolumeMeshFromSurface::generate_tetgen(MapHandler2* mh2, CMap2::VertexAttribute<VEC3> position_att, const std::string& tetgen_args)
{
	if (!mh2 || !position_att.is_valid())
		return nullptr;

	Map2* map = mh2->map();

	auto tetgen_input = export_tetgen(*map, position_att);
	tetgen::tetgenio tetgen_output;

	tetgen::tetrahedralize(tetgen_args.c_str(), tetgen_input.get(), &tetgen_output);

	MapHandler3* handler_map3 = plugin_cmap_provider_->add_cmap3("tetgen_export");
	TetgenStructureVolumeImport tetgen_import(&tetgen_output, *handler_map3->map());
	tetgen_import.create_map();

	handler_map3->attribute_added(CMap3::Vertex::ORBIT, "position");
	handler_map3->set_bb_vertex_attribute("position");
	handler_map3->create_vbo("position");

	return handler_map3;
}

Plugin_VolumeMeshFromSurface::MapHandler3* Plugin_VolumeMeshFromSurface::generate_cgal(MapHandler2* mh2, CMap2::VertexAttribute<VEC3> position_att, const CGALParameters& params)
{
#ifdef PLUGIN_MESHGEN_WITH_CGAL
	if (!mh2 || !position_att.is_valid())
		return nullptr;

	CMap2& map2 = *mh2->map();
	bool is_triangular = true;
	map2.foreach_cell([&is_triangular, &map2](CMap2::Face f) -> bool
	{
		return is_triangular = (map2.codegree(f) == 3);
	});
	if (!is_triangular)
	{
		cgogn::modeling::triangule<CMap2>(map2, position_att);
		mh2->notify_connectivity_change();
		mh2->notify_attribute_change(CMap2::Vertex::ORBIT, QString::fromStdString(position_att.name()));
	}

	MapHandler3* mh3 = plugin_cmap_provider_->add_cmap3("cgal_export");
	tetrahedralize(params, mh2, position_att, mh3);
	return mh3;
#else
	return nullptr;
#endif // PLUGIN_MESHGEN_WITH_CGAL
}

#ifdef PLUGIN_MESHGEN_WITH_CGAL_IMAGEIO
Plugin_VolumeMeshFromSurface::MapHandler3* Plugin_VolumeMeshFromSurface::generate_cgal(const plugin_image::Image3D* im, const CGALParameters& params)
{
#ifdef PLUGIN_MESHGEN_WITH_CGAL
	if (!im)
		return nullptr;
	MapHandler3* mh3 = plugin_cmap_provider_->add_cmap3("cgal_image_export");
	tetrahedralize(params, im, mh3);
	return mh3;
#else
	return nullptr;
#endif // PLUGIN_MESHGEN_WITH_CGAL
}
#endif

void Plugin_VolumeMeshFromSurface::generate_button_cgal_pressed()
{
#ifdef PLUGIN_MESHGEN_WITH_CGAL
	MapHandler2* mh2 = plugin_cmap_provider_->cmap2(dialog_->get_selected_map());
	if (mh2)
	{
		const std::string& position_att_name = dialog_->export_dialog_->comboBoxPositionSelection->currentText().toStdString();
		auto position_att = mh2->map()->get_attribute<VEC3, CMap2::Vertex::ORBIT>(position_att_name);
		generate_cgal(mh2, position_att, generation_parameters_.cgal);

	} else {
		if (dialog_->export_dialog_->comboBox_images->currentIndex() > 0)
		{
			const QString& im_path = dialog_->export_dialog_->comboBox_images->currentText();
			if (plugin_image_)
			{
#ifdef PLUGIN_MESHGEN_WITH_CGAL_IMAGEIO
				if (plugin_image::Image3D const * im = plugin_image_->image(im_path))
					generate_cgal(im, generation_parameters_.cgal);
#endif
			}
		}
	}
#endif // PLUGIN_MESHGEN_WITH_CGAL
}



} // namespace plugin_meshgen
} // namespace schnapps
