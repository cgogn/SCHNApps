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

#ifndef SCHNAPPS_CORE_CELLS_SET_H_
#define SCHNAPPS_CORE_CELLS_SET_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/types.h>

#include <cgogn/core/basic/cell.h>

#include <QObject>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

class SCHNAPPS_CORE_API CellsSetGen : public QObject
{
	Q_OBJECT

public:

	// counter for cells set unique naming
	static uint32 cells_set_count_;

	/**
	 * @brief CellsSet constructor
	 * @param name
	 * @param s
	 */
	CellsSetGen(const QString& name);

	~CellsSetGen();

	inline const QString& get_name() { return name_; }

	virtual const MapHandlerGen* get_map() const = 0;
	virtual CellType get_cell_type() const = 0;
	virtual std::size_t get_nb_cells() const = 0;

	inline void set_mutually_exclusive(bool b) { mutually_exclusive_ = b; }
	inline bool is_mutually_exclusive() { return mutually_exclusive_; }
	virtual void set_mutually_exclusive_sets(const std::vector<CellsSetGen*>& mex) = 0;

	virtual void foreach_cell(const std::function<void(cgogn::Dart)>& func) const = 0;

	virtual void select(cgogn::Dart d, bool emit_signal = true) = 0;
	virtual void select(const std::vector<cgogn::Dart>& cells) = 0;
	virtual void unselect(cgogn::Dart d, bool emit_signal = true) = 0;
	virtual void unselect(const std::vector<cgogn::Dart>& cells) = 0;
	virtual bool is_selected(cgogn::Dart d) = 0;
	virtual void clear() = 0;

private slots:

	virtual void rebuild() = 0;

signals:

	void selected_cells_changed();

protected:

	inline void emit_if_selection_changed()
	{
		if (selection_changed_)
			emit(selected_cells_changed());
		selection_changed_ = false;
	}

	QString name_;
	bool mutually_exclusive_;
	bool selection_changed_;
};

template <typename MAP_TYPE>
class MapHandler;

template <typename MAP, typename CELL>
class CellsSet : public CellsSetGen
{
public:

	using Inherit = CellsSetGen;
	using Self = CellsSet<MAP, CELL>;

	using Inherit::select;
	using Inherit::unselect;

	CellsSet(MapHandler<MAP>& m, const QString& name) :
		Inherit(name),
		map_(m),
		marker_(*map_.get_map())
	{}

	~CellsSet() override
	{}

	inline const MapHandlerGen* get_map() const override
	{
		return &map_;
	}

	inline CellType get_cell_type() const override;

	inline std::size_t get_nb_cells() const override
	{
		return cells_.size();
	}

	inline void select(CELL c, bool emit_signal = true)
	{
		if (!marker_.is_marked(c))
		{
			marker_.mark(c);
			cells_.insert(std::make_pair(map_.get_map()->embedding(c), c));
			if (this->mutually_exclusive_ && !mutually_exclusive_sets_.empty())
			{
				for (Self* cs : mutually_exclusive_sets_)
					cs->unselect(c, emit_signal);
			}
			if (emit_signal)
				emit(selected_cells_changed());
			else
				selection_changed_ = true;
		}
	}

	inline void select(const std::vector<CELL>& cells)
	{
		for (CELL c : cells)
			select(c, false);
		this->emit_if_selection_changed();
		if (this->mutually_exclusive_ && !mutually_exclusive_sets_.empty())
		{
			for (Self* cs : mutually_exclusive_sets_)
				cs->emit_if_selection_changed();
		}
	}

	inline void unselect(CELL c, bool emit_signal = true)
	{
		if(marker_.is_marked(c))
		{
			uint32 emb = map_.get_map()->embedding(c);
			auto it = cells_.find(emb);
			if (it != cells_.end())
			{
				marker_.unmark(it->second);
				cells_.erase(it);
				if (emit_signal)
					emit(selected_cells_changed());
				else
					selection_changed_ = true;
			}
		}
	}

	inline void unselect(const std::vector<CELL>& cells)
	{
		for (CELL c : cells)
			unselect(c, false);
		this->emit_if_selection_changed();
	}

	inline bool is_selected(CELL c)
	{
		return marker_.is_marked(c);
	}

	inline void clear() override
	{
		bool was_not_empty = cells_.size() > 0;
		cells_.clear();
		marker_.unmark_all();
		if (was_not_empty)
			emit(selected_cells_changed());
	}

	inline void set_mutually_exclusive_sets(const std::vector<Inherit*>& mex) override
	{
		mutually_exclusive_sets_.clear();
		for (Inherit* cs : mex)
		{
			if (cs != this)
			{
				Self* s = dynamic_cast<Self*>(cs);
				if (s)
					mutually_exclusive_sets_.push_back(s);
			}
		}
	}

	inline void rebuild() override;

	template <typename FUNC>
	inline void foreach_cell(const FUNC& f)
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, CELL>::value, "Wrong function parameter type");
		for (const auto& cell : cells_)
			f(cell.second);
	}

	virtual void foreach_cell(const std::function<void(cgogn::Dart)>& func) const override
	{
		for (const auto& cell : cells_)
			func(cell.second.dart);
	}

	virtual void select(cgogn::Dart d, bool emit_signal) override
	{
		this->select(CELL(d), emit_signal);
	}

	virtual void select(const std::vector<cgogn::Dart>& cells) override
	{
		this->select(reinterpret_cast<const std::vector<CELL>&>(cells));
	}

	virtual void unselect(cgogn::Dart d, bool emit_signal) override
	{
		this->unselect(CELL(d), emit_signal);
	}

	virtual void unselect(const std::vector<cgogn::Dart>& cells) override
	{
		this->unselect(reinterpret_cast<const std::vector<CELL>&>(cells));
	}

	virtual bool is_selected(cgogn::Dart d) override
	{
		return this->is_selected(CELL(d));
	}

protected:

	MapHandler<MAP>& map_;
	typename MAP::template CellMarker<CELL::ORBIT> marker_;
	std::map<uint32, CELL> cells_;
	std::vector<Self*> mutually_exclusive_sets_;
};

} // namespace schnapps

#include <schnapps/core/map_handler.h>

namespace schnapps
{

template <typename MAP, typename CELL>
inline CellType CellsSet<MAP, CELL>::get_cell_type() const
{
	return map_.cell_type(CELL::ORBIT);
}

template <typename MAP, typename CELL>
inline void CellsSet<MAP, CELL>::rebuild()
{
	cells_.clear();
	map_.get_map()->foreach_cell([&] (CELL c)
	{
		if (marker_.is_marked(c))
			cells_.insert(std::make_pair(map_.get_map()->embedding(c), c));
	});
	emit(selected_cells_changed());
}

} // namespace schnapps

#endif // SCHNAPPS_CORE_CELLS_SET_H_
