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

#include <schnapps/core/schnapps_window.h>
#include <cgogn/core/utils/logger.h>
#include <QOGLViewer/qoglviewer.h>

#include <QApplication>
#include <QSplashScreen>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	qoglviewer::init_ogl_context();

	QSplashScreen splash(QPixmap(":splash/cgogn/splash.png"));
	splash.show();
	splash.showMessage("Welcome to SCHNApps", Qt::AlignBottom | Qt::AlignCenter);

	QString settings_path;
	if (argc==2)
		settings_path = QString(argv[1]);
	if (! settings_path.endsWith(".json", Qt::CaseInsensitive))
	{
		if (!settings_path.isEmpty())
			cgogn_log_warning("Main Application") << "Invalid settings file: " << settings_path.toStdString() << " is not a json file";
#if defined (WIN32)
		settings_path = app.applicationDirPath() + QString("/settings.json");
#elif defined (__APPLE__)
		settings_path = app.applicationDirPath() + QString("/lib/settings.json");
#else
		settings_path = app.applicationDirPath() + QString("/../lib/settings.json");
#endif
	}

	schnapps::SCHNAppsWindow w(app.applicationDirPath(), settings_path);
	w.show();

	splash.finish(&w);

	return app.exec();
}
