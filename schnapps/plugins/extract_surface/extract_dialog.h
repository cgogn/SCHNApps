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

#ifndef SCHNAPPS_PLUGIN_EXTRACT_SURFACE_EXTRACT_DIALOG_H_
#define SCHNAPPS_PLUGIN_EXTRACT_SURFACE_EXTRACT_DIALOG_H_

#include <schnapps/plugins/extract_surface/plugin_extract_surface_export.h>
#include <ui_extract_dialog.h>

namespace schnapps
{

class SCHNApps;
class Object;

namespace plugin_extract_surface
{

class Plugin_ExtractSurface;

class PLUGIN_EXTRACT_SURFACE_EXPORT ExtractDialog : public QDialog, public Ui::ExtractSurface
{
	Q_OBJECT
	friend class Plugin_ExtractSurface;

public:
	ExtractDialog(SCHNApps* s, Plugin_ExtractSurface* p);

private slots:
	void map_added(Object*);
	void map_removed(Object*);
	void selected_map_changed(const QString&);
	void extract_validated();
private:
	SCHNApps* schnapps_;
	Plugin_ExtractSurface* plugin_;

	bool updating_ui_;
};

} // namespace plugin_extract_surface
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXTRACT_SURFACE_EXTRACT_DIALOG_H_
