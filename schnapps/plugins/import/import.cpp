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

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

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
	import_point_set_action_ = schnapps_->add_menu_action("File;Import;Point Set", "import point set");
	import_polyline_action_ = schnapps_->add_menu_action("File;Import;Polyline", "import polyline");
	import_graph_action_ = schnapps_->add_menu_action("File;Import;Graph", "import graph");
	import_surface_mesh_action_ = schnapps_->add_menu_action("File;Import;Surface Mesh", "import surface mesh");
	import_volume_mesh_action_ = schnapps_->add_menu_action("File;Import;Volume Mesh", "import volume mesh");

	connect(import_point_set_action_, SIGNAL(triggered()), this, SLOT(import_point_set_from_file_dialog()));
	connect(import_polyline_action_, SIGNAL(triggered()), this, SLOT(import_polyline_from_file_dialog()));
	connect(import_graph_action_, SIGNAL(triggered()), this, SLOT(import_graph_from_file_dialog()));
	connect(import_surface_mesh_action_, SIGNAL(triggered()), this, SLOT(import_surface_mesh_from_file_dialog()));
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

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));


	point_set_filter_ = QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::point_set_file_type_map, " "));
	polyline_filter_ = QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::polyline_file_type_map,  " "));
	graph_filter_ = QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::graph_file_type_map, " "));
	surface_filter_ = QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::surface_file_type_map, " "));
	volume_filter_ = QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::volume_file_type_map, " "));

	return true;
}



void Plugin_Import::disable()
{
	disconnect(import_point_set_action_, SIGNAL(triggered()), this, SLOT(import_point_set_from_file_dialog()));
	disconnect(import_polyline_action_, SIGNAL(triggered()), this, SLOT(import_polyline_from_file_dialog()));
	disconnect(import_graph_action_, SIGNAL(triggered()), this, SLOT(import_graph_from_file_dialog()));
	disconnect(import_surface_mesh_action_, SIGNAL(triggered()), this, SLOT(import_surface_mesh_from_file_dialog()));
	disconnect(import_volume_mesh_action_, SIGNAL(triggered()), this, SLOT(import_volume_mesh_from_file_dialog()));

	schnapps_->remove_menu_action(import_point_set_action_);
	schnapps_->remove_menu_action(import_polyline_action_);
	schnapps_->remove_menu_action(import_graph_action_);
	schnapps_->remove_menu_action(import_surface_mesh_action_);
	schnapps_->remove_menu_action(import_volume_mesh_action_);
}

CMap0Handler* Plugin_Import::import_point_set_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		CMap0Handler* mh = plugin_cmap_provider_->add_cmap0(fi.baseName());
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
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import point sets", setting_default_path_, "Point set Files ("+point_set_filter_+")");
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

CMap1Handler* Plugin_Import::import_polyline_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		CMap1Handler* mh = plugin_cmap_provider_->add_cmap1(fi.baseName());
		if (mh)
		{
			CMap1* map = mh->map();

			cgogn::io::import_polyline<VEC3>(*map, filename.toStdString());

			mh->notify_connectivity_change();

			if (map->is_embedded<CMap1::Vertex>())
			{
				const auto& container = map->attribute_container<CMap1::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					mh->notify_attribute_added(CMap1::Vertex::ORBIT, QString::fromStdString(names[i]));
			}

			if (map->nb_cells<CMap1::Vertex>() > 0)
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

void Plugin_Import::import_polyline_from_file_dialog()
{
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import polyline meshes", setting_default_path_, "Polyline mesh Files ("+polyline_filter_+")");
	QStringList::Iterator it = filenames.begin();

	if  (it != filenames.end())
	{
		QFileInfo info(*it);
		setting_default_path_ = info.path();
	}

	while (it != filenames.end())
	{
		import_polyline_from_file(*it);
		++it;
	}
}

UndirectedGraphHandler* Plugin_Import::import_graph_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		UndirectedGraphHandler* ugh = plugin_cmap_provider_->add_undirected_graph(fi.baseName());
		if (ugh)
		{
			UndirectedGraph* ug = ugh->map();

			cgogn::io::import_graph<VEC3>(*ug, filename.toStdString());

			ugh->notify_connectivity_change();

			if (ug->is_embedded<UndirectedGraph::Vertex>())
			{
				const auto& container = ug->attribute_container<UndirectedGraph::Vertex::ORBIT>();
				const std::vector<std::string>& names = container.names();
				for (std::size_t i = 0u; i < names.size(); ++i)
					ugh->notify_attribute_added(UndirectedGraph::Vertex::ORBIT, QString::fromStdString(names[i]));
			}

			if (ug->nb_cells<UndirectedGraph::Vertex>() > 0)
			{
				for (const QString& vbo_name : setting_vbo_names_)
					ugh->create_vbo(vbo_name);

				ugh->set_bb_vertex_attribute(setting_bbox_name_);
			}
		}
		return ugh;
	}
	else
		return nullptr;
}

void Plugin_Import::import_graph_from_file_dialog()
{
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import graph", setting_default_path_, "Graph Files ("+graph_filter_+")");
	QStringList::Iterator it = filenames.begin();

	if  (it != filenames.end())
	{
		QFileInfo info(*it);
		setting_default_path_ = info.path();
	}

	while (it != filenames.end())
	{
		import_graph_from_file(*it);
		++it;
	}
}

CMap2Handler* Plugin_Import::import_surface_mesh_from_file(const QString& filename)
{
	QFileInfo fi(filename);
	if (fi.exists())
	{
		CMap2Handler* mh = plugin_cmap_provider_->add_cmap2(fi.baseName());
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

	qDebug() << surface_filter_;

	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import surface meshes", setting_default_path_, "Surface mesh Files ("+surface_filter_+")");
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
		CMap3Handler* mh = plugin_cmap_provider_->add_cmap3(fi.baseName());
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
	QStringList filenames = QFileDialog::getOpenFileNames(nullptr, "Import volume meshes", setting_default_path_, "Volume mesh Files ("+volume_filter_+")");
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
