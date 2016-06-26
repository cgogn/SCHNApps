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

#ifndef SCHNAPPS_PLUGIN_EXPORT_H_
#define SCHNAPPS_PLUGIN_EXPORT_H_

#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/map_handler.h>

// forward declaration of QAction
class QAction;

namespace schnapps
{

class MapHandlerGen;

namespace plugin_export
{

class ExportDialog;

struct ExportParams
{
	ExportParams();
	void reset();

	std::string map_name_;
	std::string position_attribute_name_;
	std::map<CellType, std::vector<std::string>> other_exported_attributes_;
	std::string output_;
	bool binary_;
	bool compress_;
};

/**
* @brief Plugin for CGoGN mesh import
*/
class Plugin_Export : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class ExportDialog;

public:
	Plugin_Export();
	~Plugin_Export() override;
	void export_mesh();

private:
	bool enable() override;
	void disable() override;

public slots:
	void export_mesh(const QString& filename);
	void export_mesh_from_file_dialog();

private:
	QAction* export_mesh_action_;
	ExportDialog* export_dialog_;
	ExportParams export_params_;
};

} // namespace plugin_export
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EXPORT_H_
