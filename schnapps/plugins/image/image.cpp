/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Image                                                                 *
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

#include <schnapps/plugins/image/image.h>
#include <schnapps/plugins/image/image_dock_tab.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>

#include <cgogn/io/c_locale.h>
#include <cgogn/io/formats/vtk.h>
#include <cgogn/core/utils/string.h>

#include <QFileDialog>
#include <QFileInfo>
#include <QDir>

#ifdef SCHNAPPS_PLUGIN_IMAGE_WITH_BOOST_IOSTREAM
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#endif // SCHNAPPS_PLUGIN_IMAGE_WITH_BOOST_IOSTREAM

namespace schnapps
{

namespace plugin_image
{

Image3D::value_type const& Image3D::operator()(std::size_t i, std::size_t j, std::size_t k) const
{
	return static_cast<const value_type*>(data_->data())[get_nb_components() * ((k * image_dim_[1] + j) * image_dim_[0] + i)];
}



Plugin_Image::Plugin_Image() :
	plugin_cmap_provider_(nullptr),
	import_image_action_(nullptr),
	dock_tab_(nullptr),
	export_point_set_threshold_(0.01)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

Plugin_Image::~Plugin_Image()
{}

QString Plugin_Image::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_Image::enable()
{
	import_image_action_ = schnapps_->add_menu_action("Import;3D Image", "import image");
	connect(import_image_action_, SIGNAL(triggered()), this, SLOT(import_image_dialog()));

	dock_tab_ = new Image_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Image3D");

	dock_tab_->threshold_spinbox->setValue(export_point_set_threshold_);
	connect(dock_tab_->threshold_spinbox, SIGNAL(valueChanged(double)), this, SLOT(threshold_changed(double)));
	return true;
}

void Plugin_Image::disable()
{
	schnapps_->remove_menu_action(import_image_action_);
	import_image_action_ = nullptr;
	schnapps_->remove_plugin_dock_tab(this, dock_tab_);
	delete dock_tab_;
	dock_tab_ = nullptr;
}

void Plugin_Image::add_image(const QString& image_path)
{
	QFileInfo fileinfo(image_path);
	if (fileinfo.exists() && fileinfo.isFile())
	{
		const QString name = fileinfo.baseName();
		QString final_name = name;

		if (objects_.count(name) > 0ul)
		{
			int i = 1;
			do
			{
				final_name = name + QString("_") + QString::number(i);
				++i;
			} while (objects_.count(final_name) > 0ul);
		}

		Image3D* im = Image3D::new_image_3d(image_path, final_name, this);
		if (!im || im->is_empty())
		{
			delete im;
			return;
		}

		objects_.insert(std::make_pair(im->name(), im));
		schnapps_->notify_object_added(im);
	}
}

const Image3D*Plugin_Image::image(const QString& im_name) const
{
	if (objects_.count(im_name) > 0ul)
		return dynamic_cast<Image3D*>(objects_.at(im_name));
	else
		return nullptr;
}

void Plugin_Image::import_image_dialog()
{
	auto filenames = QFileDialog::getOpenFileNames(nullptr, "Import 3D images", schnapps_->app_path(),  "3DImages (*.inr *.vtk *.inr.gz *.vtk.gz)");
	for (const auto& im : filenames)
		add_image(im);
}

void Plugin_Image::threshold_changed(double t)
{
	export_point_set_threshold_ = t;
}

void Plugin_Image::image_removed(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		Image3D* im = dynamic_cast<Image3D*>(objects_.at(name));
		if (im)
		{
			auto items = dock_tab_->listWidget_images->findItems(name, Qt::MatchExactly);
			for (QListWidgetItem* item : items)
			{
				dock_tab_->listWidget_images->removeItemWidget(item);
				delete item;
			}
			schnapps_->notify_object_removed(im);
			objects_.erase(name);
			delete im;
		}
	}
}

void Plugin_Image::export_image_to_point_set(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		Image3D* im = dynamic_cast<Image3D*>(objects_.at(name));
		if (im)
		{
			if (!plugin_cmap_provider_)
				plugin_cmap_provider_  = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
			plugin_cmap_provider::CMap0Handler* mh = plugin_cmap_provider_->add_cmap0(im->name());
			auto* map = mh->map();
			ImagePointSetImport importer(*map);
			if (importer.import_image(*im, export_point_set_threshold_))
			{
				importer.create_map();
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
					mh->create_vbo("position");
					mh->set_bb_vertex_attribute("position");
				}
			} else {
				cgogn_log_warning("Plugin_Image") << "Unable to export the image \"" << im->name().toStdString() << "\" to point set.";
			}
		}
	}
}

Image3D::Image3D(const QString& name, PluginProvider* p) : Object(name, p),
	data_(nullptr),
	image_dim_{},
	origin_(),
	voxel_dim_{},
	translation_(),
	rotation_(),
	nb_components_(0u)
{
	image_dim_.fill(0);
	translation_.fill(0);
	rotation_.fill(0);
	origin_.fill(0);
	voxel_dim_.fill(0);
}


Image3D* Image3D::new_image_3d(const QString& image_path, const QString& objectname, PluginProvider* p)
{
	QFileInfo fileinfo(image_path);
	Image3D* res_img = new Image3D(objectname, p);

	if (fileinfo.exists() && fileinfo.isFile())
	{
		cgogn::Scoped_C_Locale locale;
		std::ifstream in_file(image_path.toStdString(), std::ios_base::binary | std::ios_base::in);
		const QString complete_suffix = fileinfo.completeSuffix();
		if (!QString::compare(fileinfo.suffix(), "gz", Qt::CaseInsensitive))
		{
			const QString temp_image_path = uncompress_gz_file(image_path);
			if (QFileInfo::exists(temp_image_path))
			{
				delete res_img;
				res_img = Image3D::new_image_3d(temp_image_path, objectname, p);
				QFile::remove(temp_image_path);
			}
		}
		else
		{
			if (!QString::compare(complete_suffix, "inr", Qt::CaseInsensitive))
				res_img->import_inr(in_file);
			else
			{
				if (!QString::compare(complete_suffix, "vtk", Qt::CaseInsensitive))
					res_img->import_vtk(in_file);
			}
		}
	}

	return res_img;
}

void Image3D::import_inr(std::istream& inr_data)
{
	std::array<char, 256> buffer;
	std::string line;
	std::string type;
	std::string cpu;
	std::size_t data_size = 0ul;

	inr_data.read(&buffer[0], buffer.size());
	std::stringstream sstream(std::string(&buffer[0], buffer.size()));
	std::getline(sstream, line);
	if (line != "#INRIMAGE-4#{")
		return;

	std::getline(sstream, line);

	while (line != "##}")
	{
		if (line.empty())
		{
			std::getline(sstream, line);
			continue;
		}

		if (line.compare(0,5,"XDIM=") == 0)
			this->image_dim_[0] = std::stoul(line.substr(5));
		if (line.compare(0,5,"YDIM=") == 0)
			this->image_dim_[1] = std::stoul(line.substr(5));
		if (line.compare(0,5,"ZDIM=") == 0)
			this->image_dim_[2] = std::stoul(line.substr(5));

		if (line.compare(0,5,"VDIM=") == 0)
			this->nb_components_ = std::stoul(line.substr(5));

		if (line.compare(0,3,"VX=") == 0)
			this->voxel_dim_[0] = std::stod(line.substr(3));
		if (line.compare(0,3,"VY=") == 0)
			this->voxel_dim_[1] = std::stod(line.substr(3));
		if (line.compare(0,3,"VZ=") == 0)
			this->voxel_dim_[2] = std::stod(line.substr(3));

		if (line.compare(0,3,"XO=") == 0)
			this->origin_[0] = std::stod(line.substr(3));
		if (line.compare(0,3,"YO=") == 0)
			this->origin_[1] = std::stod(line.substr(3));
		if (line.compare(0,3,"ZO=") == 0)
			this->origin_[2] = std::stod(line.substr(3));

		if (line.compare(0,3,"TX=") == 0)
			this->translation_[0] = std::stod(line.substr(3));
		if (line.compare(0,3,"TY=") == 0)
			this->translation_[1] = std::stod(line.substr(3));
		if (line.compare(0,3,"TZ=") == 0)
			this->translation_[2] = std::stod(line.substr(3));

		if (line.compare(0,3,"RX=") == 0)
			this->rotation_[0] = std::stod(line.substr(3));
		if (line.compare(0,3,"RY=") == 0)
			this->rotation_[1] = std::stod(line.substr(3));
		if (line.compare(0,3,"RZ=") == 0)
			this->rotation_[2] = std::stod(line.substr(3));

		if (line.compare(0,5,"TYPE=") == 0)
			type = line.substr(5);

		if (line.compare(0,8,"PIXSIZE=") == 0)
			data_size = std::stoul(line.substr(8,2))/8ul;

		if (line.compare(0,4,"CPU=") == 0)
			cpu = line.substr(4);

		std::getline(sstream, line);
	}

	cpu = cgogn::to_lower(cpu);
	type = cgogn::to_lower(type);

	const bool little_endian = (cpu == "decm" || cpu == "pc" || cpu == "alpha");

	if (type == "unsigned fixed")
	{
		switch (data_size)
		{
			case 1: data_ = cgogn::make_unique<DataInput<uint8, value_type>>(); break;
			case 2: data_ = cgogn::make_unique<DataInput<uint16, value_type>>(); break;
			case 4: data_ = cgogn::make_unique<DataInput<uint32, value_type>>(); break;
			case 8: data_ = cgogn::make_unique<DataInput<uint64, value_type>>(); break;
			default: break;
		}
	}
	else if (type == "signed fixed")
	{
		switch (data_size)
		{
			case 1: data_ = cgogn::make_unique<DataInput<int8, value_type>>(); break;
			case 2: data_ = cgogn::make_unique<DataInput<int16, value_type>>(); break;
			case 4: data_ = cgogn::make_unique<DataInput<int32, value_type>>(); break;
			case 8: data_ = cgogn::make_unique<DataInput<int64, value_type>>(); break;
			default: break;
		}
	}
	else if (type == "float")
	{
		switch (data_size)
		{
			case 4: data_ = cgogn::make_unique<DataInput<float32, value_type>>(); break;
			case 8: data_ = cgogn::make_unique<DataInput<float64, value_type>>(); break;
			default: break;
		}
	}

	if (data_)
	{
		data_->read_n(inr_data, image_dim_[0] * image_dim_[1] * image_dim_[2] * nb_components_ , true, !little_endian);
		data_ = data_->simplify();
	}
}

void Image3D::import_vtk(std::istream& vtk_data)
{
	const auto to_upper = [&](const std::string& s) { return cgogn::to_upper(s); };

	std::string line;
	std::string word;
	line.reserve(512);
	word.reserve(128);

	// 2 first lines = trash
	std::getline(vtk_data, line);
	std::getline(vtk_data, line);

	vtk_data >> word;
	bool ascii_file = to_upper(word) == "ASCII";
	if (!(ascii_file || to_upper(word) == "BINARY"))
		return;

	vtk_data >> word;
	if (to_upper(word) != "DATASET")
		return;

	vtk_data >> word;
	const std::string& dataset = to_upper(word);
	if (dataset != "STRUCTURED_POINTS")
		return;

	origin_.fill(0);

	while (!vtk_data.eof())
	{
		std::getline(vtk_data,line);
		word.clear();
		std::istringstream sstream(line);
		sstream >> word;
		word = to_upper(word);

		if (word == "DIMENSIONS")
			sstream >> image_dim_[0] >> image_dim_[1] >> image_dim_[2];

		if (word == "SPACING")
			sstream >> voxel_dim_[0] >> voxel_dim_[1] >> voxel_dim_[2];

		if (word == "ORIGIN")
			sstream >> origin_[0] >> origin_[1] >> origin_[2];

		if (word == "POINT_DATA")
		{
			std::size_t nb_data;
			sstream >> nb_data;

			std::ifstream::pos_type previous_pos;
			do
			{
				previous_pos = vtk_data.tellg();
				std::getline(vtk_data, line);
				sstream.str(line);
				sstream.clear();
				word.clear();
				sstream >> word;
				word = to_upper(word);
				if (word == "SCALARS" || word == "VECTOR")
				{
					const bool is_vector = (word != "SCALARS");
					std::string att_name;
					std::string att_type;
					nb_components_ = is_vector? 3u : 1u;
					sstream >> att_name >> att_type >> nb_components_;
					att_type = cgogn::io::vtk_data_type_to_cgogn_name_of_type(att_type);

					const auto pos_before_lookup_table = vtk_data.tellg(); // the lookup table might (or might not) be defined
					std::getline(vtk_data,line);
					sstream.str(line);
					sstream.clear();
					std::string lookup_table;
					std::string lookup_table_name;
					sstream >> lookup_table >> lookup_table_name;
					if (to_upper(lookup_table) != "LOOKUP_TABLE")
						vtk_data.seekg(pos_before_lookup_table); // if there wasn't a lookup table we go back and start reading the numerical values

					data_ = DataInputGen::template newDataIO<1, value_type>(att_type, nb_components_);
					data_->read_n(vtk_data, nb_data, !ascii_file, true);
					data_ = data_->simplify();
				}
				else
				{
					if (word == "LOOKUP_TABLE")
					{
						std::string table_name;
						/*uint32*/ nb_data = 0u;
						sstream >> table_name >> nb_data;
						if (ascii_file)
						{
							DataInput<Eigen::Vector4f> trash;
							trash.skip_n(vtk_data, nb_data, false);
						}
						else
						{
							DataInput<std::int32_t> trash;
							trash.skip_n(vtk_data, nb_data, true);
						}
					}
				}
			} while ((word == "SCALARS" || word == "LOOKUP_TABLE" || word == "VECTOR")&& (!vtk_data.eof()));
			if (!vtk_data.eof())
			{
				vtk_data.seekg(previous_pos);
				word.clear();
			}
			else
				break;
		}
	}
}

void Image3D::view_linked(View* /*view*/)
{

}

void Image3D::view_unlinked(View* /*view*/)
{

}

SCHNAPPS_PLUGIN_IMAGE_API QString uncompress_gz_file(const QString& filename_in)
{
#ifdef SCHNAPPS_PLUGIN_IMAGE_WITH_BOOST_IOSTREAM
	if (!QFileInfo::exists(filename_in))
		return QString();

	const QString filename_out = QDir::cleanPath(QDir::tempPath() + QDir::separator() + QFileInfo(filename_in).completeBaseName());
	if (QFileInfo::exists(filename_out))
		QFile::remove(filename_out);

	std::ifstream in_file(filename_in.toStdString(), std::ios_base::binary | std::ios_base::in);
	std::ofstream out_file(filename_out.toStdString(), std::ios_base::binary | std::ios_base::out);
	if (!in_file || !out_file)
		return QString();

	boost::iostreams::filtering_streambuf<boost::iostreams::input> filt_in;
	filt_in.push(boost::iostreams::gzip_decompressor());
	filt_in.push(in_file);
	boost::iostreams::copy(filt_in, out_file);
	return filename_out;
#else
	cgogn_log_warning("schnapps::plugin_image::uncompress_gz_file") << "The plugin need boost_iostreams to uncompress .gz files.";
	return QString();
#endif // SCHNAPPS_PLUGIN_IMAGE_WITH_BOOST_IOSTREAM
}


ImagePointSetImport::ImagePointSetImport(CMap0& map) :
	Inherit(map)
{}

ImagePointSetImport::~ImagePointSetImport()
{}

bool ImagePointSetImport::import_image(const Image3D& im, double threshold)
{
	ChunkArray<VEC3>* position = this->template add_vertex_attribute<VEC3>("position");

	// reading number of points
	const std::size_t nx = im.get_image_dimensions()[0];
	const std::size_t ny = im.get_image_dimensions()[1];
	const std::size_t nz = im.get_image_dimensions()[2];

	for (std::size_t i = 0 ; i < nx ; ++i)
	{
		for (std::size_t j = 0 ; j < ny  ; ++j)
		{
			for (std::size_t k = 0 ; k < nz ; ++k)
			{
				if (std::fabs(double(im(i,j,k)) >= threshold))
				{
					++nb_vertices_;
					const auto p = im.position(i,j,k);
					const uint32 vertex_id = this->insert_line_vertex_container();
					(*position)[vertex_id] = VEC3(p[0], p[1], p[2]);
				}
			}
		}
	}
	return true;
}

} // namespace plugin_image

} // namespace schnapps
