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

#include <schnapps/core/control_dock_camera_tab.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/camera.h>
#include <schnapps/core/view.h>

#include <QMessageBox>

namespace schnapps
{

ControlDock_CameraTab::ControlDock_CameraTab(SCHNApps* s) :
	schnapps_(s),
	selected_camera_(nullptr),
	updating_ui_(false)
{
	setupUi(this);

	// connect UI signals
	connect(button_addCamera, SIGNAL(clicked()), this, SLOT(add_camera_button_clicked()));
	connect(list_cameras, SIGNAL(itemSelectionChanged()), this, SLOT(selected_camera_changed()));
	connect(group_projectionType, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(camera_projection_changed(QAbstractButton*)));
	connect(check_drawCamera, SIGNAL(clicked(bool)), this, SLOT(camera_draw_clicked(bool)));
	connect(check_drawCameraPath, SIGNAL(clicked(bool)), this, SLOT(camera_draw_path_clicked(bool)));

	// connect SCHNApps signals
	connect(schnapps_, SIGNAL(camera_added(Camera*)), this, SLOT(camera_added(Camera*)));
	connect(schnapps_, SIGNAL(camera_removed(Camera*)), this, SLOT(camera_removed(Camera*)));
}

void ControlDock_CameraTab::add_camera_button_clicked()
{
	if (!updating_ui_)
		schnapps_->add_camera();
}

void ControlDock_CameraTab::selected_camera_changed()
{
	if (!updating_ui_)
	{
		Camera* old = selected_camera_;

		if (old)
		{
			disconnect(selected_camera_, SIGNAL(projection_type_changed(int)), this, SLOT(selected_camera_projection_type_changed(int)));
			disconnect(selected_camera_, SIGNAL(draw_changed(bool)), this, SLOT(selected_camera_draw_changed(bool)));
			disconnect(selected_camera_, SIGNAL(draw_path_changed(bool)), this, SLOT(selected_camera_draw_path_changed(bool)));
		}

		QList<QListWidgetItem*> items = list_cameras->selectedItems();
		if(!items.empty())
		{
			QString selected_camera_name = items[0]->text();
			selected_camera_ = schnapps_->get_camera(selected_camera_name);

			connect(selected_camera_, SIGNAL(projection_type_changed(int)), this, SLOT(selected_camera_projection_type_changed(int)));
			connect(selected_camera_, SIGNAL(draw_changed(bool)), this, SLOT(selected_camera_draw_changed(bool)));
			connect(selected_camera_, SIGNAL(draw_path_changed(bool)), this, SLOT(selected_camera_draw_path_changed(bool)));
		}
		else
			selected_camera_ = nullptr;

		update_selected_camera_info();
	}
}

void ControlDock_CameraTab::camera_projection_changed(QAbstractButton* b)
{
	if (!updating_ui_ && selected_camera_)
	{
		if(radio_orthographicProjection->isChecked())
			selected_camera_->set_projection_type(qoglviewer::Camera::ORTHOGRAPHIC);
		else if(radio_perspectiveProjection->isChecked())
			selected_camera_->set_projection_type(qoglviewer::Camera::PERSPECTIVE);
	}
}

void ControlDock_CameraTab::camera_draw_clicked(bool b)
{
	if (!updating_ui_)
		selected_camera_->set_draw(b);
}

void ControlDock_CameraTab::camera_draw_path_clicked(bool b)
{
	if (!updating_ui_)
		selected_camera_->set_draw_path(b);
}





void ControlDock_CameraTab::camera_added(Camera* c)
{
	updating_ui_ = true;
	list_cameras->addItem(c->get_name());
	updating_ui_ = false;
}

void ControlDock_CameraTab::camera_removed(Camera* c)
{
	QList<QListWidgetItem*> items = list_cameras->findItems(c->get_name(), Qt::MatchExactly);
	if (!items.empty())
	{
		updating_ui_ = true;
		delete items[0];
		updating_ui_ = false;
	}
}





void ControlDock_CameraTab::selected_camera_projection_type_changed(int t)
{
	updating_ui_ = true;
	switch (t)
	{
		case qoglviewer::Camera::ORTHOGRAPHIC : radio_orthographicProjection->setChecked(true); break;
		case qoglviewer::Camera::PERSPECTIVE : radio_perspectiveProjection->setChecked(true); break;
	}
	updating_ui_ = false;
}

void ControlDock_CameraTab::selected_camera_draw_changed(bool b)
{
	updating_ui_ = true;
	if (b)
		check_drawCamera->setCheckState(Qt::Checked);
	else
		check_drawCamera->setCheckState(Qt::Unchecked);
	updating_ui_ = false;
}

void ControlDock_CameraTab::selected_camera_draw_path_changed(bool b)
{
	updating_ui_ = true;
	if (b)
		check_drawCameraPath->setCheckState(Qt::Checked);
	else
		check_drawCameraPath->setCheckState(Qt::Unchecked);
	updating_ui_ = false;
}





void ControlDock_CameraTab::update_selected_camera_info()
{
	updating_ui_ = true;

	if (selected_camera_->get_projection_type() == qoglviewer::Camera::PERSPECTIVE)
		radio_perspectiveProjection->setChecked(true);
	else if (selected_camera_->get_projection_type() == qoglviewer::Camera::ORTHOGRAPHIC)
		radio_orthographicProjection->setChecked(true);

	check_drawCamera->setChecked(selected_camera_->get_draw());

	check_drawCameraPath->setChecked(selected_camera_->get_draw_path());

	updating_ui_ = false;
}

} // namespace schnapps
