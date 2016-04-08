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

#ifndef SCHNAPPS_CORE_CAMERA_H_
#define SCHNAPPS_CORE_CAMERA_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/schnapps.h>

#include <QOGLViewer/camera.h>
#include <QOGLViewer/manipulatedCameraFrame.h>

namespace schnapps
{

class SCHNApps;
class View;

/**
* @brief The camera class inherits from [qoglviewer::Camera] (http://libqglviewer.com/refManual/classqglviewer_1_1Camera.html)
* A camera object is generated with each new view
* Cameras can be shared among views.
*
* Python callable slots are tagged with [PYTHON]
*/
class SCHNAPPS_CORE_API Camera : public qoglviewer::Camera
{
	Q_OBJECT

	friend class View;

public:

	/// camera counter for easy camera unique naming
	static unsigned int camera_count_;

	/**
	 * @brief Camera constructor
	 * @param name
	 * @param s
	 */
	Camera(const QString& name, SCHNApps* s);

	~Camera();

	/**
	 * @brief get the name of Camera object
	 * @return const ref on name
	 */
	inline const QString& get_name() const { return name_; }

public slots:

	/**
	 * @brief [PYTHON] get the name of Camera object
	 * @return name
	 */
	QString get_name() { return name_; }

	/**
	 * @brief get the schnapps objet ptr
	 * @return the ptr
	 */
	SCHNApps* get_schnapps() const { return schnapps_; }

	/**
	 * @brief [PYTHON] test if camera is used by one view
	 * @return used / not used
	 */
	bool is_used() const;

	/**
	 * @brief [PYTHON] test is camera is used by several view
	 * @return shared / not shared (by view)
	 */
	bool is_shared() const;

	/**
	 * @brief get the projection type
	 * @return PERSPECTIVE or ORTHOGRAPHIC
	 */
	qoglviewer::Camera::Type get_projection_type() const;

	// is camera drawn ?
	bool get_draw() const;

	// is camera path drawn ?
	bool get_draw_path() const;

	/**
	 * @brief get the list of views linked with the camera
	 * @warning not python callable
	 * @return the list
	 */
	const QList<View*>& get_linked_views() const;

	/**
	 * @brief is the camera linked to the given view
	 * @param view
	 * @return
	 */
	bool is_linked_to_view(View* view) const;

	/**
	* @brief [PYTHON] set the projection type
	* @param t 0:perspective / 1::orthogonal
	*/
	void set_projection_type(int t);

	// [PYTHON] draw (or not) the camera
	void set_draw(bool b);

	// [PYTHON] draw (or not) the camera path
	void set_draw_path(bool b);

	/**
	* @brief [PYTHON] Enable the camera to update automatically with view bounding box
	*/
	void enable_views_bounding_box_fitting();

	/**
	* @brief [PYTHON] Disable the camera to update automatically with view bounding box
	*/
	void disable_views_bounding_box_fitting();

	/**
	* @brief [PYTHON] store position and rotationof camera into a string
	* @return the storage string
	*/
	QString to_string();

	/**
	* @brief [PYTHON] restore a camera from string storage
	* @param cam the string containing data
	*/
	void from_string(QString camera);

private:

	void link_view(View* view);
	void unlink_view(View* view);

private slots:

	void frame_modified();
	void fit_to_views_bounding_box();

signals:

	void projection_type_changed(int);
	void draw_changed(bool);
	void draw_path_changed(bool);

protected:

	// camera name
	QString name_;

	// pointer to schnapps object
	SCHNApps* schnapps_;

	// list of views that are using this camera
	QList<View*> views_;

	bool draw_;
	bool draw_path_;

	// fit the camera to the bounding box of view
	bool fit_to_views_bounding_box_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_CAMERA_H_
