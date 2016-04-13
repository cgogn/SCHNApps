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

#include <schnapps/core/schnapps.h>

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

	schnapps::SCHNApps schnapps(app.applicationDirPath());
	schnapps.show();

	splash.finish(&schnapps);

	return app.exec();
}
