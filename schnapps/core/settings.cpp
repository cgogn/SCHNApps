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

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <cgogn/core/utils/logger.h>
#include <schnapps/core/settings.h>

namespace schnapps
{


Settings::Settings()
{}

Settings::Settings(const QString& setting_filename)
{
	QFile setting_file(setting_filename);
	if (setting_file.exists() && setting_file.open(QIODevice::ReadOnly))
	{
		QByteArray data =  setting_file.readAll();

		QJsonParseError parse_error;
		QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);
		if (parse_error.error != QJsonParseError::NoError)
		{
			cgogn_log_warning("Settings(const QString&)") << "Error while reading the file \"" << setting_filename.toStdString() << "\". Error is : \"" << parse_error.errorString().toStdString() << "\".";
		} else {
			cgogn_log_info("Settings(const QString&)") << "Loaded setting file \"" << setting_filename.toStdString() << "\".";
		}
		map_ = doc.object().toVariantMap();
	} else {
		cgogn_log_warning("Settings(const QString&)") << "Unable to read the file \"" << setting_filename.toStdString() << "\".";
	}
}

} // namespace schnapps
