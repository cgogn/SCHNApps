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

#ifndef SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP_CellTypeS_SET_H_
#define SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP_CellTypeS_SET_H_

#include <schnapps/plugins/cmap_provider/plugin_cmap_provider_export.h>

#include <schnapps/core/types.h>

#include <cgogn/core/basic/cell.h>

#include <QObject>

namespace schnapps
{

class Object;

namespace plugin_cmap_provider
{

class PLUGIN_CMAP_PROVIDER_EXPORT CMapCellsSetGen : public QObject
{
	Q_OBJECT

public:

	// counter for cells set unique naming
	static uint32 cells_set_count_;

	CMapCellsSetGen(const QString& name);
	~CMapCellsSetGen();

	inline const QString& name() { return name_; }

	virtual Object* map_handler() const = 0;

	virtual cgogn::Orbit orbit() const = 0;
	virtual std::size_t nb_cells() const = 0;

	inline bool is_mutually_exclusive() { return mutually_exclusive_; }
	virtual void set_mutually_exclusive(bool b) = 0;

//	virtual void foreach_cell(const std::function<void(cgogn::Dart)>& func) const = 0;

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

template <typename CMapHandler, typename CellType>
class CMapCellsSet : public CMapCellsSetGen
{
public:

	using Inherit = CMapCellsSetGen;
	using Self = CMapCellsSet<CMapHandler, CellType>;

	using MapType = typename CMapHandler::MapType;

	CMapCellsSet(CMapHandler& mh, const QString& name) :
		Inherit(name),
		mh_(mh),
		marker_(*mh.map())
	{}

	~CMapCellsSet() override
	{}

	inline Object* map_handler() const override
	{
		return &mh_;
	}

	inline cgogn::Orbit orbit() const override
	{
		return CellType::ORBIT;
	}

	inline std::size_t nb_cells() const override
	{
		return cells_.size();
	}

	inline void select(CellType c, bool emit_signal = true)
	{
		if (!marker_.is_marked(c))
		{
			marker_.mark(c);
			cells_.insert(std::make_pair(mh_.map()->embedding(c), c));
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

	inline void select(const std::vector<CellType>& cells)
	{
		for (CellType c : cells)
			select(c, false);
		this->emit_if_selection_changed();
		if (this->mutually_exclusive_ && !mutually_exclusive_sets_.empty())
		{
			for (Self* cs : mutually_exclusive_sets_)
				cs->emit_if_selection_changed();
		}
	}

	inline void unselect(CellType c, bool emit_signal = true)
	{
		if(marker_.is_marked(c))
		{
			uint32 emb = mh_.map()->embedding(c);
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

	inline void unselect(const std::vector<CellType>& cells)
	{
		for (CellType c : cells)
			unselect(c, false);
		this->emit_if_selection_changed();
	}

	inline bool is_selected(CellType c)
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

private:

	inline void set_mutually_exclusive_sets(const std::vector<Self*>& mex)
	{
		mutually_exclusive_sets_.clear();
		if (this->is_mutually_exclusive())
		{
			for (Self* cs : mex)
			{
				if (cs != this)
					mutually_exclusive_sets_.push_back(cs);
			}
		}
	}

public:

	inline void set_mutually_exclusive(bool b) override
	{
		this->mutually_exclusive_ = b;

		std::vector<Self*> mex;
		mh_.foreach_cells_set(orbit(), [&] (Inherit* cs)
		{
			if (cs->is_mutually_exclusive())
				mex.push_back(static_cast<Self*>(cs));
		});
		mh_.foreach_cells_set(orbit(), [&] (Inherit* cs)
		{
			static_cast<Self*>(cs)->set_mutually_exclusive_sets(mex);
		});

		mh_.notify_cells_set_mutually_exclusive_change(orbit(), this->name_);
	}

	inline void rebuild() override
	{
		cells_.clear();
		mh_.map()->foreach_cell([&] (CellType c)
		{
			if (marker_.is_marked(c))
				cells_.insert(std::make_pair(mh_.map()->embedding(c), c));
		});
		emit(selected_cells_changed());
	}

	template <typename FUNC>
	inline void foreach_cell(const FUNC& f)
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, CellType>::value, "Wrong function parameter type");
		for (const auto& cell : cells_)
			f(cell.second);
	}

//	virtual void foreach_cell(const std::function<void(cgogn::Dart)>& func) const override
//	{
//		for (const auto& cell : cells_)
//			func(cell.second.dart);
//	}

	virtual void select(cgogn::Dart d, bool emit_signal) override
	{
		this->select(CellType(d), emit_signal);
	}

	virtual void select(const std::vector<cgogn::Dart>& cells) override
	{
		this->select(reinterpret_cast<const std::vector<CellType>&>(cells));
	}

	virtual void unselect(cgogn::Dart d, bool emit_signal) override
	{
		this->unselect(CellType(d), emit_signal);
	}

	virtual void unselect(const std::vector<cgogn::Dart>& cells) override
	{
		this->unselect(reinterpret_cast<const std::vector<CellType>&>(cells));
	}

	virtual bool is_selected(cgogn::Dart d) override
	{
		return this->is_selected(CellType(d));
	}

protected:

	CMapHandler& mh_;
	typename MapType::template CellMarker<CellType::ORBIT> marker_;
	std::map<uint32, CellType> cells_;
	std::vector<Self*> mutually_exclusive_sets_;
};

} // namespace plugin_cmap_provider

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP_CellTypeS_SET_H_
