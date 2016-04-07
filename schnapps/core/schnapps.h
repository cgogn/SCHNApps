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
class View;

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

	/*********************************************************
	 * MANAGE VIEWS
	 *********************************************************/

public slots:

	/**
	* @brief add a view with name
	* @param name name of view
	*/
	View* add_view(const QString& name);

	/**
	* @brief add a view
	*/
	View* add_view();

	/**
	* @brief [PYTHON] remove a view
	* @param name the name of the view
	*/
	void remove_view(const QString& name);

	/**
	* @brief [PYTHON] get view object
	* @param name the name of view
	*/
	View* get_view(const QString& name) const;

	// get the set of all views
	const QMap<QString, View*>& get_view_set() const { return views_; }

	/**
	* @brief [PYTHON] get the selected view
	*/
	View* get_selected_view() const { return selected_view_; }

	/**
	* @brief [PYTHON] set the selected view
	* @param view the view object
	*/
	void set_selected_view(View* view);

	/**
	* @brief [PYTHON] set the selected view
	* @param name the view name
	*/
	void set_selected_view(const QString& name);

	/**
	* @brief [PYTHON] split the view in the current orientation
	* @param orientation of split 0: Vertical Split 1:
	* @return the new View added by the split
	*/
	View* split_view(const QString& name, Qt::Orientation orientation);

	/**
	* @brief [PYTHON] save all split positions in a string
	* @return the storage string
	*/
	QString get_split_view_positions();

	/**
	* @brief [PYTHON] restore all split positions from a string storage,
	* the split's sequence must be the same than when saving
	*/
	void set_split_view_positions(QString positions);

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

	void camera_added(Camera*);
	void camera_removed(Camera*);

	void view_added(View*);
	void view_removed(View*);
	void selected_view_changed(View*, View*);

	void schnapps_closing();

protected:

	QString app_path_;

	QMap<QString, Camera*> cameras_;
	QMap<QString, View*> views_;

	View* first_view_;
	View* selected_view_;

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
