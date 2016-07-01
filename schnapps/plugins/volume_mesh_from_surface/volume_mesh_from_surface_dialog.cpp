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

#define SCHNAPPS_PLUGIN_VMFS_DLL_EXPORT

#include <volume_mesh_from_surface.h>
#include <volume_mesh_from_surface_dialog.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include "image.h"

namespace schnapps
{

namespace plugin_vmfs
{

VolumeMeshFromSurfaceDialog::VolumeMeshFromSurfaceDialog(SCHNApps* s, Plugin_VolumeMeshFromSurface* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	export_dialog_ = cgogn::make_unique<ExportDialog>();
	cgal_dialog_ = cgogn::make_unique<ExportCGALDialog>();
	tetgen_dialog_ = cgogn::make_unique<ExportTetgenDialog>();

	this->export_dialog_->setupUi(export_dialog_.get());
	this->cgal_dialog_->setupUi(cgal_dialog_.get());
	this->tetgen_dialog_->setupUi(tetgen_dialog_.get());

	connect(this->export_dialog_->comboBoxMapSelection, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(this->export_dialog_->pushButton_gen_volume_meshTetgen, SIGNAL(pressed()), this->tetgen_dialog_.get(), SLOT(show()));
	connect(this->export_dialog_->pushButtonGenMeshCGAL, SIGNAL(pressed()), this->cgal_dialog_.get(), SLOT(show()));

	connect(this->export_dialog_->pushButton_gen_volume_meshTetgen, SIGNAL(pressed()), this->export_dialog_.get(), SLOT(hide()));
	connect(this->export_dialog_->pushButtonGenMeshCGAL, SIGNAL(pressed()), this->export_dialog_.get(), SLOT(hide()));

	connect(this->tetgen_dialog_->buttonBox,SIGNAL(accepted()), plugin_, SLOT(generate_button_tetgen_pressed()));
	connect(this->cgal_dialog_->buttonBox, SIGNAL(accepted()), plugin_, SLOT(generate_button_cgal_pressed()));
	connect(this->tetgen_dialog_->lineEdit_tetgen_args, SIGNAL(textChanged(QString)), this, SLOT(tetgen_args_updated(QString)));

	connect(this->cgal_dialog_->doubleSpinBox_CellSize, SIGNAL(valueChanged(double)), this, SLOT(cell_size_changed(double)));
	connect(this->cgal_dialog_->doubleSpinBox_Radius, SIGNAL(valueChanged(double)), this, SLOT(cell_radius_edge_ratio_changed(double)));
	connect(this->cgal_dialog_->doubleSpinBoxFacetAngle, SIGNAL(valueChanged(double)), this, SLOT(facet_angle_changed(double)));
	connect(this->cgal_dialog_->doubleSpinBox_FacetSize, SIGNAL(valueChanged(double)), this, SLOT(facet_size_changed(double)));
	connect(this->cgal_dialog_->doubleSpinBox_3FacetDistance, SIGNAL(valueChanged(double)), this, SLOT(facet_distance_changed(double)));

	connect(this->cgal_dialog_->checkBox_ODT, SIGNAL(toggled(bool)), this, SLOT(odt_changed(bool)));
	connect(this->cgal_dialog_->checkBox_FreezeODT, SIGNAL(toggled(bool)), this, SLOT(odt_freeze_changed(bool)));
	connect(this->cgal_dialog_->spinBox_ODTMaxIter, SIGNAL(valueChanged(int)), this, SLOT(odt_max_iter_changed(int)));
	connect(this->cgal_dialog_->doubleSpinBox_OdtConvergence, SIGNAL(valueChanged(double)), this, SLOT(odt_convergence_changed(double)));
	connect(this->cgal_dialog_->doubleSpinBox_8OdtFreeze, SIGNAL(valueChanged(double)), this, SLOT(odt_freeze_bound_changed(double)));

	connect(this->cgal_dialog_->checkBoxLloyd, SIGNAL(toggled(bool)), this, SLOT(lloyd_changed(bool)));
	connect(this->cgal_dialog_->checkBox_FreezeLloyd, SIGNAL(toggled(bool)), this, SLOT(lloyd_freeze_changed(bool)));
	connect(this->cgal_dialog_->spinBoxMaxITLloyd, SIGNAL(valueChanged(int)), this, SLOT(lloyd_max_iter_changed(int)));
	connect(this->cgal_dialog_->doubleSpinBox_ConvergenceLloyd, SIGNAL(valueChanged(double)), this, SLOT(lloyd_convergence_changed(double)));
	connect(this->cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd, SIGNAL(valueChanged(double)), this, SLOT(lloyd_freeze_bound_changed(double)));

	connect(this->cgal_dialog_->checkBox_Perturber, SIGNAL(toggled(bool)), this, SLOT(perturber_changed(bool)));
	connect(this->cgal_dialog_->doubleSpinBox_PerturberSliver, SIGNAL(valueChanged(double)), this, SLOT(perturber_sliver_changed(double)));

	connect(this->cgal_dialog_->checkBox_5Exuder, SIGNAL(toggled(bool)), this, SLOT(exuder_changed(bool)));
	connect(this->cgal_dialog_->doubleSpinBox_ExuderSliver, SIGNAL(valueChanged(double)), this, SLOT(exuder_sliver_changed(double)));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(this->export_dialog_->comboBox_images, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_image_changed(QString)));

	update_mesh_generatuion_ui();

	this->export_dialog_->pushButton_gen_volume_meshTetgen->setDisabled(true);
	this->export_dialog_->pushButtonGenMeshCGAL->setDisabled(true);

}

void VolumeMeshFromSurfaceDialog::show_export_dialog()
{
	this->export_dialog_->show();
}

void VolumeMeshFromSurfaceDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg && dynamic_cast<MapHandler<CMap2>*>(mhg))
		this->export_dialog_->comboBoxMapSelection->addItem(mhg->get_name());
}

void VolumeMeshFromSurfaceDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg && dynamic_cast<MapHandler<CMap2>*>(mhg))
		this->export_dialog_->comboBoxMapSelection->removeItem(this->export_dialog_->comboBoxMapSelection->findText(mhg->get_name()));
}

void VolumeMeshFromSurfaceDialog::image_added(QString im_path)
{
	this->export_dialog_->comboBox_images->addItem(im_path);
}

void VolumeMeshFromSurfaceDialog::image_removed(QString im_path)
{
	this->export_dialog_->comboBox_images->removeItem(this->export_dialog_->comboBox_images->findText(im_path));
}

QString VolumeMeshFromSurfaceDialog::get_selected_map() const
{
	return export_dialog_->comboBoxMapSelection->currentText();
}

void VolumeMeshFromSurfaceDialog::selected_map_changed(QString map_name)
{
	MapHandlerGen* mhg = schnapps_->get_map(map_name);

	if (mhg && dynamic_cast<MapHandler<CMap2>*>(mhg))
	{
		this->export_dialog_->comboBox_images->setCurrentIndex(0);
		this->export_dialog_->pushButton_gen_volume_meshTetgen->setDisabled(false);
		this->export_dialog_->pushButtonGenMeshCGAL->setDisabled(false);


		this->export_dialog_->comboBoxPositionSelection->clear();
		const auto& vert_att_cont = mhg->const_attribute_container(CellType::Vertex_Cell);
		for (const auto& att_name : vert_att_cont.names())
			this->export_dialog_->comboBoxPositionSelection->addItem(QString::fromStdString(att_name));

	} else {
		this->export_dialog_->pushButton_gen_volume_meshTetgen->setDisabled(true);
		this->export_dialog_->pushButtonGenMeshCGAL->setDisabled(true);
	}

}

void VolumeMeshFromSurfaceDialog::selected_image_changed(QString /*image_name*/)
{
	this->export_dialog_->comboBoxMapSelection->setCurrentIndex(0);
	if (this->export_dialog_->comboBox_images->currentIndex() >= 1)
	{
		this->export_dialog_->pushButton_gen_volume_meshTetgen->setDisabled(true);
		this->export_dialog_->pushButtonGenMeshCGAL->setDisabled(false);
	}
}

void VolumeMeshFromSurfaceDialog::cell_size_changed(double cs)
{
	plugin_->generation_parameters_.cell_size_ = cs;
}

void VolumeMeshFromSurfaceDialog::cell_radius_edge_ratio_changed(double ratio)
{
	plugin_->generation_parameters_.cell_radius_edge_ratio_ = ratio;
}

void VolumeMeshFromSurfaceDialog::facet_angle_changed(double fa)
{
	plugin_->generation_parameters_.facet_angle_ = fa;
}

void VolumeMeshFromSurfaceDialog::facet_size_changed(double fs)
{
	plugin_->generation_parameters_.facet_size_ = fs;
}

void VolumeMeshFromSurfaceDialog::facet_distance_changed(double fd)
{
	plugin_->generation_parameters_.facet_distance_ = fd;
}

void VolumeMeshFromSurfaceDialog::odt_changed(bool b)
{
	plugin_->generation_parameters_.do_odt_ = b;
	this->cgal_dialog_->doubleSpinBox_OdtConvergence->setDisabled(!b);
	this->cgal_dialog_->spinBox_ODTMaxIter->setDisabled(!b);
	this->cgal_dialog_->checkBox_FreezeODT->setDisabled(!b);
	this->cgal_dialog_->doubleSpinBox_8OdtFreeze->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::odt_freeze_changed(bool b)
{
	plugin_->generation_parameters_.do_odt_freeze_ = b;
	this->cgal_dialog_->doubleSpinBox_8OdtFreeze->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::odt_max_iter_changed(int nb_it)
{
	plugin_->generation_parameters_.odt_max_iter_ = nb_it;
}

void VolumeMeshFromSurfaceDialog::odt_convergence_changed(double cv)
{
	plugin_->generation_parameters_.odt_convergence_ = cv;
}

void VolumeMeshFromSurfaceDialog::odt_freeze_bound_changed(double fb)
{
	plugin_->generation_parameters_.odt_freeze_bound_ = fb;
}

void VolumeMeshFromSurfaceDialog::lloyd_changed(bool b)
{
	plugin_->generation_parameters_.do_lloyd_ = b;
	this->cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!b);
	this->cgal_dialog_->doubleSpinBox_ConvergenceLloyd->setDisabled(!b);
	this->cgal_dialog_->spinBoxMaxITLloyd->setDisabled(!b);
	this->cgal_dialog_->checkBox_FreezeLloyd->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::lloyd_freeze_changed(bool b)
{
	plugin_->generation_parameters_.do_lloyd_freeze_ = b;
	this->cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::lloyd_max_iter_changed(int nb_it)
{
	plugin_->generation_parameters_.lloyd_max_iter_ = nb_it;
}

void VolumeMeshFromSurfaceDialog::lloyd_convergence_changed(double cv)
{
	plugin_->generation_parameters_.lloyd_convergence_ = cv;
}

void VolumeMeshFromSurfaceDialog::lloyd_freeze_bound_changed(double fb)
{
	plugin_->generation_parameters_.lloyd_freeze_bound_ = fb;
}

void VolumeMeshFromSurfaceDialog::perturber_changed(bool b)
{
	this->cgal_dialog_->doubleSpinBox_PerturberSliver->setDisabled(!b);
	plugin_->generation_parameters_.do_perturber_ = b;
}

void VolumeMeshFromSurfaceDialog::perturber_sliver_changed(double sb)
{
	plugin_->generation_parameters_.perturber_sliver_bound_ = sb;
}

void VolumeMeshFromSurfaceDialog::exuder_changed(bool b)
{
	this->cgal_dialog_->doubleSpinBox_ExuderSliver->setDisabled(!b);
	plugin_->generation_parameters_.do_exuder_ = b;
}

void VolumeMeshFromSurfaceDialog::exuder_sliver_changed(double sb)
{
	plugin_->generation_parameters_.exuder_sliver_bound_ = sb;
}

void VolumeMeshFromSurfaceDialog::tetgen_args_updated(QString str)
{

	plugin_->generation_parameters_.tetgen_command_line = str.toStdString();
}

void VolumeMeshFromSurfaceDialog::update_mesh_generatuion_ui()
{
	updating_ui_ = true;

	const auto& p = plugin_->generation_parameters_;
	this->tetgen_dialog_->lineEdit_tetgen_args->setText(QString::fromStdString(p.tetgen_command_line));

	this->cgal_dialog_->doubleSpinBox_CellSize->setValue(p.cell_size_);
	this->cgal_dialog_->doubleSpinBox_Radius->setValue(p.cell_radius_edge_ratio_);
	this->cgal_dialog_->doubleSpinBoxFacetAngle->setValue(p.facet_angle_);
	this->cgal_dialog_->doubleSpinBox_FacetSize->setValue(p.facet_size_);
	this->cgal_dialog_->doubleSpinBox_3FacetDistance->setValue(p.facet_distance_);

	this->cgal_dialog_->checkBox_ODT->setChecked(p.do_odt_);
	this->cgal_dialog_->checkBox_FreezeODT->setChecked(p.do_odt_freeze_);
	this->cgal_dialog_->spinBox_ODTMaxIter->setValue(p.odt_max_iter_);
	this->cgal_dialog_->doubleSpinBox_OdtConvergence->setValue(p.odt_convergence_);
	this->cgal_dialog_->doubleSpinBox_8OdtFreeze->setValue(p.odt_freeze_bound_);
	this->cgal_dialog_->doubleSpinBox_OdtConvergence->setDisabled(!p.do_odt_);
	this->cgal_dialog_->spinBox_ODTMaxIter->setDisabled(!p.do_odt_);
	this->cgal_dialog_->checkBox_FreezeODT->setDisabled(!p.do_odt_);
	this->cgal_dialog_->checkBox_ODT->setDisabled(!p.do_odt_);

	this->cgal_dialog_->checkBoxLloyd->setChecked(p.do_lloyd_);
	this->cgal_dialog_->checkBox_FreezeLloyd->setChecked(p.do_lloyd_freeze_);
	this->cgal_dialog_->spinBoxMaxITLloyd->setValue(p.lloyd_max_iter_);
	this->cgal_dialog_->doubleSpinBox_ConvergenceLloyd->setValue(p.lloyd_convergence_);
	this->cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setValue(p.lloyd_freeze_bound_);
	this->cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!p.do_lloyd_);
	this->cgal_dialog_->doubleSpinBox_ConvergenceLloyd->setDisabled(!p.do_lloyd_);
	this->cgal_dialog_->spinBoxMaxITLloyd->setDisabled(!p.do_lloyd_);
	this->cgal_dialog_->checkBox_FreezeLloyd->setDisabled(!p.do_lloyd_);

	this->cgal_dialog_->checkBox_Perturber->setChecked(p.do_perturber_);
	this->cgal_dialog_->doubleSpinBox_PerturberSliver->setValue(p.perturber_sliver_bound_);
	this->cgal_dialog_->doubleSpinBox_PerturberSliver->setDisabled(!p.do_perturber_);

	this->cgal_dialog_->checkBox_5Exuder->setChecked(p.do_exuder_);
	this->cgal_dialog_->doubleSpinBox_ExuderSliver->setValue(p.exuder_sliver_bound_);
	this->cgal_dialog_->doubleSpinBox_ExuderSliver->setDisabled(!p.do_exuder_);

	updating_ui_ = false;
}


} // namespace plugin_vmfs
} // namespace schnapps
