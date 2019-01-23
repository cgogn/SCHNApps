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

#ifndef SCHNAPPS_CORE_VIEW_H_
#define SCHNAPPS_CORE_VIEW_H_

#include <QColorDialog>

#include <schnapps/core/schnapps_core_export.h>

#include <schnapps/core/types.h>

#include <schnapps/core/view_dialog_list.h>
#include <schnapps/core/view_button_area.h>

#include <cgogn/rendering/drawer.h>

#include <QOGLViewer/qoglviewer.h>
#include <QOGLViewer/manipulatedFrame.h>

namespace schnapps
{

class SCHNApps;
class Camera;
class Plugin;
class PluginInteraction;
class Object;

/**
* @brief View class inherit from QOGLViewer (http://libqglviewer.com/refManual/classQGLViewer.html)
* It can be linked with:
* - several plugins
* - several maps
* - one camera
* One view of SCHNApps is selected (green framed).
*
* Python callable slots are tagged with [PYTHON]
*/
class SCHNAPPS_CORE_EXPORT View : public QOGLViewer
{
	Q_OBJECT

public:

	static uint32 view_count_;

	View(const QString& name, SCHNApps* s);

	~View() override;

	inline const QString& name() const { return name_; }

public slots:

	/**
	 * @brief get the name of view
	 * @return name
	 */
	inline QString name() { return name_; }

	/**
	 * @brief get the schnapps objet ptr
	 * @return the ptr
	 */
	inline SCHNApps* schnapps() const { return schnapps_; }

	/**
	 * @brief test if the view is the selected one
	 * @return
	 */
	bool is_selected_view() const;

	/*********************************************************
	 * MANAGE LINKED CAMERA
	 *********************************************************/

	/**
	 * @brief set the current camera of the view
	 * @param c the camera ptr
	 */
	void set_current_camera(Camera* c);

	/**
	* @brief set the current camera of the view
	* @param name the name of camera
	*/
	void set_current_camera(const QString& name);

	/**
	* @brief get the current camera of the view
	* @return the camera object
	*/
	inline Camera* current_camera() const { return current_camera_; }

	/**
	 * @brief test if this view use a camera
	 * @param c camera ptr
	 * @return
	 */
	inline bool uses_camera(Camera* c) const { return current_camera_ == c; }

	/**
	* @brief test if a camera is the current camera
	* @param name the name of camera
	*/
	bool uses_camera(const QString& name) const;

	/*********************************************************
	 * MANAGE LINKED PLUGINS
	 *********************************************************/

	void link_plugin(PluginInteraction* plugin, bool update_dialog_list = true);
	void link_plugin(const QString& name, bool update_dialog_list = true);

	void unlink_plugin(PluginInteraction* plugin, bool update_dialog_list = true);
	void unlink_plugin(const QString& name, bool update_dialog_list = true);

	inline const std::list<PluginInteraction*>& linked_plugins() const { return plugins_; }

	inline bool is_linked_to_plugin(PluginInteraction* plugin) const
	{
		return std::find(plugins_.begin(), plugins_.end(), plugin) != plugins_.end();
	}

	/**
	* @brief test if the view is linked to a plugin
	* @param name the name of plugin
	*/
	bool is_linked_to_plugin(const QString& name) const;

	/*********************************************************
	 * MANAGE LINKED OBJECTS
	 *********************************************************/

	void link_object(Object* o, bool update_dialog_list = true);
	void link_object(const QString& name, bool update_dialog_list = true);

	void unlink_object(Object* o, bool update_dialog_list = true);
	void unlink_object(const QString& name, bool update_dialog_list = true);

	inline const std::list<Object*>& linked_objects() const { return objects_; }

	inline bool is_linked_to_object(Object* o) const
	{
		return std::find(objects_.begin(), objects_.end(), o) != objects_.end();
	}

private:

	virtual void init() override;
	virtual void preDraw() override;
	virtual void draw() override;
	virtual void postDraw() override;
	virtual void resizeGL(int width, int height) override;

	void draw_buttons();
	void draw_frame();

	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

	inline int pixel_ratio() const
	{
		#if (QT_VERSION >> 16) == 5
			return this->devicePixelRatio();
		#else
			return 1;
		#endif
	}

public:

	/**
	 * @brief get the min and max points of the bounding box of the scene
	 * @param bb_min min to be filled
	 * @param bb_max max to be filled
	 */
	inline void bb(qoglviewer::Vec& bb_min, qoglviewer::Vec& bb_max) const
	{
		bb_min = bb_min_;
		bb_max = bb_max_;
	}

	// hide all dialogs of the view
	void hide_dialogs();

private slots:

	void close_dialogs();

	void object_added(Object* o);
	void object_removed(Object* o);
	void object_check_state_changed(QListWidgetItem* item);

	void plugin_enabled(Plugin *plugin);
	void plugin_disabled(Plugin *plugin);
	void plugin_check_state_changed(QListWidgetItem* item);

	void camera_added(Camera* camera);
	void camera_removed(Camera* camera);
	void camera_check_state_changed(QListWidgetItem* item);

public slots:

	void update_bb();

private slots:

	void color_selected();

	void ui_vertical_split_view(int x, int y, int globalX, int globalY);
	void ui_horizontal_split_view(int x, int y, int globalX, int globalY);
	void ui_close_view(int x, int y, int globalX, int globalY);
	void ui_color_view(int x, int y, int globalX, int globalY);

	void ui_objects_list_view(int x, int y, int globalX, int globalY);
	void ui_plugins_list_view(int x, int y, int globalX, int globalY);
	void ui_cameras_list_view(int x, int y, int globalX, int globalY);

signals:

	void current_camera_changed(Camera*, Camera*);

	void object_linked(Object*);
	void object_unlinked(Object*);

	void plugin_linked(PluginInteraction*);
	void plugin_unlinked(PluginInteraction*);

	void bb_changed();

protected:

	QString name_;
	SCHNApps* schnapps_;

	QColorDialog* color_dial_;
	QColor background_color_;

	Camera* current_camera_;
	std::list<PluginInteraction*> plugins_;
	std::list<Object*> objects_;

	qoglviewer::Vec bb_min_;
	qoglviewer::Vec bb_max_;

	ViewButtonArea* button_area_;

	ViewButton* close_button_;
	ViewButton* color_button_;
	ViewButton* Vsplit_button_;
	ViewButton* Hsplit_button_;

	ViewButtonArea* button_area_left_;

	ViewButton* objects_button_;
	ViewButton* plugins_button_;
	ViewButton* cameras_button_;

	QString text_info_;

	ViewDialogList* dialog_objects_;
	ViewDialogList* dialog_plugins_;
	ViewDialogList* dialog_cameras_;

	std::unique_ptr<cgogn::rendering::DisplayListDrawer> frame_drawer_;
	std::unique_ptr<cgogn::rendering::DisplayListDrawer::Renderer> frame_drawer_renderer_;

	bool save_snapshots_;

	bool updating_ui_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_VIEW_H_
