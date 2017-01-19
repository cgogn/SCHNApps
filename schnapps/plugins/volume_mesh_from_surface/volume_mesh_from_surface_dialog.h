/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Mesh From Surface                                              *
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
#ifndef SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_VECTOR_DIALOG_H_
#define SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_VECTOR_DIALOG_H_

#include "dll.h"
#include <schnapps/core/types.h>
#include <ui_cgal_export.h>
#include <ui_tetgen_export.h>
#include <ui_netgen_export.h>
#include <ui_export_dialog.h>
#include <memory>

namespace schnapps
{

class SCHNApps;
class MapHandlerGen;

namespace plugin_vmfs
{

class Plugin_VolumeMeshFromSurface;
struct MeshGeneratorParameters;

class SCHNAPPS_PLUGIN_VMFS_API ExportDialog : public QDialog, public Ui::Dialog_export
{};

class SCHNAPPS_PLUGIN_VMFS_API ExportNetgenDialog : public QDialog, public Ui::Dialog_netgen
{};

class SCHNAPPS_PLUGIN_VMFS_API ExportTetgenDialog : public QDialog, public Ui::Dialog_tetgen
{};

class SCHNAPPS_PLUGIN_VMFS_API ExportCGALDialog : public QDialog, public Ui::Dialog_cgal
{};

class SCHNAPPS_PLUGIN_VMFS_API VolumeMeshFromSurfaceDialog : QObject
{
	Q_OBJECT

	friend class Plugin_VolumeMeshFromSurface;

public:

	VolumeMeshFromSurfaceDialog(SCHNApps* s, Plugin_VolumeMeshFromSurface* p);

private:

	SCHNApps* schnapps_;
	Plugin_VolumeMeshFromSurface* plugin_;

	bool updating_ui_;
public slots:
	void show_export_dialog();

private slots:
	void generator_changed(const QString& generator);

	void selected_map_changed(QString map_name);
	void selected_image_changed(QString image_name);

	void cell_size_changed(double cs);
	void cell_radius_edge_ratio_changed(double ratio);
	void facet_angle_changed(double fa);
	void facet_size_changed(double fs);
	void facet_distance_changed(double fd);

	void odt_changed(bool b);
	void odt_freeze_changed(bool b);
	void odt_max_iter_changed(int nb_it);
	void odt_convergence_changed(double cv);
	void odt_freeze_bound_changed(double fb);

	void lloyd_changed(bool b);
	void lloyd_freeze_changed(bool b);
	void lloyd_max_iter_changed(int nb_it);
	void lloyd_convergence_changed(double cv);
	void lloyd_freeze_bound_changed(double fb);

	void perturber_changed(bool b);
	void perturber_sliver_changed(double sb);
	void exuder_changed(bool b);
	void exuder_sliver_changed(double sb);

	void map_added(MapHandlerGen* mhg);
	void map_removed(MapHandlerGen* mhg);

	void image_added(QString im_path);
	void image_removed(QString im_path);
	void tetgen_args_updated(QString str);

private:
	QString get_selected_map() const ;
	void update_mesh_generatuion_ui();

	std::unique_ptr<ExportCGALDialog> cgal_dialog_;
	std::unique_ptr<ExportNetgenDialog> netgen_dialog_;
	std::unique_ptr<ExportTetgenDialog> tetgen_dialog_;
	std::unique_ptr<ExportDialog> export_dialog_;

};

} // namespace plugin_vmfs
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MESH_FROM_SURFACE_VECTOR_DIALOG_H_
