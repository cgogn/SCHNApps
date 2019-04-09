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

#ifndef SCHNAPPS_CORE_CONTROL_DOCK_CAMERA_TAB_H_
#define SCHNAPPS_CORE_CONTROL_DOCK_CAMERA_TAB_H_

#include <schnapps/core/schnapps_core_export.h>


#include <ui_control_dock_camera_tab_widget.h>

#include <QWidget>
#include <QString>

namespace schnapps
{

class SCHNApps;
class Camera;
class View;

class SCHNAPPS_CORE_EXPORT ControlDock_CameraTab : public QWidget, public Ui::ControlDock_CameraTabWidget
{
	Q_OBJECT

public:

	ControlDock_CameraTab(SCHNApps* s);
	QString title() { return QString("Cameras"); }

private slots:
	void display_camera_widget();

	// slots called from UI actions
	void add_camera_button_clicked();
	void selected_camera_changed();
	void camera_projection_changed(QAbstractButton* b);
	void camera_draw_clicked(bool b);
	void camera_draw_path_clicked(bool b);

	// slots called from SCHNApps signals
	void camera_added(Camera* c);
	void camera_removed(Camera* c);

	// slots called from selected Camera signals
	void selected_camera_projection_type_changed(int t);
	void selected_camera_draw_changed(bool b);
	void selected_camera_draw_path_changed(bool b);

private:

	void update_selected_camera_info();

protected:

	SCHNApps* schnapps_;
	Camera* selected_camera_;
	bool updating_ui_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_CONTROL_DOCK_CAMERA_TAB_H_
