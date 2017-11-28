/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Attribute Editor                                                      *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#include "attribute_editor.h"
#include <schnapps/core/schnapps.h>
#include <QAction>
#include "add_attribute_dialog.h"
#include "edit_attribute_dialog.h"

namespace schnapps
{

namespace plugin_attribute_editor
{

QString AttributeEditorPlugin::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}


AttributeEditorPlugin::AttributeEditorPlugin() :
	add_attribute_action_(nullptr),
	add_attribute_dialog_(nullptr),
	edit_attribute_dialog_(nullptr)
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

AttributeEditorPlugin::~AttributeEditorPlugin()
{
	delete add_attribute_dialog_;
	delete edit_attribute_dialog_;
}

bool AttributeEditorPlugin::enable()
{
	if (!add_attribute_dialog_)
		add_attribute_dialog_ = new AddAttributeDialog(schnapps_, this);
	if (!edit_attribute_dialog_)
		edit_attribute_dialog_ = new EditAttributeDialog(schnapps_, this);

	add_attribute_action_ = schnapps_->add_menu_action("Attribute;Add attribute", "Add attribute");
	edit_attribute_action_ = schnapps_->add_menu_action("Attribute;Edit attribute", "Edit attribute");
	connect(add_attribute_action_, SIGNAL(triggered()), this, SLOT(add_attribute_dialog()));
	connect(edit_attribute_action_, SIGNAL(triggered()), this, SLOT(edit_attribute_dialog()));
	return true;
}

void AttributeEditorPlugin::disable()
{
	disconnect(add_attribute_action_, SIGNAL(triggered()), this, SLOT(add_attribute_dialog()));
	disconnect(edit_attribute_action_, SIGNAL(triggered()), this, SLOT(edit_attribute_dialog()));
	schnapps_->remove_menu_action(add_attribute_action_);
	schnapps_->remove_menu_action(edit_attribute_action_);
}

void AttributeEditorPlugin::add_attribute_dialog()
{
	add_attribute_dialog_->show();
}

void AttributeEditorPlugin::edit_attribute_dialog()
{
	edit_attribute_dialog_->show();
}

} // namespace plugin_attribute_editor
} // namespace schnapps
