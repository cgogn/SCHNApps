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

#ifndef SCHNAPPS_CORE_MAPHANDLER_H_
#define SCHNAPPS_CORE_MAPHANDLER_H_

#include <schnapps/core/dll.h>

#include <cgogn/core/cmap/map_base.h>
#include <cgogn/core/cmap/cmap2.h>
#include <cgogn/core/cmap/cmap3.h>

#include <cgogn/rendering/shaders/vbo.h>
#include <cgogn/rendering/map_render.h>

#include <cgogn/geometry/algos/bounding_box.h>

#include <Eigen/Dense>
#include <QOGLViewer/manipulatedFrame.h>

#include <QObject>
#include <QString>

namespace cgogn { namespace rendering { class Drawer; } }

namespace schnapps
{

class SCHNApps;
class View;

using MapBaseData = cgogn::MapBaseData<cgogn::DefaultMapTraits>;
using CMap2 = cgogn::CMap2<cgogn::DefaultMapTraits>;
using CMap3 = cgogn::CMap3<cgogn::DefaultMapTraits>;

using VEC3 = Eigen::Vector3d;

class SCHNAPPS_CORE_API MapHandlerGen : public QObject
{
	Q_OBJECT

	friend class View;

public:

	MapHandlerGen(const QString& name, SCHNApps* s, MapBaseData* map);

	~MapHandlerGen();

public slots:

	/**
	 * @brief get the name of MapHandlerGen object
	 * @return name
	 */
	inline const QString& get_name() { return name_; }

	/**
	 * @brief get the schnapps objet ptr
	 * @return the ptr
	 */
	inline SCHNApps* get_schnapps() const { return schnapps_; }

	inline const MapBaseData* get_map() const { return map_; }

	bool is_selected_map() const;

	/*********************************************************
	 * MANAGE FRAME
	 *********************************************************/

	// get the frame associated to the map
	inline qoglviewer::ManipulatedFrame& get_frame() { return frame_; }

	// get the matrix of the frame associated to the map
	QMatrix4x4 get_frame_matrix() const;

private slots:

	void frame_changed();

	/*********************************************************
	 * MANAGE TRANSFORMATION MATRIX
	 *********************************************************/

public slots:

	// get the frame associated to the map
	inline const QMatrix4x4& get_transformation_matrix() const { return transformation_matrix_; }

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/

	/**
	* @brief set if bounding box has to be drawn
	* @param b yes or no
	*/
	void set_show_bb(bool b);

	/**
	* @brief is the bounding box of the map drawn
	* @return is bounding box of the map drawn
	*/
	inline bool get_show_bb() const { return show_bb_; }

	/**
	* @brief set color for drawing BB the bounding box
	* @param color color name (red,green,...) or color format #rrggbb
	*/
	void set_bb_color(const QString& color);

	/**
	* @brief choose the vertex attribute used to compute the bounding box
	* @param name name of attribute
	*/
	virtual void set_bb_vertex_attribute(const QString& name) = 0;

	/**
	* @brief get the length of diagonal of bounding box of the map
	* @return length of the diagonal of the bounding box
	*/
	inline float get_bb_diagonal_size() const { return bb_diagonal_size_; }

	/**
	 * @brief get the bounding box of the map after transformation by frame & transformation matrix
	 * @param bb_min minimum point
	 * @param bb_max maximum point
	 * @return
	 */
	bool get_transformed_bb(qoglviewer::Vec& bb_min, qoglviewer::Vec& bb_max);

	/**
	 * @brief draw the bounding box
	 * @param pm projection matrix
	 * @param mm modelview matrix
	 */
	void draw_bb(const QMatrix4x4& pm, const QMatrix4x4& mm);

protected:

	void update_bb_drawer();
	virtual void compute_bb() = 0;

	/*********************************************************
	 * MANAGE DRAWING
	 *********************************************************/

public:

	virtual void draw(cgogn::rendering::DrawingType primitive) = 0;

	/*********************************************************
	 * MANAGE ATTRIBUTES
	 *********************************************************/

protected:

	virtual const MapBaseData::ChunkArrayContainer<uint32>& get_vertex_attribute_container() = 0;

	/*********************************************************
	 * MANAGE VBOs
	 *********************************************************/

public slots:

	/**
	* @brief create a VBO from vertex attribute (with same name)
	* @param name name of attribute
	* @return pointer on created VBO
	*/
	cgogn::rendering::VBO* create_VBO(const QString& name);

	cgogn::rendering::VBO* get_VBO(const QString& name) const;

	/*********************************************************
	 * MANAGE LINKED VIEWS
	 *********************************************************/

	// get the list of views linked to the map
	inline const QList<View*>& get_linked_views() const { return views_; }

	// test if a view is linked to this map
	inline bool is_linked_to_view(View* view) const { return views_.contains(view); }

private:

	void link_view(View* view);
	void unlink_view(View* view);

signals:

	void bb_changed();
	void bb_vertex_attribute_changed(const QString&);

	void attribute_added(cgogn::Orbit, const QString&);

protected:

	// MapHandler name
	QString name_;

	// pointer to schnapps object
	SCHNApps* schnapps_;

	// MapBaseData generic pointer
	MapBaseData* map_;

	// frame that allow user object manipulation (ctrl + mouse)
	qoglviewer::ManipulatedFrame frame_;

	// transformation matrix
	QMatrix4x4 transformation_matrix_;

	// list of views that are linked to this map
	QList<View*> views_;

	// map bounding box
	cgogn::geometry::BoundingBox<VEC3> bb_;
	cgogn::rendering::Drawer* bb_drawer_;
	View* bb_drawer_view_context_;
	bool show_bb_;
	QColor bb_color_;
	float bb_diagonal_size_;

	// MapRender object of the map
	cgogn::rendering::MapRender render_;

	// VBO managed for the map attributes
	QMap<QString, cgogn::rendering::VBO*> vbos_;
};

template <typename MAP_TYPE>
class MapHandler : public MapHandlerGen
{
public:

	template <typename T>
	using VertexAttribute = typename MAP_TYPE::template VertexAttribute<T>;
	using Vertex = typename MAP_TYPE::Vertex;

	MapHandler(const QString& name, SCHNApps* s, MAP_TYPE* map) :
		MapHandlerGen(name, s, map)
	{}

	~MapHandler()
	{}

	inline MAP_TYPE* get_map() { return static_cast<MAP_TYPE*>(map_); }

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/

	QString get_bb_vertex_attribute_name() const
	{
		if (bb_vertex_attribute_.is_valid())
			return QString::fromStdString(bb_vertex_attribute_.get_name());
		else
			return QString();
	}

	void set_bb_vertex_attribute(const QString& name) override
	{
		bb_vertex_attribute_ = get_map()->template get_attribute<VEC3, Vertex::ORBIT>(name.toStdString());
		compute_bb();
		this->update_bb_drawer();
		emit(bb_vertex_attribute_changed(name));
		emit(bb_changed());
	}

private:

	inline void compute_bb() override
	{
		this->bb_.reset();

		if (bb_vertex_attribute_.is_valid())
			cgogn::geometry::compute_bounding_box(bb_vertex_attribute_, this->bb_);

		if (this->bb_.is_initialized())
			this->bb_diagonal_size_ = this->bb_.diag_size();
		else
			this->bb_diagonal_size_ = .0f;
	}

	/*********************************************************
	 * MANAGE DRAWING
	 *********************************************************/

	void draw(cgogn::rendering::DrawingType primitive) override
	{
		if (!render_.is_primitive_uptodate(primitive))
			render_.init_primitives<VEC3>(*get_map(), primitive, bb_vertex_attribute_);
		render_.draw(primitive);
	}

	/*********************************************************
	 * MANAGE ATTRIBUTES
	 *********************************************************/

protected:

	const MapBaseData::ChunkArrayContainer<uint32>& get_vertex_attribute_container() override
	{
		return map_->get_attribute_container<Vertex::ORBIT>();
	}

private:

	VertexAttribute<VEC3> bb_vertex_attribute_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_MAPHANDLER_H_
