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

#include <schnapps/plugins/shallow_water_2/dialog_shallow_water.h>
#include <schnapps/plugins/shallow_water_2/shallow_water.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

#include <QFileDialog>

namespace schnapps
{

namespace plugin_shallow_water_2
{

ShallowWater_Dialog::ShallowWater_Dialog(SCHNApps* s, Plugin_ShallowWater* p) :
	schnapps_(s),
	plugin_(p)
{
	setupUi(this);

	connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
	connect(button_start_stop, SIGNAL(clicked()), this, SLOT(start_stop()));
	connect(button_1_step, SIGNAL(clicked()), this, SLOT(step()));
}

void ShallowWater_Dialog::simu_running_state_changed()
{
	if (plugin_->is_simu_running())
		button_start_stop->setText("Stop");
	else
		button_start_stop->setText("Start");
}

void ShallowWater_Dialog::load()
{
	QString dossier = QFileDialog::getExistingDirectory(nullptr);
	plugin_->load_project(dossier);
	plugin_->init();
}

void ShallowWater_Dialog::start_stop()
{
	if (!plugin_->is_simu_running())
		plugin_->start();
	else
		plugin_->stop();
}

void ShallowWater_Dialog::step()
{
	plugin_->step();
}

} // namespace plugin_shallow_water_2

} // namespace schnapps
