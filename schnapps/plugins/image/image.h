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

#include <schnapps/plugins/image/plugin_image_export.h>
#include <schnapps/core/object.h>
#include <schnapps/core/plugin_provider.h>

#include <cgogn/core/utils/unique_ptr.h>
#include <cgogn/core/utils/endian.h>
#include <cgogn/core/utils/logger.h>
#include <cgogn/io/data_io.h>
#include <cgogn/io/point_set_import.h>

#include <QAction>

#include <cstdint>
#include <iostream>

#ifdef PLUGIN_IMAGE_WITH_CGAL
#include <CGAL/config.h>
#include <CGAL/version.h>
#endif

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
} // namespace plugin_cmap_provider

namespace plugin_image
{

using namespace cgogn::numerics;
class Image_DockTab;

class PLUGIN_IMAGE_EXPORT Image3D final : public Object
{
public:

	using DataInputGen = cgogn::io::DataInputGen;
	template<typename BUFFER_T, typename T = BUFFER_T>
	using DataInput = cgogn::io::DataInput<1, BUFFER_T, T>;
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

	Image3D(const QString& name, PluginProvider* p);
	Image3D(const Image3D&) = delete;
	Image3D& operator=(const Image3D&) = delete;

	void const * data() const { return data_->data(); }

	value_type const& operator()(std::size_t i, std::size_t j, std::size_t k) const;

	inline std::array<float64,3> position(std::size_t i, std::size_t j, std::size_t k) const
	{
		// ignoring rotation
		return {
			origin_[0] + float64(i)* voxel_dim_[0],
			origin_[1] + float64(j)* voxel_dim_[1],
			origin_[2] + float64(k)* voxel_dim_[2]
		};
	}
	inline std::array<uint64, 3> const& get_image_dimensions() const { return image_dim_; }
	inline std::array<float64, 3> const& get_origin() const { return origin_; }
	inline std::array<float64, 3> const& get_voxel_dimensions() const { return voxel_dim_; }
	inline std::array<float64, 3> const& get_translation() const { return translation_; }
	inline std::array<float64, 3> const& get_rotation() const { return rotation_; }
	inline uint32 get_nb_components() const { return nb_components_; }
	inline bool is_little_endian() const { return cgogn::internal::cgogn_is_little_endian ; }
	inline std::size_t get_data_size() const { return data_->data_size();}
	inline DataType get_data_type() const { return data_->data_type(); }
	inline bool is_empty() const { return data_.get() == nullptr; }

	static Image3D* new_image_3d(const QString& image_path, const QString& objectname, PluginProvider* p);

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

	// Object interface
private:
	void view_linked(View* view);
	void view_unlinked(View* view);
};



class PLUGIN_IMAGE_EXPORT ImagePointSetImport final: public cgogn::io::PointSetImport<CMap0>
{
public:

	using Inherit = cgogn::io::PointSetImport<CMap0>;
	using Scalar = typename cgogn::geometry::vector_traits<VEC3>::Scalar;
	template <typename T>
	using ChunkArray = typename Inherit::template ChunkArray<T>;

	ImagePointSetImport(CMap0& map);
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(ImagePointSetImport);
	virtual ~ImagePointSetImport() override;

	bool import_image(const Image3D& im, double threshold);
};


class PLUGIN_IMAGE_EXPORT Plugin_Image : public PluginProvider
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_Image();
	~Plugin_Image() override;
	static QString plugin_name();

private:

	bool enable() override;
	void disable() override;

public:

	void add_image(const QString& image_path);
	const Image3D* image(const QString& im_name) const;

private slots:
	void import_image_dialog();
	void threshold_changed(double t);

public slots:
	void image_removed(const QString& name);
	void export_image_to_point_set(const QString& name);

signals:
	void context_menu_created(QMenu*, const QString& image_name);

private:
	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;
	QAction* import_image_action_;
	Image_DockTab*	dock_tab_;
	double export_point_set_threshold_;
};

/**
 * @brief uncompress_gz_file
 * @param filename, path of the fil to uncompress
 * @return the path of the extracted file (usually in the temporary folder), an empty QString if unsuccessfull
 */
PLUGIN_IMAGE_EXPORT QString uncompress_gz_file(const QString& filename);

} // namespace plugin_image

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_IMAGE_IMAGE_H_
