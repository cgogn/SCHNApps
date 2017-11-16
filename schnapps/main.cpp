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

#include <cgogn/core/utils/logger.h>

#include <schnapps/core/schnapps_window_factory.h>

#include <QOGLViewer/qoglviewer.h>

#include <QApplication>
#include <QSplashScreen>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	qoglviewer::init_ogl_context();

	QSplashScreen splash(QPixmap(":splash/cgogn/splash.png"));
	splash.show();
	splash.showMessage("Welcome to SCHNApps", Qt::AlignBottom | Qt::AlignCenter);

	QString default_settings_path;
#if defined (WIN32)
	default_settings_path = app.applicationDirPath() + QString("/settings.json");
#elif defined (__APPLE__)
	default_settings_path = app.applicationDirPath() + QString("/lib/settings.json");
#else
	default_settings_path = app.applicationDirPath() + QString("/../lib/settings.json");
#endif

	QCommandLineParser parser;
	parser.addOptions({
		{ { "s", "settings" }, "Settings file path", "settings", default_settings_path },
		{ { "i", "init" }, "Initialization plugin name", "init" }
	});

	parser.process(app);

	QString settings_path = parser.value("s");
	if (!settings_path.endsWith(".json", Qt::CaseInsensitive))
	{
		cgogn_log_warning("Main Application") << "Invalid settings file: " << settings_path.toStdString() << " is not a json file";
		settings_path.clear();
	}

	QString init_plugin_name = parser.value("i");

	std::unique_ptr<QMainWindow> w = schnapps::schnapps_window_factory(app.applicationDirPath(), settings_path, init_plugin_name);
	w->show();
	splash.finish(w.get());

	return app.exec();
}
