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

#include <QOGLViewer/manipulatedFrame.h>

#include <QObject>
#include <QString>

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
	 * @brief get the name of Camera object
	 * @return name
	 */
	QString get_name() { return name_; }

	/**
	 * @brief get the schnapps objet ptr
	 * @return the ptr
	 */
	SCHNApps* get_schnapps() const { return schnapps_; }

	const MapBaseData* get_map() const { return map_; }

	bool is_selected_map() const;

	/*********************************************************
	 * MANAGE FRAME
	 *********************************************************/

	// get the frame associated to the map
	qoglviewer::ManipulatedFrame* get_frame() const { return frame_; }

	// get the matrix of the frame associated to the map
	QMatrix4x4 get_frame_matrix() const;

private slots:

	void frame_modified();

	/*********************************************************
	 * MANAGE LINKED VIEWS
	 *********************************************************/

public slots:

	// get the list of views linked to the map
	const QList<View*>& get_linked_views() const { return views_; }

	// test if a view is linked to this map
	bool is_linked_to_view(View* view) const { return views_.contains(view); }

private:

	void link_view(View* view);

	void unlink_view(View* view);

signals:

	void bounding_box_modified();

protected:

	// MapHandler name
	QString name_;

	// pointer to schnapps object
	SCHNApps* schnapps_;

	// MapBaseData generic pointer
	MapBaseData* map_;

	// frame that allow user object manipulation (ctrl + mouse)
	qoglviewer::ManipulatedFrame* frame_;

	// list of views that are linked to this map
	QList<View*> views_;
};

template <typename MAP_TYPE>
class MapHandler : public MapHandlerGen
{
public:

	MapHandler(const QString& name, SCHNApps* s, MAP_TYPE* map) :
		MapHandlerGen(name, s, map)
	{}

	~MapHandler()
	{
		delete map_;
	}

	inline MAP_TYPE* get_map() { return static_cast<MAP_TYPE*>(map_); }
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_MAPHANDLER_H_
