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

#ifndef CORE_SCHNAPPS_H_
#define CORE_SCHNAPPS_H_

#include <core/dll.h>

#include <ui_schnapps.h>

#include <QTextStream>

class QVBoxLayout;
class QSplitter;
class QFile;

namespace schnapps
{

class Camera;
// class ControlDock_CameraTab;
// class ControlDock_MapTab;
// class ControlDock_PluginTab;

/**
 * @brief The SCHNApps central object application
 */
class SCHNAPPS_CORE_API SCHNApps : public QMainWindow, Ui::SCHNApps
{
	Q_OBJECT

public:

	SCHNApps(const QString& app_path);
	~SCHNApps();

public slots:

	/**
	 * @brief get the file path where application has been launched
	 * @return the path
	 */
	const QString& get_app_path() { return app_path_; }

	/*********************************************************
	 * MANAGE CAMERAS
	 *********************************************************/

public slots:

	/**
	* @brief [PYTHON] add a camera with a given name
	* @param name name of camera
	*/
	Camera* add_camera(const QString& name);

	/**
	 * @brief [PYTHON] add a camera (name automatically)
	 * @return
	 */
	Camera* add_camera();

	/**
	* @brief [PYTHON] remove a camera
	* @param name name of camera to remove
	*/
	void remove_camera(const QString& name);

	/**
	* @brief [PYTHON] get camera object
	* @param name of camera
	*/
	Camera* get_camera(const QString& name) const;

	// get the set of all cameras
	const QMap<QString, Camera*>& get_camera_set() const { return cameras_; }

public slots:

	void about_SCHNApps();
	void about_CGoGN();

	void toggle_control_dock();

	/**
	* @brief Print a message in the status bar
	* @param msg the message
	* @param msec number of milli-second that message stay printed
	*/
	void status_bar_message(const QString& msg, int msec);

	/**
	* @brief Set the window size
	* @param w width of window
	* @param h height of window
	*/
	inline void set_window_size(int w, int h) { this->resize(w, h); }

protected:

	void closeEvent(QCloseEvent *event);

signals:

	void schnapps_closing();

protected:

	QString app_path_;

	QMap<QString, Camera*> cameras_;

	QDockWidget* control_dock_;
	QTabWidget* control_dock_tab_widget_;
	// ControlDock_CameraTab* control_camera_tab_;
	// ControlDock_MapTab* control_map_tab_;
	// ControlDock_PluginTab* control_plugin_tab_;

	QVBoxLayout* central_layout_;
	QSplitter* root_splitter_;
	bool root_splitter_initialized_;
};

} // namespace schnapps

#endif // CORE_SCHNAPPS_H_
