/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
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

#include <import.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/io/map_import.h>

#include <QFileDialog>
#include <QFileInfo>

namespace schnapps
{

namespace plugin_import
{

bool Plugin_Import::enable()
{
	import_surface_mesh_action_ = schnapps_->add_menu_action("Import;Surface Mesh", "import surface mesh");
	connect(import_surface_mesh_action_, SIGNAL(triggered()), this, SLOT(import_surface_mesh_from_file_dialog()));

	import_volume_mesh_action_ = schnapps_->add_menu_action("Import;Volume Mesh", "import volume mesh");
	connect(import_volume_mesh_action_, SIGNAL(triggered()), this, SLOT(import_volume_mesh_from_file_dialog()));

	//	import_2D_image_action_ = schnapps_->add_menu_action("Surface;Import 2D Image", "import 2D image");
	//	connect(import_2D_image_action_, SIGNAL(triggered()), this, SLOT(import_2D_image_from_file_dialog()));

	if (get_setting("Bounding box attribute").isValid())
		setting_bbox_name_ = get_setting("Bounding box attribute").toString();
	else
		setting_bbox_name_ = add_setting("Bounding box attribute", "position").toString();

	if (get_setting("Compute VBO").isValid())
		setting_vbo_names_ = get_setting("Compute VBO").toStringList();
	else
		setting_vbo_names_ = add_setting("Compute VBO", QStringList({"position", "normal", "color"})).toStringList();

	return true;
}

void Plugin_Import::disable()
{
	schnapps_->remove_menu_action(import_surface_mesh_action_);
	schnapps_->remove_menu_action(import_volume_mesh_action_);
	//	schnapps_->remove_menu_action(import_2D_image_action_);
}

MapHandlerGen* Plugin_Import::import_surface_mesh_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		MapHandlerGen* mhg = schnapps_->add_map(fi.baseName(), 2);
		if (mhg)
		{
			CMap2Handler* mh = static_cast<CMap2Handler*>(mhg);
			CMap2* map = mh->get_map();

			cgogn::io::import_surface<VEC3>(*map, filename.toStdString());
			if (mhg->nb_cells(CellType::Vertex_Cell) > 0)
			{
				mh->set_bb_vertex_attribute(setting_bbox_name_);

				for (const QString& vbo_name : setting_vbo_names_)
					mhg->create_vbo(vbo_name);
			}
			//			for (unsigned int orbit = VERTEX; orbit <= VOLUME; orbit++)
			//			{
			//				AttributeContainer& cont = map->getAttributeContainer(orbit);
			//				std::vector<std::string> names;
			//				std::vector<std::string> types;
			//				cont.getAttributesNames(names);
			//				cont.getAttributesTypes(types);
			//				for(unsigned int i = 0; i < names.size(); ++i)
			//					mhg->registerAttribute(orbit, QString::fromStdString(names[i]), QString::fromStdString(types[i]));
			//			}
		}
		return mhg;
	}
	else
		return nullptr;
}

void Plugin_Import::import_surface_mesh_from_file_dialog()
{
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import surface meshes", schnapps_->get_app_path(), "Surface mesh Files (*.ply *.off *.stl *.trian *.vtk *.vtp *.vtu *.obj *.msh *.mesh *.meshb)");
	QStringList::Iterator it = filenames.begin();
	while (it != filenames.end())
	{
		import_surface_mesh_from_file(*it);
		++it;
	}
}

MapHandlerGen* Plugin_Import::import_volume_mesh_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		MapHandlerGen* mhg = schnapps_->add_map(fi.baseName(), 3);
		if (mhg)
		{
			CMap3Handler* mh = static_cast<CMap3Handler*>(mhg);
			CMap3* map = mh->get_map();

			cgogn::io::import_volume<VEC3>(*map, filename.toStdString());
			if (mhg->nb_cells(CellType::Vertex_Cell) > 0)
			{
				mh->set_bb_vertex_attribute(setting_bbox_name_);

				for (const QString& vbo_name : setting_vbo_names_)
					mhg->create_vbo(vbo_name);
			}

		}
		return mhg;
	}
	else
		return nullptr;
}

void Plugin_Import::import_volume_mesh_from_file_dialog()
{
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import volume meshes", schnapps_->get_app_path(), "Volume mesh Files (*.msh *.vtu *.vtk *.nas *.bdf *.ele *.tetmesh *.node *.mesh *.meshb *.tet)");
	QStringList::Iterator it = filenames.begin();
	while (it != filenames.end())
	{
		import_volume_mesh_from_file(*it);
		++it;
	}
}

} // namespace plugin_import

} // namespace schnapps
