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

#include <schnapps/core/view.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/camera.h>
#include <schnapps/core/object.h>
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/plugin_provider.h>

#include <QMatrix4x4>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>
#include <QListWidgetItem>

namespace schnapps
{

uint32 View::view_count_ = 0;

View::View(const QString& name, SCHNApps* s) :
	name_(name),
	schnapps_(s),
	background_color_(0, 0, 0, 255),
	current_camera_(nullptr),
	bb_min_(0., 0., 0.),
	bb_max_(0., 0., 0.),
	button_area_(nullptr),
	close_button_(nullptr),
	Vsplit_button_(nullptr),
	Hsplit_button_(nullptr),
	button_area_left_(nullptr),
	objects_button_(nullptr),
	plugins_button_(nullptr),
	cameras_button_(nullptr),
	dialog_objects_(nullptr),
	dialog_plugins_(nullptr),
	dialog_cameras_(nullptr),
	frame_drawer_(nullptr),
	frame_drawer_renderer_(nullptr),
	save_snapshots_(false),
	updating_ui_(false)
{
	++view_count_;

	this->setAutoFillBackground(true);

	this->setSnapshotFormat("BMP");
	this->setSnapshotFileName(name_);
	this->setSnapshotQuality(100);

	dialog_objects_ = new ViewDialogList("Linked Objects");
	dialog_plugins_ = new ViewDialogList("Linked Plugins");
	dialog_cameras_ = new ViewDialogList("Cameras");

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(object_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(object_removed(Object*)));
	connect(dialog_objects_->list(), SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(object_check_state_changed(QListWidgetItem*)));

	schnapps_->foreach_object([this] (Object* o) { object_added(o); });

	connect(schnapps_, SIGNAL(plugin_enabled(Plugin*)), this, SLOT(plugin_enabled(Plugin*)));
	connect(schnapps_, SIGNAL(plugin_disabled(Plugin*)), this, SLOT(plugin_disabled(Plugin*)));
	connect(dialog_plugins_->list(), SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(plugin_check_state_changed(QListWidgetItem*)));

	schnapps_->foreach_plugin([this] (Plugin* plugin) { plugin_enabled(plugin); });

	connect(schnapps_, SIGNAL(camera_added(Camera*)), this, SLOT(camera_added(Camera*)));
	connect(schnapps_, SIGNAL(camera_removed(Camera*)), this, SLOT(camera_removed(Camera*)));
	connect(dialog_cameras_->list(), SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(camera_check_state_changed(QListWidgetItem*)));

	schnapps_->foreach_camera([this] (Camera* camera) { camera_added(camera); });

	current_camera_ = schnapps_->add_camera();
	current_camera_->link_view(this);
	dialog_cameras_->check(current_camera_->name(), Qt::Checked);

	connect(schnapps_, SIGNAL(schnapps_closing()), this, SLOT(close_dialogs()));

	color_dial_ = new QColorDialog(background_color_, nullptr);
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));
}

View::~View()
{
	qoglviewer::Camera* c = new qoglviewer::Camera();
	this->setCamera(c);
	current_camera_->unlink_view(this);

	while(!plugins_.empty())
		unlink_plugin(*plugins_.begin());

	while(!objects_.empty())
		unlink_object(*objects_.begin());

	delete button_area_;
	delete button_area_left_;

	delete dialog_objects_;
	delete dialog_plugins_;
	delete dialog_cameras_;
}

bool View::is_selected_view() const
{
	return schnapps_->selected_view() == this;
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
			QListWidgetItem* prev_item = dialog_cameras_->find_item(prev->name());
			if (prev_item)
			{
				updating_ui_ = true;
				prev_item->setCheckState(Qt::Unchecked);
				updating_ui_ = false;
			}
		}

		if (current_camera_)
		{
			QListWidgetItem* cur_item = dialog_cameras_->find_item(current_camera_->name());
			if (cur_item)
			{
				updating_ui_ = true;
				cur_item->setCheckState(Qt::Checked);
				updating_ui_ = false;
			}
		}

		this->update();
	}
}

void View::set_current_camera(const QString& name)
{
	Camera* c = schnapps_->get_camera(name);
	if (c)
		set_current_camera(c);
}

bool View::uses_camera(const QString& name) const
{
	Camera* c = schnapps_->get_camera(name);
	return uses_camera(c);
}

/*********************************************************
 * MANAGE LINKED PLUGINS
 *********************************************************/

void View::link_plugin(PluginInteraction* plugin, bool update_dialog_list)
{
	if (plugin && !is_linked_to_plugin(plugin))
	{
		plugins_.push_back(plugin);
		plugin->link_view(this);

		emit(plugin_linked(plugin));

		if (update_dialog_list)
		{
			updating_ui_ = true;
			if (!plugin->auto_activate())
				dialog_plugins_->check(plugin->name(), Qt::Checked);
			updating_ui_ = false;
		}

		this->update();
	}
}

void View::link_plugin(const QString& name, bool update_dialog_list)
{
	PluginInteraction* p = dynamic_cast<PluginInteraction*>(schnapps_->plugin(name));
	if (p)
		link_plugin(p, update_dialog_list);
}

void View::unlink_plugin(PluginInteraction* plugin, bool update_dialog_list)
{
	if (is_linked_to_plugin(plugin))
	{
		plugins_.remove(plugin);
		plugin->unlink_view(this);

		emit(plugin_unlinked(plugin));

		if (update_dialog_list)
		{
			updating_ui_ = true;
			if (!plugin->auto_activate())
				dialog_plugins_->check(plugin->name(), Qt::Unchecked);
			updating_ui_ = false;
		}

		this->update();
	}
}

void View::unlink_plugin(const QString& name, bool update_dialog_list)
{
	PluginInteraction* p = dynamic_cast<PluginInteraction*>(schnapps_->plugin(name));
	if (p)
		unlink_plugin(p, update_dialog_list);
}

bool View::is_linked_to_plugin(const QString& name) const
{
	PluginInteraction* p = dynamic_cast<PluginInteraction*>(schnapps_->plugin(name));
	return is_linked_to_plugin(p);
}

/*********************************************************
 * MANAGE LINKED MAPS
 *********************************************************/

void View::link_object(Object* o, bool update_dialog_list)
{
	if (o && !is_linked_to_object(o))
	{
		objects_.push_back(o);
		o->link_view(this);

		emit(object_linked(o));

		connect(o, SIGNAL(bb_changed()), this, SLOT(update_bb()));

//		if (map->is_selected_map())
//			this->setManipulatedFrame(&map->get_frame());

		update_bb();

		if (update_dialog_list)
		{
			updating_ui_ = true;
			dialog_objects_->check(o->provider()->name() + "/" + o->name(), Qt::Checked);
			updating_ui_ = false;
		}

		this->update();
	}
}

void View::link_object(const QString& name, bool update_dialog_list)
{
	QStringList names = name.split('/');
	PluginProvider* provider = dynamic_cast<PluginProvider*>(schnapps_->plugin(names.at(0)));
	if (provider)
	{
		Object* o = provider->object(names.at(1));
		if (o)
			link_object(o, update_dialog_list);
	}
}

void View::unlink_object(Object* o, bool update_dialog_list)
{
	if (is_linked_to_object(o))
	{
		objects_.remove(o);
		o->unlink_view(this);

		emit(object_unlinked(o));

		disconnect(o, SIGNAL(bb_changed()), this, SLOT(update_bb()));

//		if (map->is_selected_map())
//			this->setManipulatedFrame(nullptr);

		update_bb();

		if (update_dialog_list)
		{
			updating_ui_ = true;
			dialog_objects_->check(o->provider()->name() + "/" + o->name(), Qt::Unchecked);
			updating_ui_ = false;
		}

		this->update();
	}
}

void View::unlink_object(const QString& name, bool update_dialog_list)
{
	QStringList names = name.split('/');
	PluginProvider* provider = dynamic_cast<PluginProvider*>(schnapps_->plugin(names.at(0)));
	if (provider)
	{
		Object* o = provider->object(names.at(1));
		if (o)
			unlink_object(o, update_dialog_list);
	}
}





void View::init()
{
	this->makeCurrent();

//	qoglviewer::Camera* c = this->camera();
	this->setCamera(current_camera_);
//	delete c;

	frame_drawer_ = cgogn::make_unique<cgogn::rendering::DisplayListDrawer>();
	frame_drawer_renderer_ = frame_drawer_->generate_renderer();

	frame_drawer_->new_list();
	frame_drawer_->color3f(0.0f,1.0f,0.0f);
	frame_drawer_->line_width(4.0f);
	frame_drawer_->begin(GL_LINE_LOOP);
		frame_drawer_->vertex3f(-1.0f,-1.0f, -1.0f);
		frame_drawer_->vertex3f( 1.0f,-1.0f, 0.0f);
		frame_drawer_->vertex3f( 1.0f, 1.0f, 0.0f);
		frame_drawer_->vertex3f(-1.0f, 1.0f, 0.0f);
	frame_drawer_->end();
	frame_drawer_->end_list();

	button_area_ = new ViewButtonArea(this);
	button_area_->set_top_right_position(this->width(), 0);

	color_button_ = new ViewButton(":icons/icons/color.png", this);
	button_area_->add_button(color_button_);
	connect(color_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_color_view(int, int, int, int)));

	Vsplit_button_ = new ViewButton(":icons/icons/Vsplit.png", this);
	button_area_->add_button(Vsplit_button_);
	connect(Vsplit_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_vertical_split_view(int, int, int, int)));

	Hsplit_button_ = new ViewButton(":icons/icons/Hsplit.png", this);
	button_area_->add_button(Hsplit_button_);
	connect(Hsplit_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_horizontal_split_view(int, int, int, int)));

	close_button_ = new ViewButton(":icons/icons/close.png", this);
	button_area_->add_button(close_button_);
	connect(close_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_close_view(int, int, int, int)));

	button_area_left_ = new ViewButtonArea(this);
	button_area_left_->set_top_left_position(0, 0);

	objects_button_ = new ViewButton(":icons/icons/maps.png", this);
	button_area_left_->add_button(objects_button_);
	connect(objects_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_objects_list_view(int, int, int, int)));

	plugins_button_ = new ViewButton(":icons/icons/plugins.png", this);
	button_area_left_->add_button(plugins_button_);
	connect(plugins_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_plugins_list_view(int, int, int, int)));

	cameras_button_ = new ViewButton(":icons/icons/cameras.png", this);
	button_area_left_->add_button(cameras_button_);
	connect(cameras_button_, SIGNAL(clicked(int, int, int, int)), this, SLOT(ui_cameras_list_view(int, int, int, int)));

	QOGLViewer::init(); // emit viewerInitialized signal
}

void View::preDraw()
{
	this->makeCurrent();
	current_camera_->setScreenWidthAndHeight(width(), height());

	QOGLViewer::preDraw();
}

void View::draw()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	schnapps_->foreach_camera([this] (Camera* camera)
	{
		if (camera != current_camera_)
		{
			// TODO recode this in QOGLViewer
//			if (camera->get_draw()) camera->draw();
//			if (camera->get_draw_path()) camera->drawAllPaths();
		}
	});

	draw_buttons();

	QMatrix4x4 pm;
	current_camera_->getProjectionMatrix(pm);
	QMatrix4x4 mm;
	current_camera_->getModelViewMatrix(mm);

	for (Object* o : objects_)
	{
		QMatrix4x4 o_mm = mm * o->frame_matrix() * o->transformation_matrix();

//		if (o->show_bb())
			o->draw_bb(this, pm, o_mm);

		for (PluginInteraction* plugin : plugins_)
			plugin->draw_object(this, o, pm, o_mm);
	}

	for (PluginInteraction* plugin : plugins_)
		plugin->draw(this, pm, mm);
}

void View::postDraw()
{
//	draw_buttons();
	if (is_selected_view())
		draw_frame();

	QOGLViewer::postDraw();
}

void View::resizeGL(int width, int height)
{
	for (PluginInteraction* plugin : plugins_)
		plugin->resizeGL(this, width, height);

	QOGLViewer::resizeGL(width, height);

	if (button_area_)
		button_area_->set_top_right_position(width, 0);

	if (button_area_left_)
		button_area_left_->set_top_left_position(0, 0);
}

void View::draw_buttons()
{
	button_area_->draw();
	button_area_left_->draw();
}

void View::draw_frame()
{
	glDisable(GL_DEPTH_TEST);
	QMatrix4x4 pm, mm;
	frame_drawer_renderer_->draw(pm, mm);
	glEnable(GL_DEPTH_TEST);
}

void View::keyPressEvent(QKeyEvent* event)
{
	quint64 k = event->modifiers();
	k <<= 32;
	k |= event->key();

	switch (event->key())
	{
		case Qt::Key_V:
			schnapps_->cycle_selected_view();
			break;

		case Qt::Key_F: {
			save_snapshots_ = !save_snapshots_;
			if (save_snapshots_)
			{
				QMessageBox msg_box;
				msg_box.setText("Snapshot every frame?");
				msg_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
				msg_box.setDefaultButton(QMessageBox::Ok);
				if (msg_box.exec() == QMessageBox::Ok)
				{
					cgogn_log_info("View::keyPressEvent") << "frame snapshot !!";
					connect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));
				}
				else
				{
					cgogn_log_info("View::keyPressEvent") <<"Cancel frame snapshot.";
					save_snapshots_ = false;
				}
			}
			else
			{
				disconnect(this, SIGNAL(drawFinished(bool)), this, SLOT(saveSnapshot(bool)));
				cgogn_log_info("View::keyPressEvent") <<"Stop frame snapshot.";
			}
			break;
		}

		case Qt::Key_Escape: {
			QMessageBox msg_box;
			msg_box.setText("Really quit SCHNApps ?");
			msg_box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msg_box.setDefaultButton(QMessageBox::Ok);
			if (msg_box.exec() == QMessageBox::Ok)
				schnapps_->close_window();
			break;
		}

		case Qt::Key_C: {
			std::cout << current_camera()->to_string().toStdString() << std::endl;
		}

		default:
		{
			bool forward_event = true;
			for (PluginInteraction* plugin : plugins_)
				forward_event &= plugin->keyPress(this, event);

			if (forward_event)
				QOGLViewer::keyPressEvent(event);
		}
	}
}

void View::keyReleaseEvent(QKeyEvent *event)
{
	bool forward_event = true;
	for (PluginInteraction* plugin : plugins_)
		forward_event &= plugin->keyRelease(this, event);

	if (forward_event)
		QOGLViewer::keyReleaseEvent(event);
}

void View::mousePressEvent(QMouseEvent* event)
{
	if (!is_selected_view())
	{
		schnapps_->set_selected_view(this);
		cgogn_log_info("View::mousePressEvent") << "Selecting view \"" << this->name().toStdString() << "\".";
	}
	else if (event->y() < 20)
		cgogn_log_info("View::mousePressEvent") << this->name().toStdString() << ".";

	if (button_area_left_->is_clicked(event->x(), event->y()))
		button_area_left_->click_button(event->x(), event->y(), event->globalX(), event->globalY());
	else
	{
		hide_dialogs();
		if (button_area_->is_clicked(event->x(), event->y()))
			button_area_->click_button(event->x(), event->y(), event->globalX(), event->globalY());
		else
		{
			bool forward_event = true;
			for (PluginInteraction* plugin : plugins_)
				forward_event &= plugin->mousePress(this, event);

			if (forward_event)
				QOGLViewer::mousePressEvent(event);
		}
	}
}

void View::mouseReleaseEvent(QMouseEvent* event)
{
	bool forward_event = true;
	for (PluginInteraction* plugin : plugins_)
		forward_event &= plugin->mouseRelease(this, event);

	if (forward_event)
		QOGLViewer::mouseReleaseEvent(event);
}

void View::mouseMoveEvent(QMouseEvent* event)
{
	bool forward_event = true;
	for (PluginInteraction* plugin : plugins_)
		forward_event &= plugin->mouseMove(this, event);

	if (forward_event)
		QOGLViewer::mouseMoveEvent(event);
}

void View::wheelEvent(QWheelEvent* event)
{
	bool forward_event = true;
	for (PluginInteraction* plugin : plugins_)
		forward_event &= plugin->wheelEvent(this, event);

	if (forward_event)
		QOGLViewer::wheelEvent(event);
}

void View::hide_dialogs()
{
	if (dialog_objects_->isVisible())
		dialog_objects_->hide();
	if (dialog_plugins_->isVisible())
		dialog_plugins_->hide();
	if (dialog_cameras_->isVisible())
		dialog_cameras_->hide();
}

void View::close_dialogs()
{
	dialog_objects_->close();
	dialog_plugins_->close();
	dialog_cameras_->close();
}

//void View::selected_object_changed(Object* prev, Object* cur)
//{
//	if (cur && is_linked_to_object(cur))
//		this->setManipulatedFrame(&cur->get_frame());
//	this->update();
//}

void View::object_added(Object* o)
{
	if (o)
	{
		dialog_objects_->add_item(o->provider()->name() + "/" + o->name());
		if (schnapps_->core_setting("Link new object to selected view").toBool() && is_selected_view())
			link_object(o);
	}
}

void View::object_removed(Object* o)
{
	if (o)
		dialog_objects_->remove_item(o->name());
}

void View::object_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_)
	{
		if (item->checkState() == Qt::Checked)
			link_object(item->text(), false);
		else
			unlink_object(item->text(), false);
	}
}

void View::plugin_enabled(Plugin *plugin)
{
	if (dynamic_cast<PluginInteraction*>(plugin) && (!plugin->auto_activate()))
		dialog_plugins_->add_item(plugin->name());
}

void View::plugin_disabled(Plugin *plugin)
{
	if (dynamic_cast<PluginInteraction*>(plugin) && (!plugin->auto_activate()))
		dialog_plugins_->remove_item(plugin->name());
}

void View::plugin_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_)
	{
		if (item->checkState() == Qt::Checked)
			link_plugin(item->text(), false);
		else
			unlink_plugin(item->text(), false);
	}
}

void View::camera_added(Camera* camera)
{
	if (camera)
		dialog_cameras_->add_item(camera->name());
}

void View::camera_removed(Camera* camera)
{
	if (camera)
		dialog_cameras_->remove_item(camera->name());
}

void View::camera_check_state_changed(QListWidgetItem* item)
{
	if (!updating_ui_)
	{
		if (item->checkState() == Qt::Checked)
			set_current_camera(item->text());
	}
}

void View::update_bb()
{
	if (!objects_.empty())
	{
		bool initialized = false;

		for (Object* o : objects_)
		{
			qoglviewer::Vec minbb;
			qoglviewer::Vec maxbb;
			if (o->transformed_bb(minbb, maxbb))
			{
				if (initialized)
				{
					for (uint32 dim = 0; dim < 3; ++dim)
					{
						if (minbb[dim] < bb_min_[dim])
							bb_min_[dim] = minbb[dim];
						if (maxbb[dim] > bb_max_[dim])
							bb_max_[dim] = maxbb[dim];
					}
				}
				else
				{
					for (uint32 dim = 0; dim < 3; ++dim)
					{
						bb_min_[dim] = minbb[dim];
						bb_max_[dim] = maxbb[dim];
					}
					initialized = true;
				}
			}
		}

		if (!initialized)
		{
			bb_min_.setValue(0, 0, 0);
			bb_max_.setValue(0, 0, 0);
		}
	}
	else
	{
		bb_min_.setValue(0, 0, 0);
		bb_max_.setValue(0, 0, 0);
	}

	emit(bb_changed());
}

void View::color_selected()
{
	background_color_ = color_dial_->currentColor();
	this->makeCurrent();
	glClearColor(background_color_.redF(), background_color_.greenF(), background_color_.blueF(), background_color_.alphaF());
	this->update();
}

void View::ui_vertical_split_view(int, int, int, int)
{
	schnapps_->split_view(name_, Qt::Horizontal);
}

void View::ui_horizontal_split_view(int, int, int, int)
{
	schnapps_->split_view(name_, Qt::Vertical);
}

void View::ui_close_view(int, int, int, int)
{
	schnapps_->remove_view(name_);
}

void View::ui_color_view(int, int, int, int)
{
	color_dial_->show();
	color_dial_->setCurrentColor(background_color_);
}

void View::ui_objects_list_view(int, int, int globalX, int globalY)
{
	if (dialog_objects_->isHidden())
	{
		dialog_objects_->show();
		dialog_objects_->move(QPoint(globalX, globalY + 8));
		dialog_cameras_->hide();
		dialog_plugins_->hide();
	}
	else
		dialog_objects_->hide();
}

void View::ui_plugins_list_view(int, int, int globalX, int globalY)
{
	if (dialog_plugins_->isHidden())
	{
		dialog_plugins_->show();
		dialog_plugins_->move(QPoint(globalX, globalY + 8));
		dialog_objects_->hide();
		dialog_cameras_->hide();
	}
	else
		dialog_plugins_->hide();
}

void View::ui_cameras_list_view(int, int, int globalX, int globalY)
{
	if (dialog_cameras_->isHidden())
	{
		dialog_cameras_->show();
		dialog_cameras_->move(QPoint(globalX, globalY + 8));
		dialog_plugins_->hide();
		dialog_objects_->hide();
	}
	else
		dialog_cameras_->hide();
}

} // namespace schnapps
