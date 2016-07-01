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

#define SCHNAPPS_PLUGIN_IMAGE_DLL_EXPORT

#include "image.h"
#include <QFileDialog>
#include <QFileInfo>
#include <image_dock_tab.h>
#include <schnapps/core/schnapps.h>
#include <cgogn/io/c_locale.h>
#include <cgogn/core/utils/string.h>
#include <cgal/cgal_image.h>

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
	auto filenames = QFileDialog::getOpenFileNames(nullptr, "Import 3D images", schnapps_->get_app_path(),  "3DImages (*.inr)");
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
	nb_components_(0u),
	little_endian_(true),
	data_size_(0ul),
	data_type_(DataType::UNKNOWN)

{
	image_dim_.fill(UINT64_MAX);
	translation_.fill(0);
	rotation_.fill(0);
	origin_.fill(0);
	voxel_dim_.fill(std::numeric_limits<float64>::quiet_NaN());
}

Image3D::Image3D(Image3D&& im) :
	data_(std::move(im.data_)),
	image_dim_(im.image_dim_),
	origin_(im.origin_),
	voxel_dim_(im.voxel_dim_),
	translation_(im.translation_),
	rotation_(im.rotation_),
	nb_components_(im.nb_components_),
	little_endian_(im.little_endian_),
	data_size_(im.data_size_),
	data_type_(im.data_type_)
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
		little_endian_ = im.little_endian_;
		data_size_ = im.data_size_;
		data_type_ = im.data_type_;
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
		if(fileinfo.suffix() == "inr")
			res_img.import_inr(in);
		else
			return res_img;
		export_to_cgal_image(res_img);
	}
	return res_img;
}

void Image3D::import_inr(std::istream& inr_data)
{
	std::array<char, 256> buffer;
	std::string line;
	std::string type;
	std::string cpu;

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
			data_size_ = std::stoul(line.substr(8,2))/8ul;

		if (line.compare(0,4,"CPU=") == 0)
			cpu = line.substr(4);

		std::getline(sstream, line);
	}

	cpu = cgogn::to_lower(cpu);
	type = cgogn::to_lower(type);

	little_endian_ = (cpu == "decm" || cpu == "pc" || cpu == "alpha");

	if (type == "unsigned fixed")
		switch (data_size_) {
			case 1: data_ = cgogn::make_unique<DataInput<uint8>>(); data_type_ = DataType::UINT8; break;
			case 2: data_ = cgogn::make_unique<DataInput<uint16>>(); data_type_ = DataType::UINT16; break;
			case 4: data_ = cgogn::make_unique<DataInput<uint32>>(); data_type_ = DataType::UINT32; break;
			case 8: data_ = cgogn::make_unique<DataInput<uint64>>(); data_type_ = DataType::UINT64; break;
			default: break;
		} else {
		if (type == "signed fixed")
			switch (data_size_) {
				case 1: data_ = cgogn::make_unique<DataInput<int8>>(); data_type_ = DataType::INT8; break;
				case 2: data_ = cgogn::make_unique<DataInput<int16>>(); data_type_ = DataType::INT16; break;
				case 4: data_ = cgogn::make_unique<DataInput<int32>>(); data_type_ = DataType::INT32; break;
				case 8: data_ = cgogn::make_unique<DataInput<int64>>(); data_type_ = DataType::INT64; break;
				default: break;
			} else {
			if (type == "float")
				switch (data_size_) {
					case 4: data_ = cgogn::make_unique<DataInput<float32>>(); data_type_ = DataType::FLOAT; break;
					case 8: data_ = cgogn::make_unique<DataInput<float64>>(); data_type_ = DataType::DOUBLE; break;
					default: break;
				}
		}
	}
	if (data_)
		data_->read_n(inr_data, image_dim_[0] * image_dim_[1] * image_dim_[2] * nb_components_ * data_size_, true, !little_endian_);
}

} // namespace plugin_image
} // namespace schnapps
