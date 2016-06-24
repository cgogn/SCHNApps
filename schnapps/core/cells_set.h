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

class SCHNAPPS_CORE_API CellsSetGen : public QObject
{
	Q_OBJECT

public:

	// counter for easy cells set unique naming
	static uint32 cells_set_count_;

	/**
	 * @brief CellsSet constructor
	 * @param name
	 * @param s
	 */
	CellsSetGen(const QString& name);

	~CellsSetGen();

	inline const QString& get_name() { return name_; }

	inline void set_mutually_exclusive(bool b) { mutually_exclusive_ = b; }
	inline bool is_mutually_exclusive() { return mutually_exclusive_; }
	virtual void set_mutually_exclusive_sets(const std::vector<CellsSetGen*>& mex) = 0;

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

	// cells set name
	QString name_;
	bool mutually_exclusive_;
	bool selection_changed_;
};


template <typename MAP, typename CELL>
class CellsSet : public CellsSetGen
{
public:

	using Inherit = CellsSetGen;
	using Self = CellsSet<MAP, CELL>;

	CellsSet(MAP& m, const QString& name) :
		Inherit(name),
		map_(m),
		marker_(map_)
	{}

	~CellsSet()
	{}

	inline void select(CELL c, bool emit_signal = true)
	{
		if (!marker_.is_marked(c))
		{
			marker_.mark(c);
			cells_.push_back(c);
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
			uint32 emb = map_.embedding(c);
			bool found = false;
			std::size_t i;
			for (i = 0; i < cells_.size() && !found; ++i)
			{
				if(map_.embedding(cells_[i]) == emb)
					found = true ;
			}
			if (found)
			{
				marker_.unmark(cells_[i-1]);
				cells_[i-1] = cells_.back();
				cells_.pop_back();
				if(emit_signal)
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

	inline void set_mutually_exclusive_sets(const std::vector<Inherit*>& mex) override
	{
		mutually_exclusive_sets_.clear();
		for (Inherit* cs : mex)
		{
			if(cs != this)
			{
				Self* s = dynamic_cast<Self*>(cs);
				if (s)
					mutually_exclusive_sets_.push_back(s);
			}
		}
	}

	inline void rebuild() override
	{
		cells_.clear();
		map_.foreach_cell([&] (CELL c)
		{
			if(marker_.is_marked(c))
				cells_.push_back(c);
		});
		emit(selected_cells_changed());
	}

	inline std::size_t get_nb_cells() const
	{
		return cells_.size();
	}

	template <typename FUNC>
	void foreach_cell(const FUNC& f)
	{
		static_assert(check_func_parameter_type(FUNC, CELL), "Wrong function parameter type");
		for (const CELL& cell : cells_)
			f(cell);
	}

protected:

	MAP& map_;
	typename MAP::template CellMarker<CELL::ORBIT> marker_;
	std::vector<CELL> cells_;
	std::vector<Self*> mutually_exclusive_sets_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_CELLS_SET_H_
