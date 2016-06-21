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
#include <schnapps/core/cells_set.h>

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

	MapHandlerGen(const QString& name, SCHNApps* s, std::unique_ptr<MapBaseData> map);

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

	inline const MapBaseData* get_map() const { return map_.get(); }

	bool is_selected_map() const;

	virtual uint8 dimension() const = 0;

	virtual uint32 nb_vertices() const = 0;
	virtual uint32 nb_edges() const = 0;
	virtual uint32 nb_faces() const = 0;
	virtual uint32 nb_volumes() const = 0;

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
	virtual void set_bb_vertex_attribute(const QString& attribute_name) = 0;

	virtual void check_bb_vertex_attribute(cgogn::Orbit orbit, const QString& attribute_name) = 0;

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
	virtual void update_vbo(const QString& name) = 0;

public slots:

	cgogn::rendering::VBO* get_vbo(const QString& name) const;

	void delete_vbo(const QString& name);

	inline const std::map<QString, std::unique_ptr<cgogn::rendering::VBO>>& get_vbo_set() const { return vbos_; }

	/*********************************************************
	 * MANAGE CELLS SETS
	 *********************************************************/

	virtual CellsSetGen* add_cells_set(cgogn::Orbit orbit, const QString& name) = 0;

	void selected_cells_changed();

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

public:

	/*********************************************************
	 * MANAGE ATTRIBUTES & CONNECTIVITY
	 *********************************************************/

	void notify_attribute_change(cgogn::Orbit, const QString&);
	void notify_connectivity_change();

private:

	void link_view(View* view);
	void unlink_view(View* view);

signals:

	void bb_changed();
	void bb_vertex_attribute_changed(const QString&);

	void vbo_added(cgogn::rendering::VBO*);
	void vbo_removed(cgogn::rendering::VBO*);

	void selected_cells_changed(CellsSetGen*);

	void attribute_added(cgogn::Orbit, const QString&);
	void attribute_removed(cgogn::Orbit, const QString&);
	void attribute_changed(cgogn::Orbit, const QString&);

	void cells_set_added(cgogn::Orbit, const QString&);

	void connectivity_changed();

protected:

	// MapHandler name
	QString name_;

	// pointer to schnapps object
	SCHNApps* schnapps_;

	// MapBaseData generic pointer
	std::unique_ptr<MapBaseData> map_;

	// frame that allow user object manipulation (ctrl + mouse)
	qoglviewer::ManipulatedFrame frame_;

	// transformation matrix
	QMatrix4x4 transformation_matrix_;

	// list of views that are linked to this map
	std::list<View*> views_;

	// map bounding box
	cgogn::rendering::DisplayListDrawer bb_drawer_;
	std::map<View*, std::unique_ptr<cgogn::rendering::DisplayListDrawer::Renderer>> bb_drawer_renderer_;
	cgogn::geometry::AABB<VEC3> bb_;
	float bb_diagonal_size_;
	bool show_bb_;
	QColor bb_color_;

	// MapRender object of the map
	cgogn::rendering::MapRender render_;

	// VBO managed for the map attributes
	std::map<QString, std::unique_ptr<cgogn::rendering::VBO>> vbos_;

	// CellsSets of the map
	std::map<QString, std::unique_ptr<CellsSetGen>> cells_sets_[cgogn::NB_ORBITS];
};


template <typename MAP_TYPE>
class MapHandler : public MapHandlerGen
{
public:

	template <typename T, cgogn::Orbit ORBIT>
	using Attribute = typename MAP_TYPE::template Attribute<T, ORBIT>;

	template <typename T>
	using VertexAttribute = typename MAP_TYPE::template VertexAttribute<T>;
	template <typename T>
	using EdgeAttribute = typename MAP_TYPE::template EdgeAttribute<T>;
	template <typename T>
	using FaceAttribute = typename MAP_TYPE::template FaceAttribute<T>;
	template <typename T>
	using VolumeAttribute = typename MAP_TYPE::template VolumeAttribute<T>;

	using CDart = typename MAP_TYPE::CDart;
	using Vertex = typename MAP_TYPE::Vertex;
	using Edge = typename MAP_TYPE::Edge;
	using Face = typename MAP_TYPE::Face;
	using Volume = typename MAP_TYPE::Volume;

	MapHandler(const QString& name, SCHNApps* s) :
		MapHandlerGen(name, s, cgogn::make_unique<MAP_TYPE>())
	{}

	~MapHandler()
	{}

	inline MAP_TYPE* get_map() const { return static_cast<MAP_TYPE*>(this->map_.get()); }

	uint8 dimension() const override { return MAP_TYPE::DIMENSION; }

	uint32 nb_vertices() const override { return get_map()->template nb_cells<Vertex::ORBIT>(); }
	uint32 nb_edges() const override { return get_map()->template nb_cells<Edge::ORBIT>(); }
	uint32 nb_faces() const override { return get_map()->template nb_cells<Face::ORBIT>(); }
	uint32 nb_volumes() const override { return get_map()->template nb_cells<Volume::ORBIT>(); }

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/

	inline QString get_bb_vertex_attribute_name() const
	{
		if (bb_vertex_attribute_.is_valid())
			return QString::fromStdString(bb_vertex_attribute_.name());
		else
			return QString();
	}

	void set_bb_vertex_attribute(const QString& attribute_name) override
	{
		bb_vertex_attribute_ = get_map()->template get_attribute<VEC3, Vertex::ORBIT>(attribute_name.toStdString());
		compute_bb();
		this->update_bb_drawer();
		emit(bb_vertex_attribute_changed(attribute_name));
		emit(bb_changed());
	}

	void check_bb_vertex_attribute(cgogn::Orbit orbit, const QString& attribute_name) override
	{
		if (bb_vertex_attribute_.is_valid())
		{
			QString bb_vertex_attribute_name = QString::fromStdString(bb_vertex_attribute_.name());
			if (orbit == Vertex::ORBIT && attribute_name == bb_vertex_attribute_name)
			{
				compute_bb();
				this->update_bb_drawer();
				emit(bb_changed());
			}
		}
	}

private:

	inline void compute_bb() override
	{
		this->bb_.reset();

		if (bb_vertex_attribute_.is_valid())
			cgogn::geometry::compute_AABB(bb_vertex_attribute_, this->bb_);

		if (this->bb_.is_initialized())
			this->bb_diagonal_size_ = this->bb_.diag_size();
		else
			this->bb_diagonal_size_ = .0f;
	}

	/*********************************************************
	 * MANAGE DRAWING
	 *********************************************************/

	inline void draw(cgogn::rendering::DrawingType primitive) override
	{
		if (!this->render_.is_primitive_uptodate(primitive))
			this->render_.init_primitives(*get_map(), primitive);
		this->render_.draw(primitive);
	}

	/*********************************************************
	 * MANAGE ATTRIBUTES
	 *********************************************************/

public:

	template <typename T, cgogn::Orbit ORBIT>
	inline Attribute<T, ORBIT> add_attribute(const QString& attribute_name)
	{
		Attribute<T, ORBIT> a = get_map()->template add_attribute<T, ORBIT>(attribute_name.toStdString());
		if (a.is_valid())
			emit(attribute_added(ORBIT, attribute_name));
		return a;
	}

	template <typename T, cgogn::Orbit ORBIT>
	inline bool remove_attribute(Attribute<T, ORBIT>& ah)
	{
		if (ah.is_valid())
		{
			QString attribute_name = QString::fromStdString(ah->get_name());
			return get_map()->remove_attribute(ah);
			emit(attribute_removed(ORBIT, attribute_name));
		}
	}

	template <typename T, cgogn::Orbit ORBIT>
	inline Attribute<T, ORBIT> get_attribute(const QString& attribute_name)
	{
		return get_map()->template get_attribute<T, ORBIT>(attribute_name.toStdString());
	}

protected:

	/*********************************************************
	 * MANAGE VBOs
	 *********************************************************/

	cgogn::rendering::VBO* create_vbo(const QString& name) override
	{
		cgogn::rendering::VBO* vbo = this->get_vbo(name);
		if (!vbo)
		{
			MAP_TYPE* map = get_map();

			const MAP_TYPE* cmap = map;
			const MapBaseData::ChunkArrayContainer<cgogn::uint32>& vcont = cmap->template const_attribute_container<Vertex::ORBIT>();
			MapBaseData::ChunkArrayGen* cag = vcont.get_chunk_array(name.toStdString());

			MapBaseData::ChunkArray<VEC4F>* ca4f = dynamic_cast<MapBaseData::ChunkArray<VEC4F>*>(cag);
			if (ca4f)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC4F> va(map, ca4f);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC4D>* ca4d = dynamic_cast<MapBaseData::ChunkArray<VEC4D>*>(cag);
			if (ca4d)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC4D> va(map, ca4d);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC3F>* ca3f = dynamic_cast<MapBaseData::ChunkArray<VEC3F>*>(cag);
			if (ca3f)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC3F> va(map, ca3f);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC3D>* ca3d = dynamic_cast<MapBaseData::ChunkArray<VEC3D>*>(cag);
			if (ca3d)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC3D> va(map, ca3d);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC2F>* ca2f = dynamic_cast<MapBaseData::ChunkArray<VEC2F>*>(cag);
			if (ca2f)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC2F> va(map, ca2f);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<VEC2D>* ca2d = dynamic_cast<MapBaseData::ChunkArray<VEC2D>*>(cag);
			if (ca2d)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC2D> va(map, ca2d);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<float32>* ca1f = dynamic_cast<MapBaseData::ChunkArray<float32>*>(cag);
			if (ca1f)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<float32> va(map, ca1f);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			MapBaseData::ChunkArray<float64>* ca1d = dynamic_cast<MapBaseData::ChunkArray<float64>*>(cag);
			if (ca1d)
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
				vbo = this->vbos_.at(name).get();
				VertexAttribute<float64> va(map, ca1d);
				cgogn::rendering::update_vbo(va, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}
		}

		return vbo;
	}

	void update_vbo(const QString& name) override
	{
		cgogn::rendering::VBO* vbo = get_vbo(name);
		if (vbo)
		{
			MAP_TYPE* map = get_map();

			const MAP_TYPE* cmap = map;
			const MapBaseData::ChunkArrayContainer<cgogn::uint32>& vcont = cmap->template const_attribute_container<Vertex::ORBIT>();
			MapBaseData::ChunkArrayGen* cag = vcont.get_chunk_array(name.toStdString());

			MapBaseData::ChunkArray<VEC4F>* ca4f = dynamic_cast<MapBaseData::ChunkArray<VEC4F>*>(cag);
			if (ca4f)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC4F> va(map, ca4f);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<VEC4D>* ca4d = dynamic_cast<MapBaseData::ChunkArray<VEC4D>*>(cag);
			if (ca4f)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC4D> va(map, ca4d);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<VEC3F>* ca3f = dynamic_cast<MapBaseData::ChunkArray<VEC3F>*>(cag);
			if (ca3f)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC3F> va(map, ca3f);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<VEC3D>* ca3d = dynamic_cast<MapBaseData::ChunkArray<VEC3D>*>(cag);
			if (ca3d)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC3D> va(map, ca3d);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<VEC2F>* ca2f = dynamic_cast<MapBaseData::ChunkArray<VEC2F>*>(cag);
			if (ca2f)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC2F> va(map, ca2f);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<VEC2D>* ca2d = dynamic_cast<MapBaseData::ChunkArray<VEC2D>*>(cag);
			if (ca2d)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<VEC2D> va(map, ca2d);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<float32>* ca1f = dynamic_cast<MapBaseData::ChunkArray<float32>*>(cag);
			if (ca1f)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<float32> va(map, ca1f);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}

			MapBaseData::ChunkArray<float64>* ca1d = dynamic_cast<MapBaseData::ChunkArray<float64>*>(cag);
			if (ca1d)
			{
				vbo = this->vbos_.at(name).get();
				VertexAttribute<float64> va(map, ca1d);
				cgogn::rendering::update_vbo(va, vbo);
				return;
			}
		}
	}

	/*********************************************************
	 * MANAGE CELLS SETS
	 *********************************************************/

	CellsSetGen* add_cells_set(cgogn::Orbit orbit, const QString& name) override
	{
		if (this->cells_sets_[orbit].count(name) > 0ul)
			return nullptr;

		switch (orbit)
		{
			case Vertex::ORBIT:
				this->cells_sets_[orbit].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Vertex>>(get_map(), name)));
				break;
			case Edge::ORBIT:
				this->cells_sets_[orbit].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Edge>>(get_map(), name)));
				break;
			case Face::ORBIT:
				this->cells_sets_[orbit].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Face>>(get_map(), name)));
				break;
			case Volume::ORBIT:
				this->cells_sets_[orbit].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Volume>>(get_map(), name)));
				break;
		}

		CellsSetGen* cells_set = this->cells_sets_[orbit].at(name).get();
		emit(cells_set_added(orbit, name));
		connect(cells_set, SIGNAL(selected_cells_changed()), this, SLOT(selected_cells_changed()));
		return cells_set;
	}

private:

	VertexAttribute<VEC3> bb_vertex_attribute_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_MAPHANDLER_H_
