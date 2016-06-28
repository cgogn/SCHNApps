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
#include <schnapps/core/schnapps.h>
#include <QFileDialog>

namespace schnapps
{

namespace plugin_image
{

Image3DBase::~Image3DBase()
{}

bool Plugin_Image::enable()
{
	import_image_action_ = schnapps_->add_menu_action("Import;3D Image", "import image");
	connect(import_image_action_, SIGNAL(triggered()), this, SLOT(import_image_dialog()));
}

void Plugin_Image::disable()
{
	schnapps_->remove_menu_action(import_image_action_);
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
	images_.push_back(cgogn::make_unique<Image3D<>>(image_path.toStdString().c_str()));
//	auto& im = *images_.back();
}

void Plugin_Image::import_image_dialog()
{
	auto filenames = QFileDialog::getOpenFileNames(nullptr, "Import 3D images", schnapps_->get_app_path());
	for (const auto& im : filenames)
		import_image(im);
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace plugin_image
} // namespace schnapps
