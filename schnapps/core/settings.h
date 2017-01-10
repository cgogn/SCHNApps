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

#ifndef SCHNAPPS_CORE_SETTINGS_H_
#define SCHNAPPS_CORE_SETTINGS_H_

#include <schnapps/core/dll.h>
#include <QVariantMap>

namespace schnapps
{

class SCHNAPPS_CORE_API Settings
{
public:
	Settings();
	Settings(const QString& setting_filename);

public:
	QVariant& operator[](const QString& s) { return map_[s]; }
	const QVariant operator[](const QString& s) const { return map_[s]; }
	bool contains(const QString& s) const { return map_.contains(s); }
private:
	QVariantMap map_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_SETTINGS_H_
