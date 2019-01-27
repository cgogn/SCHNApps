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

#ifndef SCHNAPPS_CORE_SCHNAPPS_WINDOW_H_
#define SCHNAPPS_CORE_SCHNAPPS_WINDOW_H_

#include <schnapps/core/schnapps_core_export.h>


#include <ui_schnapps.h>

#include <cgogn/core/utils/unique_ptr.h>
#include <cgogn/core/utils/assert.h>
#include <cgogn/core/utils/type_traits.h>

#include <QDockWidget>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMessageBox>

namespace schnapps
{

class SCHNApps;

class SCHNAPPS_CORE_EXPORT SCHNAppsWindow : public QMainWindow, public Ui::SCHNAppsWindow
{
	Q_OBJECT

	friend class SCHNApps;

public:

	SCHNAppsWindow(const QString& app_path, const QString& settings_path, const QString& init_plugin_name);
	~SCHNAppsWindow();

private slots:

	void about_SCHNApps()
	{
		QString str("SCHNApps:\nS... CGoGN Holder for Nice Applications\n"
					"Web site: http://cgogn.unistra.fr \n"
					"Contact information: cgogn@unistra.fr");
		QMessageBox::about(this, "About SCHNApps", str);
	}

	void about_CGoGN()
	{
		QString str("CGoGN:\nCombinatorial and Geometric modeling\n"
					"with Generic N-dimensional Maps\n"
					"Web site: http://cgogn.unistra.fr \n"
					"Contact information: cgogn@unistra.fr");
		QMessageBox::about(this, "About CGoGN", str);
	}

	void toggle_control_dock() { control_dock_->setVisible(control_dock_->isHidden()); }

	void toggle_plugin_dock() { plugin_dock_->setVisible(plugin_dock_->isHidden()); }

	/*********************************************************
	 * MANAGE MENU ACTIONS
	 *********************************************************/

	/**
	 * @brief add an entry in menu for a plugin
	 * @param menuPath path of menu (ex: "Surface; Import Mesh")
	 * @param action action to associate with entry
	 */
	QAction* add_menu_action(const QString& menu_path, const QString& action_text);

	/**
	 * @brief remove an entry in the menu
	 * @param action action entry to remove
	 */
	void remove_menu_action(QAction* action);

protected:

	void closeEvent(QCloseEvent *event);

	void keyPressEvent(QKeyEvent* event);

	std::unique_ptr<SCHNApps> schnapps_;

	QDockWidget* control_dock_;
	QTabWidget* control_dock_tab_widget_;

	QDockWidget* plugin_dock_;
	QTabWidget* plugin_dock_tab_widget_;

	std::unique_ptr<QWidget> settings_widget_;

	QVBoxLayout* central_layout_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_SCHNAPPS_H_
