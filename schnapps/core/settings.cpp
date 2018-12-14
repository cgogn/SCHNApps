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
		QByteArray data = setting_file.readAll();

		QJsonParseError parse_error;
		QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);
		if (parse_error.error != QJsonParseError::NoError)
			cgogn_log_warning("Settings(const QString&)") << "Error while reading the file \"" << setting_filename.toStdString() << "\". Error is : \"" << parse_error.errorString().toStdString() << "\".";
		else
			cgogn_log_info("Settings(const QString&)") << "Loaded setting file \"" << setting_filename.toStdString() << "\".";
		const auto obj = doc.object();
		for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
			settings->add_module(it.key(), it.value().toObject().toVariantMap());
	}
	else
		cgogn_log_warning("Settings::from_file()") << "Unable to read the file \"" << setting_filename.toStdString() << "\".";

	return settings;
}

void Settings::add_module(const QString& module_name, const QVariantMap& module)
{
	if (!map_.contains(module_name))
		map_[module_name] = module;
}

Settings::~Settings()
{}

QVariant Settings::add_setting(const QString& module_name, const QString& setting_name, const QVariant& value)
{
	if (!map_[module_name].contains(setting_name))
		map_[module_name][setting_name] = value;
	return map_[module_name][setting_name];
}

const QVariant Settings::setting(const QString& module_name, const QString& setting_name) const
{
	auto module_it = map_.constFind(module_name);
	if (module_it != map_.constEnd())
	{
		auto setting_it = module_it.value().constFind(setting_name);
		if (setting_it != module_it.value().constEnd())
			return setting_it.value();
		else
			cgogn_log_debug("Settings::setting") << "Unable to find setting \"" << setting_name.toStdString() << "\" in module \"" << module_name.toStdString() << "\".";
	}
	else
		cgogn_log_debug("Settings::setting") << "Unable to find module \"" << module_name.toStdString() << "\".";
	return QVariant();
}

void Settings::to_file(const QString& filename)
{
	QFile::remove(filename);
	QFile setting_file(filename);
	if (setting_file.open(QIODevice::WriteOnly))
	{
		QJsonObject obj;
		for (auto it = map_.constBegin(); it != map_.constEnd() ; ++it)
			obj[it.key()] = QJsonObject::fromVariantMap(it.value());
		QJsonDocument doc(obj);
		setting_file.write(doc.toJson());
	}
	else
		cgogn_log_info("Settings::to_file()") << "Unable to write in the file \"" << filename.toStdString() << "\".";
}

void Settings::set_widget(QWidget* widget)
{
	settings_widget_ = qobject_cast<SettingsWidget*>(widget);
	if (settings_widget_)
		settings_widget_->settings_ = this;
}

void Settings::setting_changed(const QString& module_name, const QString& name, const QVariant& value)
{
	if (!map_.contains(module_name))
		cgogn_log_debug("Settings::setting_changed") << "Trying to modify a setting of a non-existing module \"" << module_name.toStdString() << "\".";
	else
	{
		if (!map_[module_name].contains(name))
			cgogn_log_debug("Settings::setting_changed") << "Trying to modify a non-existing setting \"" << name.toStdString() << "\" of the module \"" << module_name.toStdString() << "\".";
		else
		{
			QVariant& v = map_[module_name][name];
			if (v.type() != value.type())
				cgogn_log_debug("Settings::setting_changed") << "Cannot replace a setting of type \"" << v.typeName() << "\" by another setting of type \"" << value.typeName() << "\".";
			else
				v = value;
		}
	}
}

void Settings::setting_changed_bool(bool b)
{
	QObject* sender = this->sender();
	setting_changed(sender->parent()->objectName(), sender->objectName(), QVariant(b));
}

void Settings::setting_changed_double(double d)
{
	QObject* sender = this->sender();
	setting_changed(sender->parent()->objectName(), sender->objectName(), QVariant(d));
}

void Settings::setting_changed_string(const QString& str)
{
	QObject* sender = this->sender();
	setting_changed(sender->parent()->objectName(), sender->objectName(), QVariant(str));
}

void Settings::setting_changed_list(QListWidgetItem* item)
{
	QObject* sender = this->sender();
	QListWidget* listw = qobject_cast<QListWidget*>(sender);
	if (listw)
	{
		const QString module_name = sender->parent()->objectName();
		const int index = listw->row(item);
		QVariantList list = map_[module_name][sender->objectName()].toList();
		if (item->text().isEmpty())
			list.removeAt(index);
		else
		{
			if (index >= list.size())
				list.push_back(QVariant(item->text()));
			else
				list[index] = QVariant(item->text());
		}
		map_[module_name][sender->objectName()] = QVariant(list);
	}
}

} // namespace schnapps
