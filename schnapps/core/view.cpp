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

#include <core/view.h>
#include <core/camera.h>

//#include <core/map_handler.h>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>

namespace schnapps
{

unsigned int View::view_count_ = 0;

View::View(const QString& name, SCHNApps* s) :
	name_(name),
	schnapps_(s),
	current_camera_(NULL),
	bb_min_(0.0, 0.0, 0.0),
	bb_max_(0.0, 0.0, 0.0),
	button_area_(NULL),
	close_button_(NULL),
	Vsplit_button_(NULL),
	Hsplit_button_(NULL),
	button_area_left_(NULL),
	maps_button_(NULL),
	plugins_button_(NULL),
	cameras_button_(NULL),
	dialog_maps_(NULL),
	dialog_plugins_(NULL),
	dialog_cameras_(NULL),
	save_snapshots_(false),
	updating_ui_(false)
{
	++view_count_;

	this->setSnapshotFormat("BMP");
	this->setSnapshotFileName(name_);
	this->setSnapshotQuality(100);

	dialog_maps_ = new ListPopUp("Linked Maps");
	dialog_plugins_ = new ListPopUp("Linked Plugins");
	dialog_cameras_ = new ListPopUp("Cameras");

//	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*,MapHandlerGen*)));

//	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
//	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
//	connect(dialog_maps_->list(), SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(map_check_state_changed(QListWidgetItem*)));

//	foreach(MapHandlerGen* map, schnapps_->get_map_set().values())
//		map_added(map);

//	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(plugin_enabled(Plugin*)));
//	connect(schnapps_, SIGNAL(plugin_disabled(Plugin*)), this, SLOT(plugin_disabled(Plugin*)));
//	connect(dialog_plugins_->list(), SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(plugin_check_state_changed(QListWidgetItem*)));

//	foreach(Plugin* plugin, schnapps_->get_plugin_set().values())
//		plugin_enabled(plugin);

//	connect(schnapps_, SIGNAL(camera_added(Camera*)), this, SLOT(camera_added(Camera*)));
//	connect(schnapps_, SIGNAL(camera_removed(Camera*)), this, SLOT(camera_removed(Camera*)));
//	connect(dialog_cameras_->list(), SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(camera_check_state_changed(QListWidgetItem*)));

//	foreach(Camera* camera, schnapps_->get_camera_set().values())
//		cameraAdded(camera);

	current_camera_ = schnapps_->add_camera();
	current_camera_->link_view(this);
	dialog_cameras_->check(current_camera_->get_name(), Qt::Checked);

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(close_dialogs()));
}

View::~View()
{
	qoglviewer::Camera* c = new qoglviewer::Camera();
	this->set_camera(c);
	current_camera_->unlink_view(this);

//	foreach(PluginInteraction* p, plugins_)
//		unlink_plugin(p);

//	foreach(MapHandlerGen* m, maps_)
//		unlink_map(m);

//	delete button_area_;
//	delete button_area_left_;

//	delete dialog_maps_;
//	delete dialog_plugins_;
//	delete dialog_cameras_;
}

/*********************************************************
 * MANAGE LINKED CAMERA
 *********************************************************/

void View::set_current_camera(Camera* c)
{
	if (c != current_camera_ && c)
	{
		Camera* prev = current_camera_;
		if (prev)
			prev->unlink_view(this);

		current_camera_ = c;
		this->setCamera(current_camera_);
		current_camera_->link_view(this);

		emit(current_camera_changed(prev, c));

		if (prev)
		{
			QListWidgetItem* prev_item = dialog_cameras_->findItem(prev->get_name());
			if(prev_item)
			{
				updating_ui_ = true;
				prev_item->setCheckState(Qt::Unchecked);
				updating_ui_ = false;
			}
		}

		if (current_camera_)
		{
			QListWidgetItem* cur_item = dialog_cameras_->findItem(current_camera_->get_name());
			if(curItem)
			{
				updating_ui_ = true;
				cur_item->setCheckState(Qt::Checked);
				updating_ui_ = false;
			}
		}

		current_camera_->fit_to_views_bounding_box();
		this->updateGL();
	}
}

void View::set_current_camera(const QString& name)
{	
	Camera* c = schnapps_->get_camera(name);
	if (c)
		set_current_camera(c);
}

bool View::usesCamera(const QString& name) const
{
	Camera* c = schnapps_->get_camera(name);
	return uses_camera(c);
}

/*********************************************************
 * MANAGE LINKED PLUGINS
 *********************************************************/

//void View::link_plugin(PluginInteraction* plugin)
//{
//	if(plugin && !plugins_.contains(plugin))
//	{
//		plugins_.push_back(plugin);
//		plugin->link_view(this);

//		emit(plugin_linked(plugin));

//		updating_ui_ = true;
//		dialog_plugins_->check(plugin->get_name(), Qt::Checked);
//		updating_ui_ = false;

//		this->updateGL();
//	}
//}

//void View::link_plugin(const QString& name)
//{
//	PluginInteraction* p = dynamic_cast<PluginInteraction*>(schnapps_->get_plugin(name));
//	if (p)
//		link_plugin(p);
//}

//void View::unlink_plugin(PluginInteraction* plugin)
//{
//	if(plugins_.removeOne(plugin))
//	{
//		plugin->unlink_view(this);

//		emit(plugin_unlinked(plugin));

//		updating_ui_ = true;
//		dialog_plugins_->check(plugin->get_name(), Qt::Unchecked);
//		updating_ui_ = false;

//		this->updateGL();
//	}
//}

//void View::unlink_plugin(const QString& name)
//{
//	PluginInteraction* p = dynamic_cast<PluginInteraction*>(schnapps_->get_plugin(name));
//	if (p)
//		unlink_plugin(p);
//}

//bool View::is_linked_to_plugin(const QString& name) const
//{
//	PluginInteraction* p = dynamic_cast<PluginInteraction*>(schnapps_->get_plugin(name));
//	return plugins_.contains(p);
//}

/*********************************************************
 * MANAGE LINKED MAPS
 *********************************************************/

//void View::link_map(MapHandlerGen* map)
//{
//	if(map && !maps_.contains(map))
//	{
//		maps_.push_back(map);
//		map->link_view(this);

//		emit(map_linked(map));

//		connect(map, SIGNAL(selected_cells_changed(CellSelectorGen*)), this, SLOT(updateGL()));
//		connect(map, SIGNAL(bounding_box_modified()), this, SLOT(update_bounding_box()));

//		if(map->is_selected_map())
//			this->setManipulatedFrame(map->getFrame());

//		update_bounding_box();

//		updating_ui_ = true;
//		dialog_maps_->check(map->get_name(), Qt::Checked);
//		updating_ui_ = false;
//	}
//}

//void View::link_map(const QString& name)
//{
//	MapHandlerGen* m = schnapps_->get_map(name);
//	if (m)
//		link_map(m);
//}

//void View::unlink_map(MapHandlerGen* map)
//{
//	if(maps_.removeOne(map))
//	{
//		map->unlink_view(this);

//		emit(map_unlinked(map));

//		disconnect(map, SIGNAL(selected_cells_changed(CellSelectorGen*)), this, SLOT(updateGL()));
//		disconnect(map, SIGNAL(bounding_box_modified()), this, SLOT(update_bounding_box()));

//		if(map->is_selected_map())
//			this->setManipulatedFrame(NULL);

//		update_bounding_box();

//		updating_ui_ = true;
//		dialog_maps_->check(map->get_name(), Qt::Unchecked);
//		updating_ui_ = false;
//	}
//}

//void View::unlink_map(const QString& name)
//{
//	MapHandlerGen* m = schnapps_->get_map(name);
//	if (m)
//		unlink_map(m);
//}

//bool View::is_linked_to_map(const QString& name) const
//{
//	MapHandlerGen* m = schnapps_->get_map(name);
//	return maps_.contains(m);
//}




void View::init()
{
	qoglviewer::Camera* c = this->camera();
	this->setCamera(current_camera_);
//	delete c;

	this->setBackgroundColor(QColor(0, 0, 0));
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

//	button_area_ = new ViewButtonArea(this);
//	button_area_->set_top_right_position(this->width(), 0);

//	Vsplit_button_ = new ViewButton(":icons/icons/Vsplit.png", this);
//	button_area_->addButton(Vsplit_button_);
//	connect(Vsplit_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_vertical_split_view(int, int, int, int)));

//	Hsplit_button_ = new ViewButton(":icons/icons/Hsplit.png", this);
//	button_area_->addButton(Hsplit_button_);
//	connect(Hsplit_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_horizontal_split_view(int, int, int, int)));

//	close_button_ = new ViewButton(":icons/icons/close.png", this);
//	button_area_->addButton(close_button_);
//	connect(close_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_close_view(int, int, int, int)));

//	button_area_left_ = new ViewButtonArea(this);
//	button_area_left_->set_top_left_position(0, 0);

//	maps_button_ = new ViewButton(":icons/icons/maps.png", this);
//	button_area_left_->addButton(maps_button_);
//	connect(maps_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_maps_list_view(int, int, int, int)));

//	plugins_button_ = new ViewButton(":icons/icons/plugins.png", this);
//	button_area_left_->addButton(plugins_button_);
//	connect(plugins_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_plugins_list_view(int, int, int, int)));

//	cameras_button_ = new ViewButton(":icons/icons/cameras.png", this);
//	button_area_left_->addButton(cameras_button_);
//	connect(cameras_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_cameras_list_view(int, int, int, int)));
}

void View::preDraw()
{
	this->makeCurrent();
	current_camera_->setScreenWidthAndHeight(width(), height());

	QGLViewer::preDraw();
}

void View::draw()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	const QMap<QString, Camera*>& cameras = schnapps_->get_camera_set();
	QList<Camera*> lc = cameras.values();
	foreach (Camera* camera, lc)
	{
		if (camera != current_camera_)
		{
			if (camera->get_draw()) camera->draw();
			if (camera->get_draw_path()) camera->drawAllPaths();
		}
	}

//	glm::mat4 mm = get_current_model_view_matrix();
//	glm::mat4 pm = get_current_projection_matrix();

//	MapHandlerGen* selected_map = schnapps_->get_selected_map();

//	foreach(MapHandlerGen* map, maps_)
//	{
//		glm::mat4 map_mm = mm * map->get_frame_matrix() * map->get_transfo_matrix();

//		if(map == selected_map)
//		{
//			Utils::Drawer* bb_drawer = map->get_bb_drawer();
//			if (bb_drawer)
//				bb_drawer->update_matrices(pm, map_mm);
//			map->draw_bb();
//		}

//		foreach(PluginInteraction* plugin, plugins_)
//		{
//			foreach(Utils::GLSLShader* shader, plugin->get_shaders())
//				shader->update_matrices(pm, map_mm);
//			plugin->draw_map(this, map);
//		}
//	}

//	foreach(PluginInteraction* plugin, plugins_)
//		plugin->draw(this);
}

void View::postDraw()
{
	if (is_selected_view())
		draw_frame();
	draw_buttons();

	QGLViewer::postDraw();
}

void View::resizeGL(int width, int height)
{
	QGLViewer::resizeGL(width, height);

//	if(button_area_)
//		button_area_->set_top_right_position(width / this->pixelRatio(), 0);

//	if(button_area_left_)
//		button_area_left_->set_top_left_position(0, 0);
}

void View::draw_buttons()
{
//	button_area_->draw();
//	button_area_left_->draw();
}

void View::draw_frame()
{
//	glDisable(GL_DEPTH_TEST);
//	frame_drawer->callList();
//	glEnable(GL_DEPTH_TEST);
}

void View::keyPressEvent(QKeyEvent* event)
{
	quint64 k = event->modifiers();
	k <<= 32;
	k |= event->key();

	switch (event->key())
	{
		case Qt::Key_S:
		{
			save_snapshots_ = !save_snapshots_;

			if (save_snapshots_)
			{
				QMessageBox msg_box;
				msg_box.setText("Snapshot every frame?");
				msg_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
				msg_box.setDefaultButton(QMessageBox::Ok);
				if (msg_box.exec() == QMessageBox::Ok)
				{
					m_schnapps->status_bar_message("frame snapshot !!", 2000);
					connect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));
				}
				else
				{
					m_schnapps->status_bar_message("cancel frame snapshot", 2000);
					save_snapshots_ = false;
				}
			}
			else
			{
				disconnect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));
				schnapps_->status_bar_message("Stop frame snapshot", 2000);
			}

		}
		break;

		default:
		{
//			foreach(PluginInteraction* plugin, plugins_)
//				plugin->keyPress(this, event);

			if (event->key() == Qt::Key_Escape)
			{
				QMessageBox msg_box;
				msg_box.setText("Really quit SCHNApps ?");
				msg_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
				msg_box.setDefaultButton(QMessageBox::Ok);
				if (msg_box.exec() == QMessageBox::Ok)
					schnapps_->close();
			}
			else
				QGLViewer::keyPressEvent(event);
		}
	}
}

void View::keyReleaseEvent(QKeyEvent *event)
{
//	foreach(PluginInteraction* plugin, plugins_)
//		plugin->keyRelease(this, event);

	QGLViewer::keyReleaseEvent(event);
}

void View::mousePressEvent(QMouseEvent* event)
{
	if (!is_selected_view())
	{
		schnapps_->set_selected_view(this);
		schnapps_->status_bar_message(QString("Selecting ") + this->get_name(), 2000);
	}
	else if (event->y() < 20)
		schnapps_->status_bar_message(this->get_name(), 2000);

//	if (button_area_left_->is_clicked(event->x(), event->y()))
//		button_area_left_->click_button(event->x(), event->y(), event->globalX(), event->globalY());
//	else
//	{
//		hide_dialogs();
//		if (button_area_->is_clicked(event->x(), event->y()))
//			button_area_->click_button(event->x(), event->y(), event->globalX(), event->globalY());
//		else
//		{
//			foreach(PluginInteraction* plugin, plugins_)
//				plugin->mousePress(this, event);

//			QGLViewer::mousePressEvent(event);
//		}
//	}
}

void View::mouseReleaseEvent(QMouseEvent* event)
{
//	foreach(PluginInteraction* plugin, plugins_)
//		plugin->mouseRelease(this, event);

	QGLViewer::mouseReleaseEvent(event);
}

void View::mouseMoveEvent(QMouseEvent* event)
{
//	foreach(PluginInteraction* plugin, plugins_)
//		plugin->mouseMove(this, event);

	QGLViewer::mouseMoveEvent(event);
}

void View::wheelEvent(QWheelEvent* event)
{
//	foreach(PluginInteraction* plugin, plugins_)
//		plugin->wheelEvent(this, event);

	QGLViewer::wheelEvent(event);
}

//glm::mat4 View::get_current_model_view_matrix() const
//{
//	GLdouble gl_mvm[16];
//	camera()->getModelViewMatrix(gl_mvm);
//	glm::mat4 mvm;
//	for(unsigned int i = 0; i < 4; ++i)
//	{
//		for(unsigned int j = 0; j < 4; ++j)
//			mvm[i][j] = (float)gl_mvm[i*4+j];
//	}
//	return mvm;
//}

//glm::mat4 View::get_current_projection_matrix() const
//{
//	GLdouble gl_pm[16];
//	camera()->getProjectionMatrix(gl_pm);
//	glm::mat4 pm;
//	for(unsigned int i = 0; i < 4; ++i)
//	{
//		for(unsigned int j = 0; j < 4; ++j)
//			pm[i][j] = (float)gl_pm[i*4+j];
//	}
//	return pm;
//}

//glm::mat4 View::get_current_model_view_projection_matrix() const
//{
//	GLdouble gl_mvpm[16];
//	camera()->getModelViewProjectionMatrix(gl_mvpm);
//	glm::mat4 mvpm;
//	for(unsigned int i = 0; i < 4; ++i)
//	{
//		for(unsigned int j = 0; j < 4; ++j)
//			mvpm[i][j] = (float)gl_mvpm[i*4+j];
//	}
//	return mvpm;
//}

void View::close_dialogs()
{
//	dialog_maps_->close();
//	dialog_plugins_->close();
//	dialog_cameras_->close();
}

void View::hide_dialogs()
{
//	if (dialog_maps_->isVisible())
//		dialog_maps_->hide();
//	if (dialog_plugins_->isVisible())
//		dialog_plugins_->hide();
//	if (dialog_cameras_->isVisible())
//		dialog_cameras_->hide();
}

//void View::selected_map_changed(MapHandlerGen* prev, MapHandlerGen* cur)
//{
//	if(cur && is_linked_to_map(cur))
//		this->setManipulatedFrame(cur->getFrame());
//	this->updateGL();
//}

//void View::ui_vertical_split_view(int x, int y, int /*globalX*/, int /*globalY*/)
//{
//	schnapps_->split_view(m_name, Qt::Horizontal);
//}

//void View::ui_horizontal_split_view(int x, int y, int /*globalX*/, int /*globalY*/)
//{
//	schnapps_->split_view(m_name, Qt::Vertical);
//}

//void View::ui_close_view(int x, int y, int /*globalX*/, int /*globalY*/)
//{
//	schnapps_->remove_view(m_name);
//}

//void View::ui_maps_list_view(int x, int y, int globalX, int globalY)
//{
//	if (dialog_maps_->isHidden())
//	{
//		dialog_maps_->show();
//		dialog_maps_->move(QPoint(globalX, globalY + 8));
//		dialog_cameras_->hide();
//		dialog_plugins_->hide();
//	}
//	else
//		dialog_maps_->hide();
//}

//void View::ui_pluginsListView(int x, int y, int globalX, int globalY)
//{
//	if (m_dialogPlugins->isHidden())
//	{
//		dialog_plugins_->show();
//		dialog_plugins_->move(QPoint(globalX, globalY + 8));
//		dialog_maps_->hide();
//		dialog_cameras_->hide();
//	}
//	else
//		dialog_plugins_->hide();
//}

//void View::ui_camerasListView(int x, int y, int globalX, int globalY)
//{
//	if (m_dialogCameras->isHidden())
//	{
//		dialog_cameras_->show();
//		dialog_cameras_->move(QPoint(globalX, globalY + 8));
//		dialog_plugins_->hide();
//		dialog_maps_->hide();
//	}
//	else
//		dialog_cameras_->hide();
//}

//void View::map_added(MapHandlerGen* mh)
//{
//	if (mh)
//		dialog_maps_->add_item(mh->get_name());
//}

//void View::map_removed(MapHandlerGen* mh)
//{
//	if (mh)
//		m_dialogMaps->remove_item(mh->get_name());
//}

//void View::map_check_state_changed(QListWidgetItem* item)
//{
//	if (!updating_ui_)
//	{
//		if (item->checkState() == Qt::Checked)
//			link_map(item->text());
//		else
//			unlink_map(item->text());
//	}
//}

//void View::plugin_enabled(Plugin *plugin)
//{
//	if (dynamic_cast<PluginInteraction*>(plugin))
//		dialog_plugins_->add_item(plugin->get_name());
//}

//void View::plugin_disabled(Plugin *plugin)
//{
//	if (dynamic_cast<PluginInteraction*>(plugin))
//		dialog_plugins_->remove_item(plugin->get_name());
//}

//void View::plugin_check_state_changed(QListWidgetItem* item)
//{
//	if (!updating_ui_)
//	{
//		if (item->checkState() == Qt::Checked)
//			link_plugin(item->text());
//		else
//			unlink_plugin(item->text());
//	}
//}

//void View::camera_added(Camera* camera)
//{
//	if (camera)
//		dialog_cameras_->add_item(camera->get_name());
//}

//void View::camera_removed(Camera* camera)
//{
//	if (camera)
//		dialog_cameras_->remove_item(camera->get_name());
//}

//void View::camera_check_state_changed(QListWidgetItem* item)
//{
//	if (!updating_ui_)
//	{
//		if (item->checkState() == Qt::Checked)
//			set_current_camera(item->text());
//	}
//}

//void View::update_bounding_box()
//{
//	if (!maps_.empty())
//	{
//		bool initialized = false;

//		foreach (MapHandlerGen* mhg, maps_)
//		{
//			qoglviewer::Vec minbb;
//			qoglviewer::Vec maxbb;
//			if (mhg->transformed_bb(minbb, maxbb))
//			{
//				if (initialized)
//				{
//					for (unsigned int dim = 0; dim < 3; ++dim)
//					{
//						if (minbb[dim] < bb_min_[dim])
//							bb_min_[dim] = minbb[dim];
//						if (maxbb[dim] > bb_max_[dim])
//							bb_max_[dim] = maxbb[dim];
//					}
//				}
//				else
//				{
//					for (unsigned int dim = 0; dim < 3; ++dim)
//					{
//						bb_min_[dim] = minbb[dim];
//						bb_max_[dim] = maxbb[dim];
//					}
//					initialized = true;
//				}
//			}
//		}

//		if (!initialized)
//		{
//			bb_min_.setValue(0, 0, 0);
//			bb_max_.setValue(0, 0, 0);
//		}
//	}
//	else
//	{
//		bb_min_.setValue(0, 0, 0);
//		bb_max_.setValue(0, 0, 0);
//	}

//	emit(bounding_box_changed());
//}

} // namespace schnapps
