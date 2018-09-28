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

#ifndef SCHNAPPS_PLUGIN_CMAP2_PROVIDER_CMAP2_HANDLER_H_
#define SCHNAPPS_PLUGIN_CMAP2_PROVIDER_CMAP2_HANDLER_H_

#include <schnapps/plugins/cmap2_provider/dll.h>
//#include <schnapps/plugins/cmap2_provider/cmap2_cells_set.h>

#include <schnapps/core/types.h>
#include <schnapps/core/object.h>

#include <cgogn/geometry/algos/bounding_box.h>

#include <cgogn/rendering/map_render.h>

#include <QString>

namespace schnapps
{

class SCHNApps;
class PluginProvider;

namespace plugin_cmap2_provider
{

class SCHNAPPS_PLUGIN_CMAP2_PROVIDER_API CMap2Handler : public Object
{
	Q_OBJECT

public:

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	CMap2Handler(const QString& name, PluginProvider* p);
	~CMap2Handler();

public:

	/**********************************************************
	 * BASIC FUNCTIONS                                        *
	 *********************************************************/

	inline CMap2* map() const { return map_; }

	void view_linked(View*) {}
	void view_unlinked(View*) {}

	/**********************************************************
	 * MANAGE DRAWING                                         *
	 *********************************************************/

	void draw(cgogn::rendering::DrawingType primitive);
	void init_primitives(cgogn::rendering::DrawingType primitive);

	/**********************************************************
	 * MANAGE VBOs                                            *
	 *********************************************************/

	cgogn::rendering::VBO* create_vbo(const QString& name);
	void update_vbo(const QString& name);
	cgogn::rendering::VBO* vbo(const QString& name) const;
	void delete_vbo(const QString& name);

	template <typename FUNC>
	void foreach_vbo(const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, cgogn::rendering::VBO*>::value, "Wrong function parameter type");
		for (const auto& vbo_it : vbos_)
			f(vbo_it.second.get());
	}

//	/**********************************************************
//	 * MANAGE CELLS SETS                                      *
//	 *********************************************************/

//	template <typename CellType>
//	CMap2CellsSet<CellType>* add_cells_set(QString name = "")
//	{

//	}

//	template <typename CellType>
//	void remove_cells_set(const QString& name)
//	{

//	}

//	template <typename CellType>
//	CMap2CellsSet<CellType> cells_set(const QString& name)
//	{

//	}

//	template <typename CellType, typename FUNC>
//	CMap2CellsSet<CellType> foreach_cells_set(const FUNC& f)
//	{
//		static_assert(cgogn::is_func_parameter_same<FUNC, CMap2CellsSet<CellType>*>::value, "Wrong function parameter type");
//		static const cgogn::Orbit ORBIT = CellType::ORBIT;
//		for (const auto& cells_set_it : cells_sets_[ORBIT])
//			f(cells_set_it.second);
//	}

//	template <typename CellType>
//	void notify_cells_set_mutually_exclusive_change(const QString& name)
//	{
//		static const cgogn::Orbit ORBIT = CellType::ORBIT;
//		emit(cells_set_mutually_exclusive_changed(ORBIT, name));
//	}

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/

	QString bb_vertex_attribute_name() const
	{
		if (bb_vertex_attribute_.is_valid())
			return QString::fromStdString(bb_vertex_attribute_.name());
		else
			return QString();
	}

	void set_bb_vertex_attribute(const QString& attribute_name)
	{
		bb_vertex_attribute_ = map_->template get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name.toStdString());
		if (bb_vertex_attribute_.is_valid())
		{
			compute_bb();
			this->update_bb_drawer();
			emit(bb_vertex_attribute_changed(attribute_name));
			emit(bb_changed());
		}
	}

	void check_bb_vertex_attribute(cgogn::Orbit orbit, const QString& attribute_name)
	{
		if (bb_vertex_attribute_.is_valid())
		{
			QString bb_vertex_attribute_name = QString::fromStdString(bb_vertex_attribute_.name());
			if (orbit == CMap2::Vertex::ORBIT && attribute_name == bb_vertex_attribute_name)
			{
				compute_bb();
				this->update_bb_drawer();
				emit(bb_changed());
			}
		}
	}

private:

	void compute_bb()
	{
		this->bb_.reset();

		if (bb_vertex_attribute_.is_valid())
			cgogn::geometry::compute_AABB(bb_vertex_attribute_, this->bb_);

		if (this->bb_.is_initialized())
			this->bb_diagonal_size_ = this->bb_.diag_size();
		else
			this->bb_diagonal_size_ = .0f;
	}

	/**********************************************************
	 * MANAGE ATTRIBUTES & CONNECTIVITY                       *
	 *********************************************************/

public:

	template <typename T, cgogn::Orbit ORBIT>
	cgogn::Attribute<T, ORBIT> add_attribute(const QString& attribute_name)
	{
		cgogn::Attribute<T, ORBIT> a = map_->template add_attribute<T, ORBIT>(attribute_name.toStdString());
		if (a.is_valid())
			notify_attribute_added(ORBIT, attribute_name);
		return a;
	}

	template <typename T, cgogn::Orbit ORBIT>
	bool remove_attribute(cgogn::Attribute<T, ORBIT>& ah)
	{
		if (ah.is_valid())
		{
			QString attribute_name = QString::fromStdString(ah.name());
			const bool res =  map_->remove_attribute(ah);
			if (res)
				emit(attribute_removed(ORBIT, attribute_name));
			return res;
		}
		return false;
	}

	bool remove_attribute(cgogn::Orbit orbit, const QString& att_name)
	{
		const bool res = map()->remove_attribute(orbit, att_name.toStdString());
		if (res)
			emit(attribute_removed(orbit, att_name));
		return res;
	}

	void notify_attribute_added(cgogn::Orbit, const QString&);
	void notify_attribute_removed(cgogn::Orbit, const QString&);
	void notify_attribute_change(cgogn::Orbit, const QString&);

	void notify_connectivity_change();

	void lock_topo_access() { topo_access_.lock(); }
	void unlock_topo_access() { topo_access_.unlock(); }

//	/*********************************************************
//	 * MANAGE CELLS SETS
//	 *********************************************************/

//	CellsSetGen* add_cells_set(CellType ct, QString name = "") override
//	{
//		if (name.isEmpty())
//			name = QString::fromStdString(cell_type_name(ct)) + QString("_set_") + QString::number(CellsSetGen::cells_set_count_);

//		if (this->cells_sets_[ct].count(name) > 0ul)
//			return nullptr;

//		switch (ct)
//		{
//			case Dart_Cell:
//				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, CDart>>(*this, name)));
//				break;
//			case Vertex_Cell:
//				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Vertex>>(*this, name)));
//				break;
//			case Edge_Cell:
//				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Edge>>(*this, name)));
//				break;
//			case Face_Cell:
//				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Face>>(*this, name)));
//				break;
//			case Volume_Cell:
//				this->cells_sets_[ct].insert(std::make_pair(name, cgogn::make_unique<CellsSet<MAP_TYPE, Volume>>(*this, name)));
//				break;
//			default:
//				break;
//		}

//		CellsSetGen* cells_set = this->cells_sets_[ct].at(name).get();
//		emit(cells_set_added(ct, name));
//		connect(this, SIGNAL(connectivity_changed()), cells_set, SLOT(rebuild()));
//		return cells_set;
//	}

//	void remove_cells_set(CellType ct, const QString& name) override
//	{
//		const auto cells_set_it = cells_sets_[ct].find(name);
//		if (cells_set_it == cells_sets_[ct].end())
//			return;
//		disconnect(this, SIGNAL(connectivity_changed()), cells_set_it->second.get(), SLOT(rebuild()));
//		auto tmp = std::move(cells_set_it->second); // the object shall not be destroyed before the signal 'cells_set_removed' is processed.
//		cells_sets_[ct].erase(cells_set_it);
//		emit(cells_set_removed(ct, name));
//	}

signals:

	void bb_vertex_attribute_changed(const QString&);

	void vbo_added(cgogn::rendering::VBO*);
	void vbo_removed(cgogn::rendering::VBO*);

	void attribute_added(cgogn::Orbit, const QString&);
	void attribute_removed(cgogn::Orbit, const QString&);
	void attribute_changed(cgogn::Orbit, const QString&);

//	void cells_set_added(cgogn::Orbit, const QString&);
//	void cells_set_removed(cgogn::Orbit, const QString&);
//	void cells_set_selected_cells_changed(cgogn::Orbit, const QString&);
//	void cells_set_mutually_exclusive_changed(cgogn::Orbit, const QString&);

	void connectivity_changed();

protected:

	CMap2* map_;

	std::mutex topo_access_;

	// MapRender object of the map
	cgogn::rendering::MapRender render_;

	// VBO managed for the map attributes
	std::map<QString, std::unique_ptr<cgogn::rendering::VBO>> vbos_;

	// CellsSets of the map
//	std::array<std::map<QString, CMap2CellsSetGen*>, cgogn::NB_ORBITS> cells_sets_;

	CMap2::VertexAttribute<VEC3> bb_vertex_attribute_;
};

} // namespace plugin_cmap2_provider

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CMAP2_PROVIDER_CMAP2_HANDLER_H_
