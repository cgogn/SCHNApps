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

#include "image.h"
#include <QFileDialog>
#include <QFileInfo>
#include <image_dock_tab.h>
#include <schnapps/core/schnapps.h>
#include <cgogn/io/c_locale.h>
#include <cgogn/io/vtk_io.h>
#include <cgogn/core/utils/string.h>

namespace schnapps
{

namespace plugin_image
{

Plugin_Image::Plugin_Image() :
	images_(),
	import_image_action_(nullptr),
	dock_tab_(nullptr)
{}

const std::list<std::pair<QString, Image3D> >& Plugin_Image::get_images() const
{
	return images_;
}

const Image3D*Plugin_Image::get_image(const QString& im_path)
{
	if (images_.empty())
		return nullptr;
	for (const auto& it : images_)
		if (it.first == im_path)
			return &(it.second);
	return nullptr;
}

bool Plugin_Image::enable()
{
	import_image_action_ = schnapps_->add_menu_action("Import;3D Image", "import image");
	connect(import_image_action_, SIGNAL(triggered()), this, SLOT(import_image_dialog()));

	dock_tab_ = new Image_DockTab(this->schnapps_, this);
	schnapps_->add_plugin_dock_tab(this, dock_tab_, "Image3D");
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

void Plugin_Image::draw(schnapps::View* view, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
}

void Plugin_Image::draw_map(schnapps::View* view, schnapps::MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
}

void Plugin_Image::keyPress(schnapps::View* view, QKeyEvent* event)
{
}

void Plugin_Image::keyRelease(schnapps::View* view, QKeyEvent* event)
{
}

void Plugin_Image::mousePress(schnapps::View* view, QMouseEvent* event)
{
}

void Plugin_Image::mouseRelease(schnapps::View* view, QMouseEvent* event)
{
}

void Plugin_Image::mouseMove(schnapps::View* view, QMouseEvent* event)
{
}

void Plugin_Image::wheelEvent(schnapps::View* view, QWheelEvent* event)
{
}

void Plugin_Image::view_linked(schnapps::View* view)
{
}

void Plugin_Image::view_unlinked(schnapps::View* view)
{
}

void Plugin_Image::import_image(const QString& image_path)
{
	QFileInfo fileinfo(image_path);
	if (fileinfo.exists() && fileinfo.isFile())
	{
		images_.push_back({image_path, Image3D::new_image_3d(image_path)});
		emit(image_added(image_path));
	}

}

void Plugin_Image::import_image_dialog()
{
	auto filenames = QFileDialog::getOpenFileNames(nullptr, "Import 3D images", schnapps_->get_app_path(),  "3DImages (*.inr *.vtk)");
	for (const auto& im : filenames)
		import_image(im);
}

void Plugin_Image::image_removed()
{
	const int current_image_id = dock_tab_->listWidget_images->currentRow();
	if (current_image_id >= 0)
	{
		QListWidgetItem* curr_item = dock_tab_->listWidget_images->currentItem();
		emit(image_removed(curr_item->text()));
		dock_tab_->listWidget_images->removeItemWidget(curr_item);
		delete curr_item;

		auto it = images_.begin();
		std::advance(it, current_image_id);
		images_.erase(it);
	}
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

Image3D::Image3D() :
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

Image3D::Image3D(Image3D&& im) :
	data_(std::move(im.data_)),
	image_dim_(im.image_dim_),
	origin_(im.origin_),
	voxel_dim_(im.voxel_dim_),
	translation_(im.translation_),
	rotation_(im.rotation_),
	nb_components_(im.nb_components_)
{}


Image3D& Image3D::operator=(Image3D&& im)
{
	if (this != &im)
	{
		data_ = std::move(im.data_);
		image_dim_ = im.image_dim_;
		origin_ = im.origin_;
		voxel_dim_ = im.voxel_dim_;
		translation_ = im.translation_;
		rotation_ = im.rotation_;
		nb_components_ = im.nb_components_;
	}

	return *this;
}

Image3D Image3D::new_image_3d(const QString& image_path)
{
	QFileInfo fileinfo(image_path);
	Image3D res_img;
	if (fileinfo.exists() && fileinfo.isFile())
	{
		cgogn::Scoped_C_Locale locale;
		std::ifstream in(image_path.toStdString(), std::ios_base::binary | std::ios_base::in);
		const auto& suffix = fileinfo.suffix();
		if(suffix == "inr")
			res_img.import_inr(in);
		else
		{
			if (suffix == "vtk")
				res_img.import_vtk(in);
			else
				return res_img;

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

	while (!line.empty() && line[0] != '#')
	{
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
		switch (data_size) {
			case 1: data_ = cgogn::make_unique<DataInput<uint8, float32>>(); break;
			case 2: data_ = cgogn::make_unique<DataInput<uint16, float32>>(); break;
			case 4: data_ = cgogn::make_unique<DataInput<uint32, float32>>(); break;
			case 8: data_ = cgogn::make_unique<DataInput<uint64, float32>>(); break;
			default: break;
		} else {
		if (type == "signed fixed")
			switch (data_size) {
				case 1: data_ = cgogn::make_unique<DataInput<int8, float32>>(); break;
				case 2: data_ = cgogn::make_unique<DataInput<int16, float32>>(); break;
				case 4: data_ = cgogn::make_unique<DataInput<int32, float32>>(); break;
				case 8: data_ = cgogn::make_unique<DataInput<int64, float32>>(); break;
				default: break;
			} else {
			if (type == "float")
				switch (data_size) {
					case 4: data_ = cgogn::make_unique<DataInput<float32>>(); break;
					case 8: data_ = cgogn::make_unique<DataInput<float64, float32>>(); break;
					default: break;
				}
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
	const auto to_upper = [=](const std::string& s) { return cgogn::to_upper(s); };

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


	while(!vtk_data.eof())
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
		{
			sstream >> origin_[0] >> origin_[1] >> origin_[2];
			origin_.fill(0);
		}


		if (word == "POINT_DATA")
		{
			std::size_t nb_data;
			sstream >> nb_data;

			std::ifstream::pos_type previous_pos;
			do {
				previous_pos = vtk_data.tellg();
				std::getline(vtk_data, line);
				sstream.str(line);
				sstream.clear();
				word.clear();
				sstream >> word;
				word = to_upper(word);
				if (word == "SCALARS" || word == "VECTOR")
				{
					const bool is_vector = !(word == "SCALARS");
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

					data_ = DataInputGen::template newDataIO<1, float32>(att_type, nb_components_);
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

} // namespace plugin_image
} // namespace schnapps
