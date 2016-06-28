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

#ifndef SCHNAPPS_PLUGIN_IMAGE_IMAGE_H_
#define SCHNAPPS_PLUGIN_IMAGE_IMAGE_H_

#include <cstdint>
#include <iostream>
#include <cgogn/core/utils/logger.h>
#include "dll.h"
#include <cgogn/core/utils/unique_ptr.h>
#include <schnapps/core/plugin_interaction.h>
#include <QAction>
#include "cimg/CImg.h"

//namespace cimg_library
//{
//	template<class>
//	class CImg;
//} // namespace cimg_library

namespace schnapps
{

namespace plugin_image
{

class Image3DBase
{
public:
	virtual ~Image3DBase();
};

template<typename T = std::float_t>
class Image3D : public Image3DBase
{
public:
	using Inherit = Image3DBase;
	using value_type = T;
	using Self = Image3D<value_type>;
	using image_type = cimg_library::CImg<value_type>;

	inline Image3D(const QString& im_path) : img_(im_path.toStdString().c_str())
	{
		cgogn_log_debug("Image3D") << "dim " <<img_.height() << " " << img_.width() << " " << img_.depth();
	}
	~Image3D() override {}

private:
	image_type img_;
};


class Plugin_Image : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)


	// Plugin interface
private:
	bool enable() override;
	void disable() override;

	// PluginInteraction interface
private:
	void draw(View* view, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	void keyPress(View* view, QKeyEvent* event) override;
	void keyRelease(View* view, QKeyEvent* event) override;
	void mousePress(View* view, QMouseEvent* event) override;
	void mouseRelease(View* view, QMouseEvent* event) override;
	void mouseMove(View* view, QMouseEvent* event) override;
	void wheelEvent(View* view, QWheelEvent* event) override;
	void view_linked(View* view) override;
	void view_unlinked(View* view) override;

	void import_image(const QString& image_path);

private slots:
	void import_image_dialog();

private:
	std::list<std::unique_ptr<Image3D<>>> images_;
	QAction* import_image_action_;
};

} // namespace plugin_image
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_IMAGE_IMAGE_H_
