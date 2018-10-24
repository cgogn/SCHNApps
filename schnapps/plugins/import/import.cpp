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

#include <schnapps/plugins/import/import.h>

#include <schnapps/plugins/cmap0_provider/cmap0_provider.h>
#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>
#include <schnapps/plugins/cmap3_provider/cmap3_provider.h>

#include <schnapps/core/schnapps.h>

#include <cgogn/io/map_import.h>

#include <QFileDialog>
#include <QFileInfo>

namespace schnapps
{

namespace plugin_import
{

Plugin_Import::Plugin_Import()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_Import::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_Import::enable()
{
	import_point_set_action_ = schnapps_->add_menu_action("Import;Point Set", "import point set");
	connect(import_point_set_action_, SIGNAL(triggered()), this, SLOT(import_point_set_from_file_dialog()));

	import_surface_mesh_action_ = schnapps_->add_menu_action("Import;Surface Mesh", "import surface mesh");
	connect(import_surface_mesh_action_, SIGNAL(triggered()), this, SLOT(import_surface_mesh_from_file_dialog()));

	import_volume_mesh_action_ = schnapps_->add_menu_action("Import;Volume Mesh", "import volume mesh");
	connect(import_volume_mesh_action_, SIGNAL(triggered()), this, SLOT(import_volume_mesh_from_file_dialog()));

	if (setting("Bounding box attribute").isValid())
		setting_bbox_name_ = setting("Bounding box attribute").toString();
	else
		setting_bbox_name_ = add_setting("Bounding box attribute", "position").toString();

	if (setting("Create VBO").isValid())
		setting_vbo_names_ = setting("Create VBO").toStringList();
	else
		setting_vbo_names_ = add_setting("Create VBO", QStringList({"position", "normal", "color"})).toStringList();

	if (setting("Default path").isValid())
		setting_default_path_ = setting("Default path").toString();
	else
		setting_default_path_ = add_setting("Default path", schnapps_->app_path() ).toString();

	plugin_cmap0_provider_ = reinterpret_cast<plugin_cmap0_provider::Plugin_CMap0Provider*>(schnapps_->enable_plugin(plugin_cmap0_provider::Plugin_CMap0Provider::plugin_name()));
	plugin_cmap2_provider_ = reinterpret_cast<plugin_cmap2_provider::Plugin_CMap2Provider*>(schnapps_->enable_plugin(plugin_cmap2_provider::Plugin_CMap2Provider::plugin_name()));
	plugin_cmap3_provider_ = reinterpret_cast<plugin_cmap3_provider::Plugin_CMap3Provider*>(schnapps_->enable_plugin(plugin_cmap3_provider::Plugin_CMap3Provider::plugin_name()));

	return true;
}

void Plugin_Import::disable()
{
	schnapps_->remove_menu_action(import_point_set_action_);
	schnapps_->remove_menu_action(import_surface_mesh_action_);
	schnapps_->remove_menu_action(import_volume_mesh_action_);
}

CMap0Handler* Plugin_Import::import_point_set_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		CMap0Handler* mh = plugin_cmap0_provider_->add_map(fi.baseName());
		if (mh)
		{
			CMap0* map = mh->map();

			cgogn::io::import_point_set<VEC3>(*map, filename.toStdString());

			mh->notify_connectivity_change();

			if (map->is_embedded<CMap0::Vertex>())
			{
				const auto& container = map->attribute_container<CMap0::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap0::Vertex::ORBIT, QString::fromStdString(names[i]));
			}

			if (map->nb_cells<CMap0::Vertex>() > 0)
			{
				for (const QString& vbo_name : setting_vbo_names_)
					mh->create_vbo(vbo_name);

				mh->set_bb_vertex_attribute(setting_bbox_name_);
			}
		}
		return mh;
	}
	else
		return nullptr;
}

void Plugin_Import::import_point_set_from_file_dialog()
{
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import surface meshes", setting_default_path_, "Surface mesh Files (*.plo)");
	QStringList::Iterator it = filenames.begin();

	if  (it != filenames.end())
	{
		QFileInfo info(*it);
		setting_default_path_ = info.path();
	}

	while (it != filenames.end())
	{
		import_point_set_from_file(*it);
		++it;
	}
}

CMap2Handler* Plugin_Import::import_surface_mesh_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		CMap2Handler* mh = plugin_cmap2_provider_->add_map(fi.baseName());
		if (mh)
		{
			CMap2* map = mh->map();

			cgogn::io::import_surface<VEC3>(*map, filename.toStdString());

			mh->notify_connectivity_change();

			if (map->is_embedded<CMap2::CDart>())
			{
				const auto& container = map->attribute_container<CMap2::CDart::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap2::CDart::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap2::Vertex>())
			{
				const auto& container = map->attribute_container<CMap2::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap2::Vertex::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap2::Edge>())
			{
				const auto& container = map->attribute_container<CMap2::Edge::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap2::Edge::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap2::Face>())
			{
				const auto& container = map->attribute_container<CMap2::Face::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap2::Face::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap2::Volume>())
			{
				const auto& container = map->attribute_container<CMap2::Volume::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap2::Volume::ORBIT, QString::fromStdString(names[i]));
			}

			if (map->nb_cells<CMap2::Vertex>() > 0)
			{
				for (const QString& vbo_name : setting_vbo_names_)
					mh->create_vbo(vbo_name);

				mh->set_bb_vertex_attribute(setting_bbox_name_);
			}
		}
		return mh;
	}
	else
		return nullptr;
}

void Plugin_Import::import_surface_mesh_from_file_dialog()
{
    QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import surface meshes", setting_default_path_, "Surface mesh Files (*.ply *.off *.stl *.trian *.vtk *.vtp *.vtu *.obj *.2dm *.msh *.mesh *.meshb)");
	QStringList::Iterator it = filenames.begin();

	if  (it != filenames.end())
	{
		QFileInfo info(*it);
		setting_default_path_ = info.path();
	}

	while (it != filenames.end())
	{
		import_surface_mesh_from_file(*it);
		++it;
	}
}

CMap3Handler* Plugin_Import::import_volume_mesh_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		CMap3Handler* mh = plugin_cmap3_provider_->add_map(fi.baseName());
		if (mh)
		{
			CMap3* map = mh->map();

			cgogn::io::import_volume<VEC3>(*map, filename.toStdString());

			mh->notify_connectivity_change();

			if (map->is_embedded<CMap3::CDart>())
			{
				const auto& container = map->attribute_container<CMap3::CDart::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap3::CDart::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap3::Vertex>())
			{
				const auto& container = map->attribute_container<CMap3::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap3::Vertex::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap3::Edge>())
			{
				const auto& container = map->attribute_container<CMap3::Edge::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap3::Edge::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap3::Face>())
			{
				const auto& container = map->attribute_container<CMap3::Face::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap3::Face::ORBIT, QString::fromStdString(names[i]));
			}
			if (map->is_embedded<CMap3::Volume>())
			{
				const auto& container = map->attribute_container<CMap3::Volume::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap3::Volume::ORBIT, QString::fromStdString(names[i]));
			}

			if (mh->map()->nb_cells<CMap3::Vertex>() > 0)
			{
				for (const QString& vbo_name : setting_vbo_names_)
					mh->create_vbo(vbo_name);

				mh->set_bb_vertex_attribute(setting_bbox_name_);
			}
		}
		return mh;
	}
	else
		return nullptr;
}

void Plugin_Import::import_volume_mesh_from_file_dialog()
{
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import volume meshes", setting_default_path_, "Volume mesh Files (*.msh *.vtu *.vtk *.nas *.bdf *.ele *.tetmesh *.node *.mesh *.meshb *.tet)");
	QStringList::Iterator it = filenames.begin();

	if  (it != filenames.end())
	{
		QFileInfo info(*it);
		setting_default_path_ = info.path();
	}

	while (it != filenames.end())
	{
		import_volume_mesh_from_file(*it);
		++it;
	}
}

} // namespace plugin_import

} // namespace schnapps
