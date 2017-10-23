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

#ifndef SCHNAPPS_CORE_SCHNAPPS_WINDOW_FACTORY_H_
#define SCHNAPPS_CORE_SCHNAPPS_WINDOW_FACTORY_H_

#include <schnapps/core/dll.h>

#include <QMainWindow>
#include <QString>

namespace schnapps
{

/**
* factory of SCHNApps_Window (avoid include of schnapps_window.h)
* definition in schnapps_window.cpp
*/
SCHNAPPS_CORE_API std::unique_ptr<QMainWindow> schnapps_window_factory(const QString& app_path, const QString& settings_path);

}

#endif // SCHNAPPS_CORE_SCHNAPPS_WINDOW_FACTORY_H_
