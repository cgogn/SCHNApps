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

#ifndef SCHNAPPS_CORE_SETTINGS_WIDGET_H_
#define SCHNAPPS_CORE_SETTINGS_WIDGET_H_

#include <schnapps/core/schnapps_core_export.h>

#include <ui_settings_widget.h>
#include <QWidget>

namespace schnapps
{

class Settings;

class SCHNAPPS_CORE_EXPORT SettingsWidget : public QWidget, public Ui::SettingsWidget
{
	friend class Settings;
	Q_OBJECT

public:
	SettingsWidget(const QString& settings_path, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
	~SettingsWidget() override;

	QString settings_export_path();
	bool export_at_exit();

public slots:
	void display_setting_widget();
	void save_mode_changed(int stateChanged);
	void export_settings();

private:
	void fill_widget();
	QHBoxLayout* add_option_to_widget(QWidget* widget, const QString& option_name, const QVariant& var);
	QHBoxLayout* add_option_to_widget(QWidget* widget, const QString& option_name, bool value);
	QHBoxLayout* add_option_to_widget(QWidget* widget, const QString& option_name, double value);
	QHBoxLayout* add_option_to_widget(QWidget* widget, const QString& option_name, const QString& value);
	QHBoxLayout* add_option_to_widget(QWidget* widget, const QString& option_name, const QVariantList& value);

	Settings* settings_;
	QString settings_path_;
	bool export_at_exit_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_SETTINGS_WIDGET_H_
