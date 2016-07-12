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
#include <cgogn/core/utils/endian.h>
#include <cgogn/io/data_io.h>
#include <schnapps/core/plugin_interaction.h>
#include <QAction>
#ifdef PLUGIN_IMAGE_WITH_CGAL
#include <CGAL/config.h>
#include <CGAL/version.h>
#endif

namespace schnapps
{

namespace plugin_image
{

using namespace cgogn::numerics;
class Image_DockTab;


class SCHNAPPS_PLUGIN_IMAGE_API Image3D
{
public:
	using DataInputGen = cgogn::io::DataInputGen<cgogn::DefaultMapTraits::CHUNK_SIZE>;
	template<typename BUFFER_T, typename T = BUFFER_T>
	using DataInput = cgogn::io::DataInput<cgogn::DefaultMapTraits::CHUNK_SIZE, 1, BUFFER_T, T>;
	using DataType = cgogn::io::DataType;

#ifdef PLUGIN_IMAGE_WITH_CGAL
#if CGAL_VERSION_NR >= CGAL_VERSION_NUMBER(4,8,0)
	using value_type = float32;
#else
	using value_type = unsigned char;
#endif // CGAL_VERSION_NR >= CGAL_VERSION_NUMBER(4,8,0)
#else
	using value_type = float32;
#endif // PLUGIN_IMAGE_WITH_CGAL

	Image3D();
	Image3D(const Image3D&) = delete;
	Image3D(Image3D&& im);
	Image3D& operator=(Image3D&& im);
	Image3D& operator=(const Image3D&) = delete;

	void const * data() const { return data_->data(); }
	inline std::array<uint64, 3> const& get_image_dimensions() const { return image_dim_; }
	inline std::array<float64, 3> const& get_origin() const { return origin_; }
	inline std::array<float64, 3> const& get_voxel_dimensions() const { return voxel_dim_; }
	inline std::array<float64, 3> const& get_translation() const { return translation_; }
	inline std::array<float64, 3> const& get_rotation() const { return rotation_; }
	inline uint8 get_nb_components() const { return nb_components_; }
	inline bool is_little_endian() const { return cgogn::internal::cgogn_is_little_endian ; }
	inline std::size_t get_data_size() const { return data_->data_size();}
	inline DataType get_data_type() const { return data_->data_type(); }


	static Image3D new_image_3d(const QString& image_path);
private:
	void import_inr(std::istream& inr_data);
	void import_vtk(std::istream& vtk_data);

private:
	std::unique_ptr<DataInputGen> data_;
	std::array<uint64, 3> image_dim_;
	std::array<float64, 3> origin_;
	std::array<float64, 3> voxel_dim_;
	std::array<float64, 3> translation_;
	std::array<float64, 3> rotation_;
	uint32 nb_components_;
};

class SCHNAPPS_PLUGIN_IMAGE_API Plugin_Image : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:
	Plugin_Image();

	std::list<std::pair<QString, Image3D>> const& get_images() const;
	const Image3D* get_image(const QString& im_path);
private:
	bool enable() override;
	void disable() override;

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
	void image_removed();

signals:
	void image_added(QString im_path);
	void image_removed(QString im_path);

private:
	std::list<std::pair<QString, Image3D>> images_;
	QAction* import_image_action_;
	Image_DockTab*	dock_tab_;
};

} // namespace plugin_image
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_IMAGE_IMAGE_H_
