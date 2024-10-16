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

#ifndef SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_EDITOR_H_
#define SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_EDITOR_H_

#include <schnapps/plugins/attribute_editor/plugin_attribute_editor_export.h>
#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/types.h>
#include <cgogn/core/basic/cell.h>

class QAction;

namespace schnapps
{

namespace plugin_cmap_provider
{
class Plugin_CMapProvider;
class CMapHandlerGen;
} // namespace plugin_cmap_provider

namespace plugin_attribute_editor
{

class AddAttributeDialog;
class EditAttributeDialog;

cgogn::Orbit PLUGIN_ATTRIBUTE_EDITOR_EXPORT orbit_from_string(const QString& str);
QStringList PLUGIN_ATTRIBUTE_EDITOR_EXPORT get_attribute_names(const plugin_cmap_provider::CMapHandlerGen* mhg, cgogn::Orbit orb);
const std::vector<cgogn::Orbit> PLUGIN_ATTRIBUTE_EDITOR_EXPORT available_orbits(const plugin_cmap_provider::CMapHandlerGen* mhg);

class PLUGIN_ATTRIBUTE_EDITOR_EXPORT AttributeEditorPlugin : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	AttributeEditorPlugin();
	inline ~AttributeEditorPlugin() override {}
	static QString plugin_name();
	const plugin_cmap_provider::Plugin_CMapProvider* cmap_provider() const;

private:

	bool enable() override;
	void disable() override;

	public slots:
	private slots:
	void add_attribute_dialog();
	void edit_attribute_dialog();

private:

	QAction* add_attribute_action_;
	QAction* edit_attribute_action_;
	AddAttributeDialog* add_attribute_dialog_;
	EditAttributeDialog* edit_attribute_dialog_;
	plugin_cmap_provider::Plugin_CMapProvider* plugin_cmap_provider_;
};

} // namespace plugin_attribute_editor

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_ATTRIBUTE_EDITOR_ATTRIBUTE_EDITOR_H_
