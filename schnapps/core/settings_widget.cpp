/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#include <schnapps/core/settings_widget.h>
#include <schnapps/core/settings.h>
#include <cgogn/core/utils/logger.h>

#include <QLayout>
#include <QCheckBox>
#include <QSpacerItem>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDoubleSpinBox>

namespace schnapps
{

SettingsWidget::SettingsWidget(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f),
	settings_(nullptr)
{
	setupUi(this);
}

SettingsWidget::~SettingsWidget()
{

}

void SettingsWidget::display_setting_widget()
{
	fill_widget();
	show();
}

QHBoxLayout* SettingsWidget::add_option_to_widget(QWidget* widget, const QString& option_name, const QVariant& var)
{
	switch (var.type())
	{
		case QVariant::Type::Bool: return add_option_to_widget(widget, option_name, var.toBool());
		case QVariant::Type::Double: return add_option_to_widget(widget, option_name, var.toDouble());
		case QVariant::Type::String: return add_option_to_widget(widget, option_name, var.toString());
		case QVariant::Type::List: return add_option_to_widget(widget, option_name, var.toList());
		default:
		{
			cgogn_log_debug("SettingsWidget::add_option_to_widget") << "Unable to add an option of type " << var.typeName();
			return nullptr;
		}
	}
}

QHBoxLayout* SettingsWidget::add_option_to_widget(QWidget* widget, const QString& option_name, bool value)
{
	QHBoxLayout* hlay = new QHBoxLayout();
	QCheckBox* checkbox = new QCheckBox(widget);
	checkbox->setText(option_name);
	checkbox->setChecked(value);
	checkbox->setObjectName(option_name);
	connect(checkbox, SIGNAL(toggled(bool)), settings_, SLOT(setting_changed_bool(bool)));
	hlay->addWidget(checkbox);
	return hlay;
}

QHBoxLayout*SettingsWidget::add_option_to_widget(QWidget* widget, const QString& option_name, double value)
{
	QHBoxLayout* hlay = new QHBoxLayout();
	QLabel* label = new QLabel(widget);
	label->setText(option_name);
	QDoubleSpinBox* spinbox = new QDoubleSpinBox(widget);
	spinbox->setValue(value);
	spinbox->setObjectName(option_name);
	connect(spinbox, SIGNAL(valueChanged(double)), settings_, SLOT(setting_changed_double(double)));
	hlay->addWidget(label);
	hlay->addWidget(spinbox);
	QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
	hlay->addItem(spacer);
	return hlay;
}

QHBoxLayout* SettingsWidget::add_option_to_widget(QWidget* widget, const QString& option_name, const QString& value)
{
	QHBoxLayout* hlay = new QHBoxLayout();
	QLabel* label = new QLabel(widget);
	label->setText(option_name);
	QLineEdit* line_edit = new QLineEdit(widget);
	line_edit->setText(value);
	line_edit->setObjectName(option_name);
	connect(line_edit, SIGNAL(textChanged(QString)), settings_, SLOT(setting_changed_string(QString)));
	hlay->addWidget(label);
	hlay->addWidget(line_edit);
	return hlay;
}

QHBoxLayout*SettingsWidget::add_option_to_widget(QWidget* widget, const QString& option_name, const QList<QVariant>& value)
{
	QHBoxLayout* hlay = new QHBoxLayout();
	QLabel* label = new QLabel(widget);
	label->setText(option_name);
	QListWidget* list_widget = new QListWidget(widget);
	list_widget->setObjectName(option_name);
	for (const QVariant& item_var : value)
	{
		const QString str = item_var.toString();
		if (str.isEmpty())
			continue;
		QListWidgetItem *item = new QListWidgetItem(list_widget);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setText(str);
	}
	connect(list_widget, SIGNAL(itemChanged(QListWidgetItem*)), settings_, SLOT(setting_changed_list(QListWidgetItem*)));
	hlay->addWidget(label);
	hlay->addWidget(list_widget);
	return hlay;
}

void SettingsWidget::fill_widget()
{
	if(!settings_)
	{
		cgogn_log_debug("SettingsWidget::fill_widget()") << "Invalid Settings.";
		return;
	}

	tabWidget->clear();

	for(auto it = settings_->modules_.constBegin(); it != settings_->modules_.constEnd(); ++it)
	{
		QWidget* new_w = new QWidget();
		tabWidget->addTab(new_w, it.key());
		QVBoxLayout* vertical_layout = new QVBoxLayout(new_w);
		for (const QString& key : it.value())
		{
			QHBoxLayout* hlay = SettingsWidget::add_option_to_widget(new_w, key, settings_->map_[key]);
			if (hlay)
				vertical_layout->addLayout(hlay);
		}
		QSpacerItem* spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
		vertical_layout->addItem(spacer);
	}
}

} // namespace schnapps
