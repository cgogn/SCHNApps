/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Mesh From Surface                                              *
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

#define SCHNAPPS_PLUGIN_VMFS_DLL_EXPORT

#include <volume_mesh_from_surface.h>
#include <tetgen_structure_io.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <cgogn/core/utils/unique_ptr.h>
#include <cgogn/io/map_export.h>
#ifdef PLUGIN_VMFS_WITH_CGAL
#include <cgal/c3t3_import.h>
#endif // PLUGIN_VMFS_WITH_CGAL
#include <image.h>
#include <tetgen/tetgen.h>

namespace schnapps
{

namespace plugin_vmfs
{

MeshGeneratorParameters::MeshGeneratorParameters() :
	tetgen_command_line("-pqY"),
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
{}

Plugin_VolumeMeshFromSurface::Plugin_VolumeMeshFromSurface() :
	gen_mesh_action_(nullptr),
	plugin_image_(nullptr),
	generation_parameters_(),
	dialog_(nullptr)
{}

bool Plugin_VolumeMeshFromSurface::enable()
{
	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(plugin_enabled(Plugin*)));

	if (!dialog_)
		dialog_ = cgogn::make_unique<VolumeMeshFromSurfaceDialog>(schnapps_, this);

	gen_mesh_action_ = schnapps_->add_menu_action("Export;Tetrahedralize", "Tetrahedralize");
	connect(gen_mesh_action_, SIGNAL(triggered()), dialog_.get(), SLOT(show_export_dialog()));

	schnapps_->foreach_map([&](MapHandlerGen* mhg)
	{
		dialog_->map_added(mhg);
	});

	Plugin* image_plugin_gen = schnapps_->get_plugin("image");
	plugin_enabled(image_plugin_gen);

	if (plugin_image_)
		for (const auto& im : plugin_image_->get_images())
			dialog_->image_added(im.first);

	return true;
}

void Plugin_VolumeMeshFromSurface::disable()
{
	schnapps_->remove_menu_action(gen_mesh_action_);
}

void Plugin_VolumeMeshFromSurface::generate_button_tetgen_pressed()
{
	MapHandlerGen* mhg = schnapps_->get_map(dialog_->get_selected_map());
	MapHandler2* handler_map2 = dynamic_cast<MapHandler2*>(mhg);
	if (handler_map2)
	{
		Map2* map = handler_map2->get_map();
		const std::string& position_att_name = dialog_->export_dialog_->comboBoxPositionSelection->currentText().toStdString();
		auto position_att = map->template get_attribute<VEC3, Map2::Vertex::ORBIT>(position_att_name);

		if (!position_att.is_valid())
		{
			cgogn_log_info("Plugin_VolumeMeshFromSurface") << "The position attribute has to be of type VEC3.";
			return;
		}
		const std::string& tetgen_command_line = generation_parameters_.tetgen_command_line;
		generate_tetgen(handler_map2, position_att, tetgen_command_line.c_str());
	}
}

Plugin_VolumeMeshFromSurface::MapHandler3* Plugin_VolumeMeshFromSurface::generate_tetgen(MapHandler2* mh2, CMap2::Attribute<VEC3, CMap2::Vertex::ORBIT> position_att, const std::string& tetgen_args)
{
	if (!mh2 || !position_att.is_valid())
		return nullptr;

	Map2* map = mh2->get_map();

	auto tetgen_input = export_tetgen(*map, position_att);
	tetgen::tetgenio tetgen_output;

	tetgen::tetrahedralize(tetgen_args.c_str(), tetgen_input.get(), &tetgen_output);

	TetgenStructureVolumeImport tetgen_import(&tetgen_output);
	tetgen_import.import_file("");

	MapHandler3* handler_map3 = dynamic_cast<MapHandler3*>(schnapps_->add_map("tetgen_export", 3));
	tetgen_import.create_map(*handler_map3->get_map());

	return handler_map3;
}

Plugin_VolumeMeshFromSurface::MapHandler3* Plugin_VolumeMeshFromSurface::generate_cgal(MapHandler2* mh2, CMap2::Attribute<VEC3, CMap2::Vertex::ORBIT> position_att, const MeshGeneratorParameters& params)
{
#ifdef PLUGIN_VMFS_WITH_CGAL
	if (!mh2 || !position_att.is_valid())
		return nullptr;

	MapHandler3* mh3 = dynamic_cast<MapHandler3*>(schnapps_->add_map("cgal_export", 3));
	tetrahedralize(params, mh2, position_att, mh3);
	return mh3;
#else
	return nullptr;
#endif // PLUGIN_VMFS_WITH_CGAL
}

Plugin_VolumeMeshFromSurface::MapHandler3* Plugin_VolumeMeshFromSurface::generate_cgal(const plugin_image::Image3D* im, const MeshGeneratorParameters& params)
{
#ifdef PLUGIN_VMFS_WITH_CGAL
	if (!im)
		return nullptr;
	MapHandler3* mh3 = dynamic_cast<MapHandler3*>(schnapps_->add_map("cgal_image_export", 3));
	tetrahedralize(params, im, mh3);
	return mh3;
#else
	return nullptr;
#endif // PLUGIN_VMFS_WITH_CGAL
}

void Plugin_VolumeMeshFromSurface::generate_button_cgal_pressed()
{
#ifdef PLUGIN_VMFS_WITH_CGAL
	MapHandlerGen* mhg = schnapps_->get_map(dialog_->get_selected_map());
	if (mhg)
	{
		MapHandler2* mh2 = dynamic_cast<MapHandler2*>(mhg);
		const std::string& position_att_name = dialog_->export_dialog_->comboBoxPositionSelection->currentText().toStdString();
		auto position_att = mh2->template get_attribute<VEC3, Map2::Vertex::ORBIT>(QString::fromStdString(position_att_name));
		generate_cgal(mh2, position_att, generation_parameters_);

	} else {
		if (dialog_->export_dialog_->comboBox_images->currentIndex() > 0)
		{
			const QString& im_path = dialog_->export_dialog_->comboBox_images->currentText();
			if (plugin_image_)
			{
				plugin_image::Image3D const * im = plugin_image_->get_image(im_path);
				if (im)
				{
					generate_cgal(im, generation_parameters_);
				}
			}
		}
	}
#endif // PLUGIN_VMFS_WITH_CGAL
}

void Plugin_VolumeMeshFromSurface::plugin_enabled(Plugin* plugin)
{
	if (!plugin_image_)
	{
		plugin_image_ = dynamic_cast<plugin_image::Plugin_Image*>(plugin);
		if (plugin_image_)
		{
				connect(plugin_image_, SIGNAL(image_added(QString)), dialog_.get(), SLOT(image_added(QString)));
				connect(plugin_image_, SIGNAL(image_removed(QString)), dialog_.get(), SLOT(image_removed(QString)));
		}
	}


}



} // namespace plugin_vmfs
} // namespace schnapps
