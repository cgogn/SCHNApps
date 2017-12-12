/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin ExtractSurface                                                        *
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

#ifndef SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_EDIT_ATTRIBUTE_DIALOG_H
#define SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_EDIT_ATTRIBUTE_DIALOG_H

#include <schnapps/plugins/attribute_editor/dll.h>

#include <ui_edit_attribute_dialog.h>

#include <schnapps/core/types.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_attribute_editor
{

class AttributeEditorPlugin;

class SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_API EditAttributeDialog : public QDialog, public Ui::EditAttribute
{
	Q_OBJECT
	friend class AttributeEditorPlugin;

public:
	EditAttributeDialog(SCHNApps* s, AttributeEditorPlugin* p);

private slots:
	void map_added(MapHandlerGen*);
	void map_removed(MapHandlerGen*);
	void selected_map_changed(const QString&);
	void orbit_changed(const QString&);
	void cells_set_changed(const QString&);
	void attribute_changed(const QString&);
	void edit_attribute_validated();

private:
	void update_cells_sets(MapHandlerGen*, CellType ct);
	void update_attribute_list(MapHandlerGen*, CellType ct);
private:
	SCHNApps* schnapps_;
	AttributeEditorPlugin* plugin_;
	bool updating_ui_;
};

} // namespace plugin_attribute_editor

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_EDIT_ATTRIBUTE_DIALOG_H
