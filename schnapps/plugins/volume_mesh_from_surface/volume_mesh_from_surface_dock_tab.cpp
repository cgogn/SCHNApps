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

#include <volume_mesh_from_surface.h>
#include <volume_mesh_from_surface_dock_tab.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

VolumeMeshFromSurface_DockTab::VolumeMeshFromSurface_DockTab(SCHNApps* s, Plugin_VolumeMeshFromSurface* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
	connect(this->pushButton_gen_volume_meshTetgen,SIGNAL(pressed()), plugin_, SLOT(generate_button_tetgen_pressed()));
	connect(this->pushButtonGenMeshCGAL, SIGNAL(pressed()), plugin_, SLOT(generate_button_cgal_pressed()));
	connect(this->lineEdit_tetgen_args, SIGNAL(textChanged(QString)), plugin_, SLOT(tetgen_args_updated(QString)));


	connect(this->doubleSpinBox_CellSize, SIGNAL(valueChanged(double)), this, SLOT(cell_size_changed(double)));
	connect(this->doubleSpinBox_Radius, SIGNAL(valueChanged(double)), this, SLOT(cell_radius_edge_ratio_changed(double)));
	connect(this->doubleSpinBoxFacetAngle, SIGNAL(valueChanged(double)), this, SLOT(facet_angle_changed(double)));
	connect(this->doubleSpinBox_FacetSize, SIGNAL(valueChanged(double)), this, SLOT(facet_size_changed(double)));
	connect(this->doubleSpinBox_3FacetDistance, SIGNAL(valueChanged(double)), this, SLOT(facet_distance_changed(double)));

	connect(this->checkBox_ODT, SIGNAL(toggled(bool)), this, SLOT(odt_changed(bool)));
	connect(this->checkBox_FreezeODT, SIGNAL(toggled(bool)), this, SLOT(odt_freeze_changed(bool)));
	connect(this->spinBox_ODTMaxIter, SIGNAL(valueChanged(int)), this, SLOT(odt_max_iter_changed(int)));
	connect(this->doubleSpinBox_OdtConvergence, SIGNAL(valueChanged(double)), this, SLOT(odt_convergence_changed(double)));
	connect(this->doubleSpinBox_8OdtFreeze, SIGNAL(valueChanged(double)), this, SLOT(odt_freeze_bound_changed(double)));

	connect(this->checkBoxLloyd, SIGNAL(toggled(bool)), this, SLOT(lloyd_changed(bool)));
	connect(this->checkBox_FreezeLloyd, SIGNAL(toggled(bool)), this, SLOT(lloyd_freeze_changed(bool)));
	connect(this->spinBoxMaxITLloyd, SIGNAL(valueChanged(int)), this, SLOT(lloyd_max_iter_changed(int)));
	connect(this->doubleSpinBox_ConvergenceLloyd, SIGNAL(valueChanged(double)), this, SLOT(lloyd_convergence_changed(double)));
	connect(this->doubleSpinBox_FreezeBoundLLoyd, SIGNAL(valueChanged(double)), this, SLOT(lloyd_freeze_bound_changed(double)));

	connect(this->checkBox_Perturber, SIGNAL(toggled(bool)), this, SLOT(perturber_changed(bool)));
	connect(this->doubleSpinBox_PerturberSliver, SIGNAL(valueChanged(double)), this, SLOT(perturber_sliver_changed(double)));

	connect(this->checkBox_5Exuder, SIGNAL(toggled(bool)), this, SLOT(exuder_changed(bool)));
	connect(this->doubleSpinBox_ExuderSliver, SIGNAL(valueChanged(double)), this, SLOT(exuder_sliver_changed(double)));

	this->selected_map_changed(nullptr, schnapps_->get_selected_map());
}

void VolumeMeshFromSurface_DockTab::selected_map_changed(MapHandlerGen*, MapHandlerGen* curr)
{
	if (curr && dynamic_cast<const CMap2*>(curr->get_map()))
	{
		this->pushButton_gen_volume_meshTetgen->setDisabled(false);
		this->pushButtonGenMeshCGAL->setDisabled(false);
		this->lineEdit_tetgen_args->setDisabled(false);
	} else {
		this->pushButton_gen_volume_meshTetgen->setDisabled(true);
		this->pushButtonGenMeshCGAL->setDisabled(true);
		this->lineEdit_tetgen_args->setDisabled(true);
	}
	update_map_parameters(curr, plugin_->parameter_set_[curr]);
}

void VolumeMeshFromSurface_DockTab::cell_size_changed(double cs)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].cell_size_ = cs;
}

void VolumeMeshFromSurface_DockTab::cell_radius_edge_ratio_changed(double ratio)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].cell_radius_edge_ratio_ = ratio;
}

void VolumeMeshFromSurface_DockTab::facet_angle_changed(double fa)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].facet_angle_ = fa;
}

void VolumeMeshFromSurface_DockTab::facet_size_changed(double fs)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].facet_size_ = fs;
}

void VolumeMeshFromSurface_DockTab::facet_distance_changed(double fd)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].facet_distance_ = fd;
}

void VolumeMeshFromSurface_DockTab::odt_changed(bool b)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].do_odt_ = b;
	this->doubleSpinBox_OdtConvergence->setDisabled(!b);
	this->spinBox_ODTMaxIter->setDisabled(!b);
	this->checkBox_FreezeODT->setDisabled(!b);
	this->doubleSpinBox_8OdtFreeze->setDisabled(!b);

}

void VolumeMeshFromSurface_DockTab::odt_freeze_changed(bool b)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].do_odt_freeze_ = b;
	this->doubleSpinBox_8OdtFreeze->setDisabled(!b);
}

void VolumeMeshFromSurface_DockTab::odt_max_iter_changed(int nb_it)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].odt_max_iter_ = nb_it;
}

void VolumeMeshFromSurface_DockTab::odt_convergence_changed(double cv)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].odt_convergence_ = cv;
}

void VolumeMeshFromSurface_DockTab::odt_freeze_bound_changed(double fb)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].odt_freeze_bound_ = fb;
}

void VolumeMeshFromSurface_DockTab::lloyd_changed(bool b)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].do_lloyd_ = b;
	this->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!b);
	this->doubleSpinBox_ConvergenceLloyd->setDisabled(!b);
	this->spinBoxMaxITLloyd->setDisabled(!b);
	this->checkBox_FreezeLloyd->setDisabled(!b);
}

void VolumeMeshFromSurface_DockTab::lloyd_freeze_changed(bool b)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].do_lloyd_freeze_ = b;
	this->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!b);
}

void VolumeMeshFromSurface_DockTab::lloyd_max_iter_changed(int nb_it)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].lloyd_max_iter_ = nb_it;
}

void VolumeMeshFromSurface_DockTab::lloyd_convergence_changed(double cv)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].lloyd_convergence_ = cv;
}

void VolumeMeshFromSurface_DockTab::lloyd_freeze_bound_changed(double fb)
{
	plugin_->parameter_set_[schnapps_->get_selected_map()].lloyd_freeze_bound_ = fb;
}

void VolumeMeshFromSurface_DockTab::perturber_changed(bool b)
{
	this->doubleSpinBox_PerturberSliver->setDisabled(!b);
	plugin_->parameter_set_[schnapps_->get_selected_map()].do_perturber_ = b;
}

void VolumeMeshFromSurface_DockTab::perturber_sliver_changed(double sb)
{
	this->doubleSpinBox_PerturberSliver->setValue(sb);
}

void VolumeMeshFromSurface_DockTab::exuder_changed(bool b)
{
	this->doubleSpinBox_ExuderSliver->setDisabled(!b);
	plugin_->parameter_set_[schnapps_->get_selected_map()].do_exuder_ = b;
}

void VolumeMeshFromSurface_DockTab::exuder_sliver_changed(double sb)
{
	this->doubleSpinBox_ExuderSliver->setValue(sb);
}

void VolumeMeshFromSurface_DockTab::update_map_parameters(MapHandlerGen* map, const MapParameters& p)
{
	updating_ui_ = true;
	if (map && dynamic_cast<MapHandler<CMap2>*>(map))
	{
		plugin_->tetgen_args_updated(QString::fromStdString(p.tetgen_command_line));

		this->doubleSpinBox_CellSize->setValue(p.cell_size_);
		this->doubleSpinBox_Radius->setValue(p.cell_radius_edge_ratio_);
		this->doubleSpinBoxFacetAngle->setValue(p.facet_angle_);
		this->doubleSpinBox_FacetSize->setValue(p.facet_size_);
		this->doubleSpinBox_3FacetDistance->setValue(p.facet_distance_);

		this->checkBox_ODT->setChecked(p.do_odt_);
		this->checkBox_FreezeODT->setChecked(p.do_odt_freeze_);
		this->spinBox_ODTMaxIter->setValue(p.odt_max_iter_);
		this->doubleSpinBox_OdtConvergence->setValue(p.odt_convergence_);
		this->doubleSpinBox_8OdtFreeze->setValue(p.odt_freeze_bound_);
		this->doubleSpinBox_OdtConvergence->setDisabled(!p.do_odt_);
		this->spinBox_ODTMaxIter->setDisabled(!p.do_odt_);
		this->checkBox_FreezeODT->setDisabled(!p.do_odt_);
		this->checkBox_ODT->setDisabled(!p.do_odt_);

		this->checkBoxLloyd->setChecked(p.do_lloyd_);
		this->checkBox_FreezeLloyd->setChecked(p.do_lloyd_freeze_);
		this->spinBoxMaxITLloyd->setValue(p.lloyd_max_iter_);
		this->doubleSpinBox_ConvergenceLloyd->setValue(p.lloyd_convergence_);
		this->doubleSpinBox_FreezeBoundLLoyd->setValue(p.lloyd_freeze_bound_);
		this->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!p.do_lloyd_);
		this->doubleSpinBox_ConvergenceLloyd->setDisabled(!p.do_lloyd_);
		this->spinBoxMaxITLloyd->setDisabled(!p.do_lloyd_);
		this->checkBox_FreezeLloyd->setDisabled(!p.do_lloyd_);

		this->checkBox_Perturber->setChecked(p.do_perturber_);
		this->doubleSpinBox_PerturberSliver->setValue(p.perturber_sliver_bound_);
		this->doubleSpinBox_PerturberSliver->setDisabled(!p.do_perturber_);

		this->checkBox_5Exuder->setChecked(p.do_exuder_);
		this->doubleSpinBox_ExuderSliver->setValue(p.exuder_sliver_bound_);
		this->doubleSpinBox_ExuderSliver->setDisabled(!p.do_exuder_);
	}
	updating_ui_ = false;
}

} // namespace schnapps
