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
#include <QStringList>
#include <memory>

class QHBoxLayout;
class QListWidgetItem;

namespace schnapps
{

class SettingsWidget;

class SCHNAPPS_CORE_API Settings : public QObject
{
	Q_OBJECT

	friend class SettingsWidget;

public:

	Settings() = default;
	Settings(const Settings&) = delete;
	Settings(Settings&&) = delete;
	Settings& operator=(const Settings&) = delete;
	Settings& operator=(Settings&&) = delete;
	~Settings() override;

	/**
	 * @brief add_setting, store a new setting
	 * @param module_name
	 * @param setting_name
	 * @param value
	 * @return the setting value
	 */
	QVariant add_setting(const QString& module_name, const QString& setting_name, const QVariant& value);
	const QVariant setting(const QString& module_name, const QString& setting_name) const;

	void to_file(const QString& filename);
	static std::unique_ptr<Settings> from_file(const QString& setting_filename);

	void set_widget(QWidget* widget);

private slots:

	void setting_changed(const QString& module_name, const QString& name, const QVariant& value);
	void setting_changed_bool(bool b);
	void setting_changed_double(double d);
	void setting_changed_string(const QString& str);
	void setting_changed_list(QListWidgetItem* item);

private:

	void add_module(const QString& module_name, const QVariantMap& module);
	SettingsWidget* settings_widget_;
	QMap<QString, QVariantMap> map_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_SETTINGS_H_
