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

#ifndef SCHNAPPS_CORE_OBJECT_H_
#define SCHNAPPS_CORE_OBJECT_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/types.h>

#include <cgogn/rendering/drawer.h>
#include <cgogn/geometry/algos/bounding_box.h>

#include <QOGLViewer/manipulatedFrame.h>

#include <QObject>

namespace schnapps
{

class View;
class PluginProvider;

class SCHNAPPS_CORE_API Object : public QObject
{
	Q_OBJECT

	friend class View;

public:

	Object(const QString& name, PluginProvider* p);
	~Object() override;

	inline const QString& name() const { return name_; }
	inline QString name() { return name_; }

	inline PluginProvider* provider() const { return provider_; }

	inline const std::list<View*>& linked_views() const { return views_; }

	inline bool is_linked_to_view(View* view) const
	{
		return std::find(views_.begin(), views_.end(), view) != views_.end();
	}

	/**********************************************************
	 * FRAME                                                  *
	 *********************************************************/

	inline const qoglviewer::ManipulatedFrame& frame() const { return frame_; }
	QMatrix4x4 frame_matrix() const;

private slots:

	void frame_changed();

	/*********************************************************
	 * TRANSFORMATION MATRIX                                 *
	 ********************************************************/

public:

	inline const QMatrix4x4& transformation_matrix() const { return transformation_matrix_; }

	QVector3D scale();
	void rescale(float32 sx, float32 sy, float32 sz);

	/**********************************************************
	 * BOUNDING BOX                                           *
	 *********************************************************/

	inline const cgogn::geometry::AABB<VEC3>& bb() const { return bb_; }
	inline float bb_diagonal_size() const { return bb_diagonal_size_; }
	inline bool show_bb() const { return show_bb_; }
	bool transformed_bb(qoglviewer::Vec& bb_min, qoglviewer::Vec& bb_max) const;

	void update_bb_drawer();
	void draw_bb(View* view, const QMatrix4x4& pm, const QMatrix4x4& mm);

private:

	void link_view(View* view);
	void unlink_view(View* view);

	virtual void view_linked(View* view) = 0;
	virtual void view_unlinked(View* view) = 0;

private slots:

	void viewer_initialized();

signals:

	void bb_changed();

protected:

	// object name
	QString name_;

	// pointer to the provider of the object
	PluginProvider* provider_;

	// list of views that are linked to this plugin
	std::list<View*> views_;

	// frame that allow user object manipulation (ctrl + mouse)
	qoglviewer::ManipulatedFrame frame_;

	// transformation matrix
	QMatrix4x4 transformation_matrix_;

	// bounding box
	cgogn::geometry::AABB<VEC3> bb_;
	float bb_diagonal_size_;
	bool show_bb_;
	QColor bb_color_;
	cgogn::rendering::DisplayListDrawer bb_drawer_;
	std::map<View*, std::unique_ptr<cgogn::rendering::DisplayListDrawer::Renderer>> bb_drawer_renderer_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_OBJECT_H_
