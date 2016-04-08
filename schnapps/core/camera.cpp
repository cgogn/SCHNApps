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

#include <schnapps/core/camera.h>
#include <schnapps/core/view.h>

namespace schnapps
{

unsigned int Camera::camera_count_ = 0;

Camera::Camera(const QString& name, SCHNApps* schnapps) :
	name_(name),
	schnapps_(schnapps),
	draw_(false),
	draw_path_(false),
	fit_to_views_bounding_box_(true)
{
	++camera_count_;
	connect(this->frame(), SIGNAL(modified()), this, SLOT(frame_modified()));
}

Camera::~Camera()
{}

bool Camera::is_used() const
{
	return !views_.empty();
	return true;
}

bool Camera::is_shared() const
{
	return views_.size() > 1;
	return true;
}

qoglviewer::Camera::Type Camera::get_projection_type() const
{
	return this->type();
}

bool Camera::get_draw() const
{
	return draw_;
}

bool Camera::get_draw_path() const
{
	return draw_path_;
}

const QList<View*>& Camera::get_linked_views() const
{
	return views_;
}

bool Camera::is_linked_to_view(View* view) const
{
	return views_.contains(view);
}

void Camera::set_projection_type(int t)
{
	this->setType(qoglviewer::Camera::Type(t));
	emit(projection_type_changed(t));
	foreach(View* view, schnapps_->get_view_set().values())
		view->update();
}

void Camera::set_draw(bool b)
{
	draw_ = b;
	emit(draw_changed(b));
	foreach(View* view, schnapps_->get_view_set().values())
		view->update();
}

void Camera::set_draw_path(bool b)
{
	draw_path_ = b;
	emit(draw_path_changed(b));
	foreach(View* view, schnapps_->get_view_set().values())
		view->update();
}

void Camera::enable_views_bounding_box_fitting()
{
	fit_to_views_bounding_box_ = true;
}

void Camera::disable_views_bounding_box_fitting()
{
	fit_to_views_bounding_box_ = false;
}

QString Camera::to_string()
{
	QString res;
	QTextStream str(&res);
	qoglviewer::Vec pos = this->position();
	qoglviewer::Quaternion ori = this->orientation();
	str << pos[0] << " " << pos[1] << " " << pos[2] << " ";
	str << ori[0] << " " << ori[1] << " " << ori[2] << " " << ori[3];
	return res;
}

void Camera::from_string(QString cam)
{
	QTextStream str(&cam);
	qoglviewer::Vec pos = this->position();
	qoglviewer::Quaternion ori = this->orientation();
	str >> pos[0];
	str >> pos[1];
	str >> pos[2];
	str >> ori[0];
	str >> ori[1];
	str >> ori[2];
	str >> ori[3];
	this->setPosition(pos);
	this->setOrientation(ori);
}

void Camera::link_view(View* view)
{
	if(view && !views_.contains(view))
	{
		views_.push_back(view);
		fit_to_views_bounding_box();
		connect(view, SIGNAL(bounding_box_changed()), this, SLOT(fit_to_views_bounding_box()));
	}
}

void Camera::unlink_view(View* view)
{
	if (views_.removeOne(view))
	{
		fit_to_views_bounding_box();
		disconnect(view, SIGNAL(bounding_box_changed()), this, SLOT(fit_to_views_bounding_box()));
	}
}

void Camera::frame_modified()
{
	if(draw_ || draw_path_)
	{
		foreach(View* view, schnapps_->get_view_set().values())
			view->update();
	}
	else
	{
		foreach(View* view, views_)
			view->update();
	}
}

void Camera::fit_to_views_bounding_box()
{
	if (fit_to_views_bounding_box_)
	{
		qoglviewer::Vec bb_min;
		qoglviewer::Vec bb_max;

		if (!views_.empty())
		{
			views_.first()->get_bb(bb_min, bb_max);

			foreach(View* v, views_)
			{
				qoglviewer::Vec minbb;
				qoglviewer::Vec maxbb;
				v->get_bb(minbb, maxbb);
				for(unsigned int dim = 0; dim < 3; ++dim)
				{
					if(minbb[dim] < bb_min[dim])
						bb_min[dim] = minbb[dim];
					if(maxbb[dim] > bb_max[dim])
						bb_max[dim] = maxbb[dim];
				}
			}
		}
		else
		{
			bb_min.setValue(0, 0, 0);
			bb_max.setValue(0, 0, 0);
		}

		this->setSceneBoundingBox(bb_min, bb_max);
		this->showEntireScene();
	}
}

} // namespace schnapps
