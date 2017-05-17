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
#include <cgogn/core/cmap/cmap3.h>

#include <cgogn/geometry/algos/bounding_box.h>

#include <cgogn/rendering/shaders/vbo.h>
#include <cgogn/rendering/map_render.h>

#include <QOGLViewer/manipulatedFrame.h>

#include <QObject>
#include <QString>

namespace cgogn { namespace rendering { class Drawer; } }

namespace schnapps
{

class SCHNApps;
class View;

class SCHNAPPS_CORE_API MapHandlerGen : public QObject
{
	Q_OBJECT

	friend class View;

public:

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	template<typename T>
	using ChunkArrayContainer = MapBaseData::ChunkArrayContainer<T>;
	using ChunkArrayGen = cgogn::MapBaseData::ChunkArrayGen;
	template<typename T>
	using ChunkArray = cgogn::MapBaseData::ChunkArray<T>;

	using AttributeGen = cgogn::AttributeGen;
	template <typename T>
	using Attribute_T = cgogn::Attribute_T<T>;
	template <typename T, cgogn::Orbit ORBIT>
	using Attribute = cgogn::Attribute<T,ORBIT>;

	MapHandlerGen(const QString& name, SCHNApps* s, std::unique_ptr<MapBaseData> map);

	~MapHandlerGen();

public:

	/**********************************************************
	 * BASIC FUNCTIONS                                        *
	 *********************************************************/

	/**
	 * @brief get the name of MapHandlerGen object
	 * @return name
	 */
	inline const QString& get_name() const { return name_; }

	/**
	 * @brief get the schnapps objet ptr
	 * @return the ptr
	 */
	inline const SCHNApps* get_schnapps() const { return schnapps_; }

	inline const MapBaseData* get_map() const { return map_.get(); }

	virtual bool merge(const MapHandlerGen* other_map) = 0;

	bool is_selected_map() const;

	virtual uint8 dimension() const = 0;

	virtual CellType cell_type(cgogn::Orbit orbit) const = 0;

	virtual cgogn::Orbit orbit(CellType ct) const = 0;

	virtual uint32 nb_cells(CellType ct) const = 0;

	virtual bool same_cell(cgogn::Dart d, cgogn::Dart e, CellType ct) const = 0;

	virtual std::pair<cgogn::Dart, cgogn::Dart> vertices(cgogn::Dart edge) const = 0;

	/**************************************************************************
	 * Generic map traversal                                                  *
	 * Use these functions only when you don't know the exact type of the map *
	 * to avoid the extra cost of std::function                               *
	 *************************************************************************/

	virtual void foreach_cell(CellType ct, const std::function<void(cgogn::Dart)>& func) const = 0;

	virtual void parallel_foreach_cell(CellType ct, const std::function<void(cgogn::Dart,uint32)>& func) const = 0;

	/**********************************************************
	 * MANAGE FRAME                                           *
	 *********************************************************/

	// get the frame associated to the map
	inline qoglviewer::ManipulatedFrame& get_frame() { return frame_; }

	// get the matrix of the frame associated to the map
	QMatrix4x4 get_frame_matrix() const;

private slots:

	void frame_changed();

	/*********************************************************
	 * MANAGE TRANSFORMATION MATRIX                          *
	 ********************************************************/

public slots:

	// get the frame associated to the map
	inline const QMatrix4x4& get_transformation_matrix() const { return transformation_matrix_; }

	/**********************************************************
	 * MANAGE BOUNDING BOX                                    *
	 *********************************************************/

	const cgogn::geometry::AABB<VEC3>& get_bb() { return bb_; }

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

	virtual QString get_bb_vertex_attribute_name() const = 0;

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

	/**********************************************************
	 * MANAGE DRAWING                                         *
	 *********************************************************/

public:

	virtual void draw(cgogn::rendering::DrawingType primitive) = 0;

	/**********************************************************
	 * MANAGE VBOs                                            *
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

	/**********************************************************
	 * MANAGE CELLS SETS                                      *
	 *********************************************************/

	virtual CellsSetGen* add_cells_set(CellType ct, const QString& name) = 0;
	virtual void remove_cells_set(CellType ct, const QString& name) = 0;

	CellsSetGen* get_cells_set(CellType ct, const QString& name);

public:

	template <typename FUNC>
	void foreach_cells_set(CellType ct, const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, CellsSetGen*>::value, "Wrong function parameter type");
		for (const auto& cells_set_it : cells_sets_[ct])
			f(cells_set_it.second.get());
	}

	void update_mutually_exclusive_cells_sets(CellType ct);

private slots:

	void viewer_initialized();
//	void selected_cells_changed();

	/**********************************************************
	 * MANAGE LINKED VIEWS                                    *
	 *********************************************************/

public slots:

	// get the list of views linked to the map
	inline const std::list<View*>& get_linked_views() const { return views_; }

	// test if a view is linked to this map
	inline bool is_linked_to_view(View* view) const
	{
		return std::find(views_.begin(), views_.end(), view) != views_.end();
	}

public:

	/**********************************************************
	 * MANAGE ATTRIBUTES & CONNECTIVITY                       *
	 *********************************************************/

	virtual bool is_embedded(CellType ct) const = 0;
	virtual uint32 embedding(cgogn::Dart d,CellType ct) const = 0;

	virtual const ChunkArrayContainer<uint32>* attribute_container(CellType ct) const = 0;

	void notify_attribute_change(cgogn::Orbit, const QString&);

	void notify_connectivity_change();

	virtual QStringList get_attribute_names(CellType ct) const = 0;

	virtual bool remove_attribute(CellType ct, const QString& att_name) = 0;

private:

	void link_view(View* view);
	void unlink_view(View* view);

signals:

	void bb_changed();
	void bb_vertex_attribute_changed(const QString&);

	void vbo_added(cgogn::rendering::VBO*);
	void vbo_removed(cgogn::rendering::VBO*);

//	void selected_cells_changed(CellsSetGen*);

	void attribute_added(cgogn::Orbit, const QString&);
	void attribute_removed(cgogn::Orbit, const QString&);
	void attribute_changed(cgogn::Orbit, const QString&);

	void cells_set_added(CellType, const QString&);
	void cells_set_removed(CellType, const QString&);

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
	std::array<std::map<QString, std::unique_ptr<CellsSetGen>>, NB_CELL_TYPES> cells_sets_;
};


template <typename MAP_TYPE>
class MapHandler : public MapHandlerGen
{
public:

	using ConcreteMap = typename MAP_TYPE::ConcreteMap;
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

	template <typename CELL>
	using CellSet = ::schnapps::CellsSet<MAP_TYPE, CELL>;
	using DartSet	= CellSet<CDart>;
	using VertexSet	= CellSet<Vertex>;
	using EdgeSet	= CellSet<Edge>;
	using FaceSet	= CellSet<Face>;
	using VolumeSet	= CellSet<Volume>;

	MapHandler(const QString& name, SCHNApps* s) :
		MapHandlerGen(name, s, cgogn::make_unique<MAP_TYPE>())
	{}

	~MapHandler() override
	{}

	inline MAP_TYPE* get_map() const { return static_cast<MAP_TYPE*>(this->map_.get()); }

	uint8 dimension() const override { return ConcreteMap::DIMENSION; }

	uint32 nb_cells(CellType ct) const override
	{
		switch (ct)
		{
			case CellType::Dart_Cell: return get_map()->nb_darts();
			case CellType::Vertex_Cell: return get_map()->template nb_cells<Vertex::ORBIT>();
			case CellType::Edge_Cell: return get_map()->template nb_cells<Edge::ORBIT>();
			case CellType::Face_Cell: return get_map()->template nb_cells<Face::ORBIT>();
			case CellType::Volume_Cell: return get_map()->template nb_cells<Volume::ORBIT>();
			default:
				return 0u;
		}
	}

	bool is_embedded(CellType ct) const override
	{
		switch (ct)
		{
			case CellType::Dart_Cell: return get_map()->is_embedded(CDart::ORBIT);
			case CellType::Vertex_Cell: return get_map()->is_embedded(Vertex::ORBIT);
			case CellType::Edge_Cell: return get_map()->is_embedded(Edge::ORBIT);
			case CellType::Face_Cell: return get_map()->is_embedded(Face::ORBIT);
			case CellType::Volume_Cell: return get_map()->is_embedded(Volume::ORBIT);
			default:
				return false;
		}
	}

	const ChunkArrayContainer<uint32>* attribute_container(CellType ct) const override
	{
		switch (ct)
		{
			case CellType::Dart_Cell: return &(get_map()->attribute_container(CDart::ORBIT));
			case CellType::Vertex_Cell: return &(get_map()->attribute_container(Vertex::ORBIT));
			case CellType::Edge_Cell: return &(get_map()->attribute_container(Edge::ORBIT));
			case CellType::Face_Cell: return &(get_map()->attribute_container(Face::ORBIT));
			case CellType::Volume_Cell: return &(get_map()->attribute_container(Volume::ORBIT));
			default:
				cgogn_log_warning("MapHandler::attribute_container") << "Invalid CellType \"" << cell_type_name(ct) << "\".";
				return nullptr;
		}
	}

	cgogn::Orbit orbit(CellType ct) const override
	{
		switch (ct)
		{
			case CellType::Dart_Cell: return CDart::ORBIT;
			case CellType::Vertex_Cell: return Vertex::ORBIT;
			case CellType::Edge_Cell: return Edge::ORBIT;
			case CellType::Face_Cell: return Face::ORBIT;
			case CellType::Volume_Cell: return Volume::ORBIT;
			default:
				cgogn_log_warning("MapHandler::orbit") << "Invalid CellType \"" << cell_type_name(ct) << "\".";
				return CDart::ORBIT;
		}
	}

	CellType cell_type(cgogn::Orbit orbit) const override
	{
		switch (orbit)
		{
			case CDart::ORBIT: return CellType::Dart_Cell;
			case Vertex::ORBIT: return CellType::Vertex_Cell;
			case Edge::ORBIT: return CellType::Edge_Cell;
			case Face::ORBIT: return CellType::Face_Cell;
			case Volume::ORBIT: return CellType::Volume_Cell;
			default:
			{
				cgogn_log_warning("MapHandler") << "The orbit \"" << cgogn::orbit_name(orbit) << "\" is not handled.";
				return CellType::Unknown;
			}
		}
	}

	virtual void foreach_cell(CellType ct, const std::function<void(cgogn::Dart)>& func) const override
	{
		const cgogn::Orbit orb = orbit(ct);
		switch (orb)
		{
			case CDart::ORBIT: get_map()->foreach_cell([&](CDart d) { func(d.dart); }); break;
			case Vertex::ORBIT: get_map()->foreach_cell([&](Vertex v) { func(v.dart); }); break;
			case Edge::ORBIT: get_map()->foreach_cell([&](Edge e) { func(e.dart); }); break;
			case Face::ORBIT: get_map()->foreach_cell([&](Face f) { func(f.dart); }); break;
			case Volume::ORBIT: get_map()->foreach_cell([&](Volume w) { func(w.dart); }); break;
			default: break;
		}
	}

	virtual void parallel_foreach_cell(CellType ct, const std::function<void(cgogn::Dart,uint32)>& func) const override
	{
		const cgogn::Orbit orb = orbit(ct);
		switch (orb)
		{
			case CDart::ORBIT: get_map()->parallel_foreach_cell([&](CDart d, uint32 th) { func(d.dart, th); }); break;
			case Vertex::ORBIT: get_map()->parallel_foreach_cell([&](Vertex v, uint32 th) { func(v.dart, th); }); break;
			case Edge::ORBIT: get_map()->parallel_foreach_cell([&](Edge e, uint32 th) { func(e.dart, th); }); break;
			case Face::ORBIT: get_map()->parallel_foreach_cell([&](Face f, uint32 th) { func(f.dart, th); }); break;
			case Volume::ORBIT: get_map()->parallel_foreach_cell([&](Volume w, uint32 th) { func(w.dart, th); }); break;
			default: break;
		}
	}

	virtual uint32 embedding(cgogn::Dart d, CellType ct) const override
	{
		return get_map()->embedding(d, orbit(ct));
	}

	virtual bool same_cell(cgogn::Dart d, cgogn::Dart e, CellType ct) const override
	{
		const cgogn::Orbit orb = orbit(ct);
		const auto* map = get_map();
		switch (orb)
		{
			case CDart::ORBIT: return d == e;
			case Vertex::ORBIT: return map->same_cell(Vertex(d), Vertex(e));
			case Edge::ORBIT: return map->same_cell(Edge(d), Edge(e));
			case Face::ORBIT: return map->same_cell(Face(d), Face(e));
			case Volume::ORBIT: return map->same_cell(Volume(d), Volume(e));
			default:
				return false;
		}
	}

	virtual std::pair<cgogn::Dart, cgogn::Dart> vertices(cgogn::Dart edge) const override
	{
		auto && p = get_map()->vertices(Edge(edge));
		return std::make_pair(p.first.dart, p.second.dart);
	}

	virtual bool merge(const MapHandlerGen* other_map) override
	{
		if (other_map->dimension() > this->dimension())
		{
			cgogn_log_warning("MapHandler::Merge") << "Cannot merge a map of dimension " << other_map->dimension() << " inside a map of lower dimension " << this->dimension() << '.';
			return false;
		}

		if (other_map->dimension() == this->dimension())
		{
			const MapHandler* mh = dynamic_cast<const MapHandler*>(other_map);
			typename MAP_TYPE::DartMarker dm(*this->get_map());
			this->get_map()->merge(*mh->get_map(), dm);
		} else {
			if (other_map->dimension() == 2)
			{
				const CMap2Handler* mh = dynamic_cast<const CMap2Handler*>(other_map);
				typename MAP_TYPE::DartMarker dm(*this->get_map());
				this->get_map()->merge(*mh->get_map(), dm);
			} else {
				// other non-handled cases like merging a 1-map in a 3-map or merging a 1-map in a 2-map.
				cgogn_log_warning("MapHandler::Merge") << "Merge nod handled: dimension" << other_map->dimension() << " inside a map of dimension " << this->dimension() << '.';
				return false;
			}
		}
		return true;
	}

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/

	QString get_bb_vertex_attribute_name() const override
	{
		if (bb_vertex_attribute_.is_valid())
			return QString::fromStdString(bb_vertex_attribute_.name());
		else
			return QString();
	}

	void set_bb_vertex_attribute(const QString& attribute_name) override
	{
		bb_vertex_attribute_ = get_map()->template get_attribute<VEC3, Vertex::ORBIT>(attribute_name.toStdString());
		if (bb_vertex_attribute_.is_valid())
		{
			compute_bb();
			this->update_bb_drawer();
			emit(bb_vertex_attribute_changed(attribute_name));
			emit(bb_changed());
		}
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

	inline bool has_attribute(cgogn::Orbit orbit, const QString& att_name)
	{
		return get_map()->has_attribute(orbit, att_name.toStdString());
	}

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
			QString attribute_name = QString::fromStdString(ah.name());
			const bool res =  get_map()->remove_attribute(ah);
			if (res)
				emit(attribute_removed(ORBIT, attribute_name));
			return res;
		}
		return false;
	}

	virtual bool remove_attribute(CellType ct, const QString& att_name) override
	{
		const auto orb = orbit(ct);
		const bool res = get_map()->remove_attribute(orb, att_name.toStdString());
		if (res)
			emit(attribute_removed(orb, att_name));
		return res;
	}

	template <typename T, cgogn::Orbit ORBIT>
	inline Attribute<T, ORBIT> get_attribute(const QString& attribute_name)
	{
		return get_map()->template get_attribute<T, ORBIT>(attribute_name.toStdString());
	}

	virtual QStringList get_attribute_names(CellType ct) const override
	{
		QStringList res;
		const ChunkArrayContainer<uint32>* cont = attribute_container(ct);
		if (!cont)
			return res;

		const auto& names = cont->names();
		res.reserve(int32(names.size()));
		for (const auto& name : names)
			res.push_back(QString::fromStdString(name));
		return res;
	}

	/*********************************************************
	 * MANAGE VBOs
	 *********************************************************/

	cgogn::rendering::VBO* create_vbo(const QString& name) override
	{
		cgogn::rendering::VBO* vbo = this->get_vbo(name);
		if (!vbo)
		{
			if (!has_attribute(Vertex::ORBIT, name))
				return nullptr;

			const MAP_TYPE* cmap = get_map();

			const VertexAttribute<VEC4F> va4f = cmap->template get_attribute<VEC4F, Vertex::ORBIT>(name.toStdString());
			if (va4f.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va4f, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<VEC4D> va4d = cmap->template get_attribute<VEC4D, Vertex::ORBIT>(name.toStdString());
			if (va4d.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(4)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va4d, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<VEC3F> va3f = cmap->template get_attribute<VEC3F, Vertex::ORBIT>(name.toStdString());
			if (va3f.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va3f, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<VEC3D> va3d = cmap->template get_attribute<VEC3D, Vertex::ORBIT>(name.toStdString());
			if (va3d.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va3d, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<AVEC3D> ava3d = cmap->template get_attribute<AVEC3D, Vertex::ORBIT>(name.toStdString());
			if (ava3d.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(ava3d, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<AVEC3F> ava3f = cmap->template get_attribute<AVEC3F, Vertex::ORBIT>(name.toStdString());
			if (ava3f.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(3)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(ava3f, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}


			const VertexAttribute<VEC2F> va2f = cmap->template get_attribute<VEC2F, Vertex::ORBIT>(name.toStdString());
			if (va2f.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va2f, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<VEC2D> va2d = cmap->template get_attribute<VEC2D, Vertex::ORBIT>(name.toStdString());
			if (va2d.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(2)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va2d, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<float32> vaf32 = cmap->template get_attribute<float32, Vertex::ORBIT>(name.toStdString());
			if (vaf32.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(vaf32, vbo);
				emit(vbo_added(vbo));
				return vbo;
			}

			const VertexAttribute<float64> vaf64 = cmap->template get_attribute<float64, Vertex::ORBIT>(name.toStdString());
			if (vaf64.is_valid())
			{
				this->vbos_.insert(std::make_pair(name, cgogn::make_unique<cgogn::rendering::VBO>(1)));
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(vaf64, vbo);
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
			const MAP_TYPE* cmap = get_map();

			const VertexAttribute<VEC4F> va4f = cmap->template get_attribute<VEC4F, Vertex::ORBIT>(name.toStdString());
			if (va4f.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va4f, vbo);
				return;
			}

			const VertexAttribute<VEC4D> va4d = cmap->template get_attribute<VEC4D, Vertex::ORBIT>(name.toStdString());
			if (va4d.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va4d, vbo);
				return;
			}

			const VertexAttribute<VEC3F> va3f = cmap->template get_attribute<VEC3F, Vertex::ORBIT>(name.toStdString());
			if (va3f.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va3f, vbo);
				return;
			}

			const VertexAttribute<VEC3D> va3d = cmap->template get_attribute<VEC3D, Vertex::ORBIT>(name.toStdString());
			if (va3d.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va3d, vbo);
				return;
			}

			const VertexAttribute<AVEC3F> ava3f = cmap->template get_attribute<AVEC3F, Vertex::ORBIT>(name.toStdString());
			if (ava3f.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(ava3f, vbo);
				return;
			}

			const VertexAttribute<AVEC3D> ava3d = cmap->template get_attribute<AVEC3D, Vertex::ORBIT>(name.toStdString());
			if (ava3d.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(ava3d, vbo);
				return;
			}


			const VertexAttribute<VEC2F> va2f = cmap->template get_attribute<VEC2F, Vertex::ORBIT>(name.toStdString());
			if (va2f.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va2f, vbo);
				return;
			}

			const VertexAttribute<VEC2D> va2d = cmap->template get_attribute<VEC2D, Vertex::ORBIT>(name.toStdString());
			if (va2d.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(va2d, vbo);
				return;
			}

			const VertexAttribute<float32> vaf32 = cmap->template get_attribute<float32, Vertex::ORBIT>(name.toStdString());
			if (vaf32.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(vaf32, vbo);
				return;
			}

			const VertexAttribute<float64> vaf64 = cmap->template get_attribute<float64, Vertex::ORBIT>(name.toStdString());
			if (vaf64.is_valid())
			{
				vbo = this->vbos_.at(name).get();
				cgogn::rendering::update_vbo(vaf64, vbo);
				return;
			}
		}
	}

protected:

	/*********************************************************
	 * MANAGE CELLS SETS
	 *********************************************************/

	CellsSetGen* add_cells_set(CellType ct, const QString& name) override
	{
		if (this->cells_sets_[ct].count(name) > 0ul)
			return nullptr;

		switch (ct)
		{
			case Dart_Cell:
				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, CDart>>(*this, name)));
				break;
			case Vertex_Cell:
				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Vertex>>(*this, name)));
				break;
			case Edge_Cell:
				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Edge>>(*this, name)));
				break;
			case Face_Cell:
				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Face>>(*this, name)));
				break;
			case Volume_Cell:
				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Volume>>(*this, name)));
				break;
			default:
				break;
		}

		CellsSetGen* cells_set = this->cells_sets_[ct].at(name).get();
		emit(cells_set_added(ct, name));
		connect(this, SIGNAL(connectivity_changed()), cells_set, SLOT(rebuild()));
		return cells_set;
	}

	void remove_cells_set(CellType ct, const QString& name) override
	{
		const auto cells_set_it = cells_sets_[ct].find(name);
		if (cells_set_it == cells_sets_[ct].end())
			return;
		disconnect(this, SIGNAL(connectivity_changed()), cells_set_it->second.get(), SLOT(rebuild()));
		auto tmp = std::move(cells_set_it->second); // the object shall not be destroyed before the signal 'cells_set_removed' is processed.
		cells_sets_[ct].erase(cells_set_it);
		emit(cells_set_removed(ct, name));
	}

private:

	VertexAttribute<VEC3> bb_vertex_attribute_;
};

#ifndef SCHNAPPS_CORE_MAPHANDLER_CPP_
template class SCHNAPPS_CORE_API MapHandler<CMap2>;
template class SCHNAPPS_CORE_API MapHandler<CMap3>;
#endif // SCHNAPPS_CORE_MAPHANDLER_CPP_

} // namespace schnapps

#endif // SCHNAPPS_CORE_MAPHANDLER_H_
