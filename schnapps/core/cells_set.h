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

signals:

	void selected_cells_changed();

protected:

	// cells set name
	QString name_;
	bool mutually_exclusive_;
};


template <typename MAP, typename CellType>
class CellsSet : public CellsSetGen
{
public:

	using Inherit = CellsSetGen;
	using Self = CellsSet<MAP, CellType>;

	CellsSet(MAP& m, const QString& name) :
		Inherit(name),
		map_(m),
		marker_(map_)
	{}

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

protected:

	MAP& map_;
	typename MAP::template CellMarker<CellType::ORBIT> marker_;
	std::vector<CellType> cells_;
	std::vector<Self*> mutually_exclusive_sets_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_CELLS_SET_H_
