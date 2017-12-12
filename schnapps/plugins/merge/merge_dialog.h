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

#ifndef SCHNAPPS_MERGE_PLUGIN_MERGE_DIALOG_H
#define SCHNAPPS_MERGE_PLUGIN_MERGE_DIALOG_H

#include <schnapps/plugins/merge/dll.h>

#include <ui_merge_dialog.h>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_merge
{

class Plugin_Merge;

class SCHNAPPS_PLUGIN_MERGE_PLUGIN_API MergeDialog : public QDialog, public Ui::MergeDialog
{
	Q_OBJECT

public:

	MergeDialog(SCHNApps* s, Plugin_Merge* p);

private slots:

	// slots called from UI signals
	void merge_validated();

	// slots called from SCHNApps signals
	void map_added(MapHandlerGen*);
	void map_removed(MapHandlerGen*);

private:

	SCHNApps* schnapps_;
	Plugin_Merge* plugin_;
};

} // namespace plugin_merge

} // namespace schnapps

#endif // SCHNAPPS_MERGE_PLUGIN_MERGE_DIALOG_H
