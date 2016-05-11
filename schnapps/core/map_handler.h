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
#include <schnapps/core/types.h>

#include <cgogn/core/cmap/map_base.h>
#include <cgogn/core/cmap/cmap2.h>
#include <cgogn/core/cmap/cmap3.h>

#include <cgogn/rendering/shaders/vbo.h>
#include <cgogn/rendering/map_render.h>

#include <cgogn/geometry/algos/bounding_box.h>

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

	virtual uint32 nb_vertices() = 0;
	virtual uint32 nb_edges() = 0;
	virtual uint32 nb_faces() = 0;

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
	void draw_bb(View* view, const QMatrix4x4& pm, const QMatrix4x4& mm);

protected:

	void update_bb_drawer();
	virtual void compute_bb() = 0;

	/*********************************************************
	 * MANAGE DRAWING
	 *********************************************************/

public:

	virtual void draw(cgogn::rendering::DrawingType primitive) = 0;

	/*********************************************************
	 * MANAGE VBOs
	 *********************************************************/

	/**
	* @brief create a VBO from vertex attribute (with same name)
	* @param name name of attribute
	* @return pointer on created VBO
	*/
	virtual cgogn::rendering::VBO* create_vbo(const QString& name) = 0;

public slots:

	cgogn::rendering::VBO* get_vbo(const QString& name) const;

	void delete_vbo(const QString& name);

	inline const std::map<QString, std::unique_ptr<cgogn::rendering::VBO>>& get_vbo_set() const { return vbos_; }

	/*********************************************************
	 * MANAGE LINKED VIEWS
	 *********************************************************/

	// get the list of views linked to the map
	inline const std::list<View*>& get_linked_views() const { return views_; }

	// test if a view is linked to this map
	inline bool is_linked_to_view(View* view) const
	{
		return std::find(views_.begin(), views_.end(), view) != views_.end();
	}

private:

	void link_view(View* view);
	void unlink_view(View* view);

signals:

	void bb_changed();
	void bb_vertex_attribute_changed(const QString&);

	void vbo_added(cgogn::rendering::VBO*);
	void vbo_removed(cgogn::rendering::VBO*);

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
	std::list<View*> views_;

	// map bounding box
	cgogn::rendering::DisplayListDrawer bb_drawer_;
	std::map<View*, std::unique_ptr<cgogn::rendering::DisplayListDrawer::Renderer>> bb_drawer_renderer_;
	cgogn::geometry::BoundingBox<VEC3> bb_;
	float bb_diagonal_size_;
	bool show_bb_;
	QColor bb_color_;

	// MapRender object of the map
	cgogn::rendering::MapRender render_;

	// VBO managed for the map attributes
	std::map<QString, std::unique_ptr<cgogn::rendering::VBO>> vbos_;
};


template <typename MAP_TYPE>
class MapHandler : public MapHandlerGen
{
public:

	template <typename T>
	using VertexAttribute = typename MAP_TYPE::template VertexAttribute<T>;
	template <typename T>
	using EdgeAttribute = typename MAP_TYPE::template EdgeAttribute<T>;
	template <typename T>
	using FaceAttribute = typename MAP_TYPE::template FaceAttribute<T>;
	using Vertex = typename MAP_TYPE::Vertex;
	using Edge = typename MAP_TYPE::Edge;
	using Face = typename MAP_TYPE::Face;

	MapHandler(const QString& name, SCHNApps* s, MAP_TYPE* map) :
		MapHandlerGen(name, s, map)
	{}

	~MapHandler()
	{}

	inline MAP_TYPE* get_map() { return static_cast<MAP_TYPE*>(map_); }

	uint32 nb_vertices() override { return get_map()->template nb_cells<Vertex::ORBIT>(); }
	uint32 nb_edges() override { return get_map()->template nb_cells<Edge::ORBIT>(); }
	uint32 nb_faces() override { return get_map()->template nb_cells<Face::ORBIT>(); }

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
			render_.init_primitives<VEC3>(*get_map(), primitive);
		render_.draw(primitive);
	}

	/*********************************************************
	 * MANAGE ATTRIBUTES
	 *********************************************************/

//protected:

//	const MapBaseData::ChunkArrayContainer<cgogn::uint32>& get_vertex_attribute_container()
//	{
//		const MapBaseData* cm = this->map_;
//		return cm->get_attribute_container<Vertex::ORBIT>();
//	}

	/*********************************************************
	 * MANAGE VBOs
	 *********************************************************/

	cgogn::rendering::VBO* create_vbo(const QString& name) override
	{
		cgogn::rendering::VBO* vbo = get_vbo(name);

		if (!vbo)
		{
			MAP_TYPE* map = get_map();

			const MAP_TYPE* cmap = map;
			const MapBaseData::ChunkArrayContainer<cgogn::uint32>& vcont = cmap->template get_attribute_container<Vertex::ORBIT>();
			MapBaseData::ChunkArrayGen* cag = vcont.get_attribute(name.toStdString());

			MapBaseData::ChunkArray<VEC4>* ca4 = dynamic_cast<MapBaseData::ChunkArray<VEC4>*>(cag);
			if (ca4)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
				vbo = vbos_.find(name)->second.get();
				VertexAttribute<VEC4> va(map, ca4);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC3>* ca3 = dynamic_cast<MapBaseData::ChunkArray<VEC3>*>(cag);
			if (ca3)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = vbos_.find(name)->second.get();
				VertexAttribute<VEC3> va(map, ca3);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC2>* ca2 = dynamic_cast<MapBaseData::ChunkArray<VEC2>*>(cag);
			if (ca2)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
				vbo = vbos_.find(name)->second.get();
				VertexAttribute<VEC2> va(map, ca2);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<SCALAR>* ca1 = dynamic_cast<MapBaseData::ChunkArray<SCALAR>*>(cag);
			if (ca1)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
				vbo = vbos_.find(name)->second.get();
				VertexAttribute<SCALAR> va(map, ca1);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}
		}

		return vbo;
	}

private:

	VertexAttribute<VEC3> bb_vertex_attribute_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_MAPHANDLER_H_
