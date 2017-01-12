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

#ifndef SCHNAPPS_PLUGIN_IMPORT_H_
#define SCHNAPPS_PLUGIN_IMPORT_H_

#include <schnapps/core/plugin_processing.h>
#include <schnapps/core/settings.h>
#include <schnapps/plugins/import/dll.h>
#include <QAction>


namespace schnapps
{

class MapHandlerGen; // forward declaration of class

namespace plugin_import
{

/**
* @brief Plugin for CGoGN mesh import
*/
class SCHNAPPS_PLUGIN_IMPORT_API Plugin_Import : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	inline Plugin_Import() {}
	~Plugin_Import() override {}

private:

	bool enable() override;
	void disable() override;

	public slots:

	/**
		* @brief import a surface mesh from a file
		* @param filename file name of mesh file
		* @return a new MapHandlerGen that handles the mesh
		*/
	MapHandlerGen* import_surface_mesh_from_file(const QString& filename);

	/**
		* @brief import a surface mesh by opening a FileDialog
		*/
	void import_surface_mesh_from_file_dialog();

	MapHandlerGen* import_volume_mesh_from_file(const QString& filename);
	void import_volume_mesh_from_file_dialog();
	//	/**
	//	 * @brief import a 2D image into a surface mesh from a file
	//	 * @param filename file name of mesh file
	//	 * @return a new MapHandlerGen that handles the mesh
	//	 */
	//	MapHandlerGen* import_2D_image_from_file(const QString& filename);

	//	/**
	//	 * @brief import a 2D image into a surface mesh by opening a FileDialog
	//	 */
	//	void import_2D_image_from_file_dialog();

private:

	QAction* import_surface_mesh_action_;
	QAction* import_volume_mesh_action_;
	//	QAction* import_2D_image_action_;
};

} // namespace plugin_import
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_IMPORT_H_
