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
#include <schnapps/core/settings.h>
#include <schnapps/core/settings_widget.h>

#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QListWidget>

namespace schnapps
{

std::unique_ptr<Settings> Settings::from_file(const QString& setting_filename)
{
	std::unique_ptr<Settings> settings = cgogn::make_unique<Settings>();
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
		const auto obj = doc.object();
		for(auto it = obj.constBegin(); it != obj.constEnd(); ++it)
			settings->add_module(it.key(), it.value().toObject().toVariantMap());
	} else {
		cgogn_log_warning("Settings::from_file()") << "Unable to read the file \"" << setting_filename.toStdString() << "\".";
	}
	return settings;
}

void Settings::add_module(const QString& module_name, const QVariantMap& module)
{
	for (auto it = module.constBegin(); it != module.constEnd() ; ++it)
	{
		if (!map_.contains(it.key()))
		{
			map_[it.key()] = it.value();
			modules_[module_name].push_back(it.key());
		}
	}
}

Settings::~Settings()
{

}

void Settings::add_setting(const QString& module_name, const QString& setting_name, const QVariant& value)
{
	if (!map_.contains(setting_name))
	{
		map_[setting_name] = value;
		modules_[module_name].push_back(setting_name);
	}
}

void Settings::to_file(const QString& filename)
{
	QFile::remove(filename);
	QFile setting_file(filename);
	if (setting_file.open(QIODevice::WriteOnly))
	{
		QJsonObject obj;
		for (auto it = modules_.constBegin(); it != modules_.constEnd() ; ++it)
		{
			QVariantMap map;
			for (const QString s : it.value())
				map[s] = map_[s];
			obj[it.key()] = QJsonObject::fromVariantMap(map);
		}
		QJsonDocument doc(obj);
		setting_file.write(doc.toJson());
	} else {
		cgogn_log_info("Settings::to_file()") << "Unable to write in the file \"" << filename.toStdString() << "\".";
	}
}

void Settings::set_widget(QWidget* widget)
{
	settings_widget_ = dynamic_cast<SettingsWidget*>(widget);
	if (settings_widget_)
		settings_widget_->settings_ = this;
}

void Settings::setting_changed(const QString& name, const QVariant& value)
{
	if (!map_.contains(name))
	{
		cgogn_log_debug("Settings::setting_changed") << "Trying to modify a non-existing setting \"" << name.toStdString() << "\".";
	} else {
		QVariant& v = map_[name];
		if (v.type() != value.type())
		{
			cgogn_log_debug("Settings::setting_changed") << "Cannot replace a setting of type \"" << v.typeName() << "\" by another setting of type \"" << value.typeName() << "\".";
		} else {
			v = value;
		}
	}
}

void Settings::setting_changed_bool(bool b)
{
	setting_changed(QObject::sender()->objectName(), QVariant(b));
}

void Settings::setting_changed_double(double d)
{
	setting_changed(QObject::sender()->objectName(), QVariant(d));
}

void Settings::setting_changed_string(const QString& str)
{
	setting_changed(QObject::sender()->objectName(), QVariant(str));
}

void Settings::setting_changed_list(QListWidgetItem* item)
{
	QObject* sender = QObject::sender();
	QListWidget* listw = dynamic_cast<QListWidget*>(sender);
	if (listw)
	{
		const int index = listw->row(item);
		QVariantList list = map_[sender->objectName()].toList();
		if (item->text().isEmpty())
			list.removeAt(index);
		else
		{
			if (index >= list.size())
				list.push_back(QVariant(item->text()));
			else
				list[index] = QVariant(item->text());
		}
		map_[sender->objectName()] = QVariant(list);
	}
}

} // namespace schnapps
