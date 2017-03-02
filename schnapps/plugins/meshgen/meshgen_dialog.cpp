/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin MeshGen                                                               *
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

#include <meshgen.h>
#include <meshgen_dialog.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include "image.h"

namespace schnapps
{

namespace plugin_meshgen
{

VolumeMeshFromSurfaceDialog::VolumeMeshFromSurfaceDialog(SCHNApps* s, Plugin_VolumeMeshFromSurface* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	export_dialog_ = cgogn::make_unique<ExportDialog>();
	cgal_dialog_ = cgogn::make_unique<ExportCGALDialog>();
	netgen_dialog_ = cgogn::make_unique<ExportNetgenDialog>();
	tetgen_dialog_ = cgogn::make_unique<ExportTetgenDialog>();

	export_dialog_->setupUi(export_dialog_.get());
	cgal_dialog_->setupUi(cgal_dialog_.get());
	netgen_dialog_->setupUi(netgen_dialog_.get());
	tetgen_dialog_->setupUi(tetgen_dialog_.get());

	connect(export_dialog_->comboBoxMapSelection, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(export_dialog_->comboBox_generator, SIGNAL(currentIndexChanged(QString)), this, SLOT(generator_changed(QString)));

	connect(netgen_dialog_->buttonBox,SIGNAL(accepted()), plugin_, SLOT(generate_button_netgen_pressed()));
	connect(tetgen_dialog_->buttonBox,SIGNAL(accepted()), plugin_, SLOT(generate_button_tetgen_pressed()));
	connect(cgal_dialog_->buttonBox, SIGNAL(accepted()), plugin_, SLOT(generate_button_cgal_pressed()));
	connect(tetgen_dialog_->lineEdit_tetgen_args, SIGNAL(textChanged(QString)), this, SLOT(tetgen_args_updated(QString)));

	connect(cgal_dialog_->doubleSpinBox_CellSize, SIGNAL(valueChanged(double)), this, SLOT(cell_size_changed(double)));
	connect(cgal_dialog_->doubleSpinBox_Radius, SIGNAL(valueChanged(double)), this, SLOT(cell_radius_edge_ratio_changed(double)));
	connect(cgal_dialog_->doubleSpinBoxFacetAngle, SIGNAL(valueChanged(double)), this, SLOT(facet_angle_changed(double)));
	connect(cgal_dialog_->doubleSpinBox_FacetSize, SIGNAL(valueChanged(double)), this, SLOT(facet_size_changed(double)));
	connect(cgal_dialog_->doubleSpinBox_3FacetDistance, SIGNAL(valueChanged(double)), this, SLOT(facet_distance_changed(double)));

	connect(cgal_dialog_->checkBox_ODT, SIGNAL(toggled(bool)), this, SLOT(odt_changed(bool)));
	connect(cgal_dialog_->checkBox_FreezeODT, SIGNAL(toggled(bool)), this, SLOT(odt_freeze_changed(bool)));
	connect(cgal_dialog_->spinBox_ODTMaxIter, SIGNAL(valueChanged(int)), this, SLOT(odt_max_iter_changed(int)));
	connect(cgal_dialog_->doubleSpinBox_OdtConvergence, SIGNAL(valueChanged(double)), this, SLOT(odt_convergence_changed(double)));
	connect(cgal_dialog_->doubleSpinBox_8OdtFreeze, SIGNAL(valueChanged(double)), this, SLOT(odt_freeze_bound_changed(double)));

	connect(cgal_dialog_->checkBoxLloyd, SIGNAL(toggled(bool)), this, SLOT(lloyd_changed(bool)));
	connect(cgal_dialog_->checkBox_FreezeLloyd, SIGNAL(toggled(bool)), this, SLOT(lloyd_freeze_changed(bool)));
	connect(cgal_dialog_->spinBoxMaxITLloyd, SIGNAL(valueChanged(int)), this, SLOT(lloyd_max_iter_changed(int)));
	connect(cgal_dialog_->doubleSpinBox_ConvergenceLloyd, SIGNAL(valueChanged(double)), this, SLOT(lloyd_convergence_changed(double)));
	connect(cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd, SIGNAL(valueChanged(double)), this, SLOT(lloyd_freeze_bound_changed(double)));

	connect(cgal_dialog_->checkBox_Perturber, SIGNAL(toggled(bool)), this, SLOT(perturber_changed(bool)));
	connect(cgal_dialog_->doubleSpinBox_PerturberSliver, SIGNAL(valueChanged(double)), this, SLOT(perturber_sliver_changed(double)));

	connect(cgal_dialog_->checkBox_5Exuder, SIGNAL(toggled(bool)), this, SLOT(exuder_changed(bool)));
	connect(cgal_dialog_->doubleSpinBox_ExuderSliver, SIGNAL(valueChanged(double)), this, SLOT(exuder_sliver_changed(double)));

	ExportNetgenDialog* netgen_dial = netgen_dialog_.get();
	connect(netgen_dial->checkBox_uselocalh, SIGNAL(toggled(bool)), this, SLOT(netgen_uselocalh_toggled(bool)));
	connect(netgen_dial->doubleSpinBox_maxh, SIGNAL(valueChanged(double)), this, SLOT(netgen_maxh_changed(double)));
	connect(netgen_dial->doubleSpinBox_minh, SIGNAL(valueChanged(double)), this, SLOT(netgen_minh_changed(double)));
	connect(netgen_dial->doubleSpinBox_fineness, SIGNAL(valueChanged(double)), this, SLOT(netgen_fineness_changed(double)));
	connect(netgen_dial->doubleSpinBox_grading, SIGNAL(valueChanged(double)), this, SLOT(netgen_grading_changed(double)));
	connect(netgen_dial->doubleSpinBox_elementsperedge, SIGNAL(valueChanged(double)), this, SLOT(netgen_elemsperedge_changed(double)));
	connect(netgen_dial->doubleSpinBox_elementspercurve, SIGNAL(valueChanged(double)), this, SLOT(netgen_elemspercurve_changed(double)));
	connect(netgen_dial->checkBox_closeedgeenable, SIGNAL(toggled(bool)), this, SLOT(netgen_closeedgeenable_toggled(bool)));
	connect(netgen_dial->doubleSpinBox_closeedgefact, SIGNAL(valueChanged(double)), this, SLOT(netgen_closeedgefact_changed(double)));
	connect(netgen_dial->checkBox_minedgelenenable, SIGNAL(toggled(bool)), this, SLOT(netgen_minedgelenenable_toggled(bool)));
	connect(netgen_dial->doubleSpinBox_minedgelen, SIGNAL(valueChanged(double)), this, SLOT(netgen_minedgelen_changed(double)));
	connect(netgen_dial->checkBox_second_order, SIGNAL(toggled(bool)), this, SLOT(netgen_second_order_toggled(bool)));
	connect(netgen_dial->checkBox_quad_dominated, SIGNAL(toggled(bool)), this, SLOT(netgen_quad_dominated_toggled(bool)));
	connect(netgen_dial->spinBox_optsteps_3d, SIGNAL(valueChanged(int)), this, SLOT(netgen_optsteps_3d_changed(int)));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	connect(export_dialog_->comboBox_images, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_image_changed(QString)));

	update_mesh_generatuion_ui();
}

void VolumeMeshFromSurfaceDialog::show_export_dialog()
{
	export_dialog_->show();
}

void VolumeMeshFromSurfaceDialog::generator_changed(const QString& generator)
{
	if (generator == "cgal")
	{
		cgal_dialog_->show();
		tetgen_dialog_->hide();
		netgen_dialog_->hide();
	} else {
		if (generator == "tetgen")
		{
			tetgen_dialog_->show();
			netgen_dialog_->hide();
			cgal_dialog_->hide();
		} else {
			if (generator =="netgen")
			{
				netgen_dialog_->show();
				tetgen_dialog_->hide();
				cgal_dialog_->hide();
			}
		}
	}
	const int old_idx = export_dialog_->comboBox_generator->currentIndex();
	export_dialog_->comboBox_generator->setCurrentIndex(0);
	if (old_idx != 0)
		export_dialog_->hide();
}

void VolumeMeshFromSurfaceDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg && dynamic_cast<CMap2Handler*>(mhg))
		export_dialog_->comboBoxMapSelection->addItem(mhg->get_name());
}

void VolumeMeshFromSurfaceDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg && dynamic_cast<CMap2Handler*>(mhg))
		export_dialog_->comboBoxMapSelection->removeItem(export_dialog_->comboBoxMapSelection->findText(mhg->get_name()));
}

void VolumeMeshFromSurfaceDialog::image_added(QString im_path)
{
	export_dialog_->comboBox_images->addItem(im_path);
}

void VolumeMeshFromSurfaceDialog::image_removed(QString im_path)
{
	export_dialog_->comboBox_images->removeItem(export_dialog_->comboBox_images->findText(im_path));
}

QString VolumeMeshFromSurfaceDialog::get_selected_map() const
{
	return export_dialog_->comboBoxMapSelection->currentText();
}

void VolumeMeshFromSurfaceDialog::selected_map_changed(QString map_name)
{
	MapHandlerGen* mhg = schnapps_->get_map(map_name);
	QSignalBlocker blocker(export_dialog_->comboBox_generator);
	export_dialog_->comboBox_generator->clear();
	if (mhg && dynamic_cast<CMap2Handler*>(mhg))
	{
		QStringList list;
		list << "-select-" << "cgal" << "netgen" << "tetgen";
		export_dialog_->comboBox_generator->insertItems(0, list);
		QSignalBlocker blocker(export_dialog_->comboBox_images);
		export_dialog_->comboBox_images->setCurrentIndex(0);


		export_dialog_->comboBoxPositionSelection->clear();
		const auto* vert_att_cont = mhg->const_attribute_container(CellType::Vertex_Cell);
		for (const auto& att_name : vert_att_cont->names())
			export_dialog_->comboBoxPositionSelection->addItem(QString::fromStdString(att_name));
	}
}

void VolumeMeshFromSurfaceDialog::selected_image_changed(QString /*image_name*/)
{
	QSignalBlocker blocker(export_dialog_->comboBox_generator);
	export_dialog_->comboBoxMapSelection->setCurrentIndex(0);
	export_dialog_->comboBox_generator->clear();
	if (export_dialog_->comboBox_images->currentIndex() >= 1)
	{
		QStringList list;
		list << "-select-" << "cgal";
		export_dialog_->comboBox_generator->insertItems(0, list);
	}
}

void VolumeMeshFromSurfaceDialog::cell_size_changed(double cs)
{
	plugin_->generation_parameters_.cgal.cell_size_ = cs;
}

void VolumeMeshFromSurfaceDialog::cell_radius_edge_ratio_changed(double ratio)
{
	plugin_->generation_parameters_.cgal.cell_radius_edge_ratio_ = ratio;
}

void VolumeMeshFromSurfaceDialog::facet_angle_changed(double fa)
{
	plugin_->generation_parameters_.cgal.facet_angle_ = fa;
}

void VolumeMeshFromSurfaceDialog::facet_size_changed(double fs)
{
	plugin_->generation_parameters_.cgal.facet_size_ = fs;
}

void VolumeMeshFromSurfaceDialog::facet_distance_changed(double fd)
{
	plugin_->generation_parameters_.cgal.facet_distance_ = fd;
}

void VolumeMeshFromSurfaceDialog::odt_changed(bool b)
{
	plugin_->generation_parameters_.cgal.do_odt_ = b;
	cgal_dialog_->doubleSpinBox_OdtConvergence->setDisabled(!b);
	cgal_dialog_->spinBox_ODTMaxIter->setDisabled(!b);
	cgal_dialog_->checkBox_FreezeODT->setDisabled(!b);
	cgal_dialog_->doubleSpinBox_8OdtFreeze->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::odt_freeze_changed(bool b)
{
	plugin_->generation_parameters_.cgal.do_odt_freeze_ = b;
	cgal_dialog_->doubleSpinBox_8OdtFreeze->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::odt_max_iter_changed(int nb_it)
{
	plugin_->generation_parameters_.cgal.odt_max_iter_ = nb_it;
}

void VolumeMeshFromSurfaceDialog::odt_convergence_changed(double cv)
{
	plugin_->generation_parameters_.cgal.odt_convergence_ = cv;
}

void VolumeMeshFromSurfaceDialog::odt_freeze_bound_changed(double fb)
{
	plugin_->generation_parameters_.cgal.odt_freeze_bound_ = fb;
}

void VolumeMeshFromSurfaceDialog::lloyd_changed(bool b)
{
	plugin_->generation_parameters_.cgal.do_lloyd_ = b;
	cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!b);
	cgal_dialog_->doubleSpinBox_ConvergenceLloyd->setDisabled(!b);
	cgal_dialog_->spinBoxMaxITLloyd->setDisabled(!b);
	cgal_dialog_->checkBox_FreezeLloyd->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::lloyd_freeze_changed(bool b)
{
	plugin_->generation_parameters_.cgal.do_lloyd_freeze_ = b;
	cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!b);
}

void VolumeMeshFromSurfaceDialog::lloyd_max_iter_changed(int nb_it)
{
	plugin_->generation_parameters_.cgal.lloyd_max_iter_ = nb_it;
}

void VolumeMeshFromSurfaceDialog::lloyd_convergence_changed(double cv)
{
	plugin_->generation_parameters_.cgal.lloyd_convergence_ = cv;
}

void VolumeMeshFromSurfaceDialog::lloyd_freeze_bound_changed(double fb)
{
	plugin_->generation_parameters_.cgal.lloyd_freeze_bound_ = fb;
}

void VolumeMeshFromSurfaceDialog::perturber_changed(bool b)
{
	cgal_dialog_->doubleSpinBox_PerturberSliver->setDisabled(!b);
	plugin_->generation_parameters_.cgal.do_perturber_ = b;
}

void VolumeMeshFromSurfaceDialog::perturber_sliver_changed(double sb)
{
	plugin_->generation_parameters_.cgal.perturber_sliver_bound_ = sb;
}

void VolumeMeshFromSurfaceDialog::exuder_changed(bool b)
{
	cgal_dialog_->doubleSpinBox_ExuderSliver->setDisabled(!b);
	plugin_->generation_parameters_.cgal.do_exuder_ = b;
}

void VolumeMeshFromSurfaceDialog::exuder_sliver_changed(double sb)
{
	plugin_->generation_parameters_.cgal.exuder_sliver_bound_ = sb;
}

void VolumeMeshFromSurfaceDialog::tetgen_args_updated(QString str)
{

	plugin_->generation_parameters_.tetgen.tetgen_command_line = str.toStdString();
}

void VolumeMeshFromSurfaceDialog::netgen_uselocalh_toggled(bool b)
{
	plugin_->generation_parameters_.netgen.uselocalh = b;
}

void VolumeMeshFromSurfaceDialog::netgen_maxh_changed(double d)
{
	plugin_->generation_parameters_.netgen.maxh = d;
}

void VolumeMeshFromSurfaceDialog::netgen_minh_changed(double d)
{
	plugin_->generation_parameters_.netgen.minh = d;
}

void VolumeMeshFromSurfaceDialog::netgen_fineness_changed(double d)
{
	plugin_->generation_parameters_.netgen.fineness = d;
}

void VolumeMeshFromSurfaceDialog::netgen_grading_changed(double d)
{
	plugin_->generation_parameters_.netgen.grading = d;
}

void VolumeMeshFromSurfaceDialog::netgen_elemsperedge_changed(double d)
{
	plugin_->generation_parameters_.netgen.elementsperedge = d;
}

void VolumeMeshFromSurfaceDialog::netgen_elemspercurve_changed(double d)
{
	plugin_->generation_parameters_.netgen.elementspercurve = d;
}

void VolumeMeshFromSurfaceDialog::netgen_closeedgeenable_toggled(bool b)
{
	plugin_->generation_parameters_.netgen.closeedgeenable = b;
}

void VolumeMeshFromSurfaceDialog::netgen_closeedgefact_changed(double d)
{
	plugin_->generation_parameters_.netgen.closeedgefact = d;
}

void VolumeMeshFromSurfaceDialog::netgen_minedgelenenable_toggled(bool b)
{
	plugin_->generation_parameters_.netgen.minedgelenenable = b;
}

void VolumeMeshFromSurfaceDialog::netgen_minedgelen_changed(double d)
{
	plugin_->generation_parameters_.netgen.minedgelen = d;
}

void VolumeMeshFromSurfaceDialog::netgen_second_order_toggled(bool b)
{
	plugin_->generation_parameters_.netgen.second_order = b;
}

void VolumeMeshFromSurfaceDialog::netgen_quad_dominated_toggled(bool b)
{
	plugin_->generation_parameters_.netgen.quad_dominated = b;
}

void VolumeMeshFromSurfaceDialog::netgen_optsteps_3d_changed(int i)
{
	plugin_->generation_parameters_.netgen.optsteps_3d = i;
}

void VolumeMeshFromSurfaceDialog::update_mesh_generatuion_ui()
{
	updating_ui_ = true;

	const auto& p = plugin_->generation_parameters_;
	tetgen_dialog_->lineEdit_tetgen_args->setText(QString::fromStdString(p.tetgen.tetgen_command_line));

	cgal_dialog_->doubleSpinBox_CellSize->setValue(p.cgal.cell_size_);
	cgal_dialog_->doubleSpinBox_Radius->setValue(p.cgal.cell_radius_edge_ratio_);
	cgal_dialog_->doubleSpinBoxFacetAngle->setValue(p.cgal.facet_angle_);
	cgal_dialog_->doubleSpinBox_FacetSize->setValue(p.cgal.facet_size_);
	cgal_dialog_->doubleSpinBox_3FacetDistance->setValue(p.cgal.facet_distance_);

	cgal_dialog_->checkBox_ODT->setChecked(p.cgal.do_odt_);
	cgal_dialog_->checkBox_FreezeODT->setChecked(p.cgal.do_odt_freeze_);
	cgal_dialog_->spinBox_ODTMaxIter->setValue(p.cgal.odt_max_iter_);
	cgal_dialog_->doubleSpinBox_OdtConvergence->setValue(p.cgal.odt_convergence_);
	cgal_dialog_->doubleSpinBox_8OdtFreeze->setValue(p.cgal.odt_freeze_bound_);
	cgal_dialog_->doubleSpinBox_OdtConvergence->setDisabled(!p.cgal.do_odt_);
	cgal_dialog_->spinBox_ODTMaxIter->setDisabled(!p.cgal.do_odt_);
	cgal_dialog_->checkBox_FreezeODT->setDisabled(!p.cgal.do_odt_);
	cgal_dialog_->checkBox_ODT->setDisabled(!p.cgal.do_odt_);

	cgal_dialog_->checkBoxLloyd->setChecked(p.cgal.do_lloyd_);
	cgal_dialog_->checkBox_FreezeLloyd->setChecked(p.cgal.do_lloyd_freeze_);
	cgal_dialog_->spinBoxMaxITLloyd->setValue(p.cgal.lloyd_max_iter_);
	cgal_dialog_->doubleSpinBox_ConvergenceLloyd->setValue(p.cgal.lloyd_convergence_);
	cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setValue(p.cgal.lloyd_freeze_bound_);
	cgal_dialog_->doubleSpinBox_FreezeBoundLLoyd->setDisabled(!p.cgal.do_lloyd_);
	cgal_dialog_->doubleSpinBox_ConvergenceLloyd->setDisabled(!p.cgal.do_lloyd_);
	cgal_dialog_->spinBoxMaxITLloyd->setDisabled(!p.cgal.do_lloyd_);
	cgal_dialog_->checkBox_FreezeLloyd->setDisabled(!p.cgal.do_lloyd_);

	cgal_dialog_->checkBox_Perturber->setChecked(p.cgal.do_perturber_);
	cgal_dialog_->doubleSpinBox_PerturberSliver->setValue(p.cgal.perturber_sliver_bound_);
	cgal_dialog_->doubleSpinBox_PerturberSliver->setDisabled(!p.cgal.do_perturber_);

	cgal_dialog_->checkBox_5Exuder->setChecked(p.cgal.do_exuder_);
	cgal_dialog_->doubleSpinBox_ExuderSliver->setValue(p.cgal.exuder_sliver_bound_);
	cgal_dialog_->doubleSpinBox_ExuderSliver->setDisabled(!p.cgal.do_exuder_);

	netgen_dialog_->checkBox_uselocalh->setChecked(p.netgen.uselocalh);
	netgen_dialog_->doubleSpinBox_maxh->setValue(p.netgen.maxh);
	netgen_dialog_->doubleSpinBox_minh->setValue(p.netgen.minh);
	netgen_dialog_->doubleSpinBox_fineness->setValue(p.netgen.fineness);
	netgen_dialog_->doubleSpinBox_grading->setValue(p.netgen.grading);
	netgen_dialog_->doubleSpinBox_elementsperedge->setValue(p.netgen.elementsperedge);
	netgen_dialog_->doubleSpinBox_elementspercurve->setValue(p.netgen.elementspercurve);
	netgen_dialog_->checkBox_closeedgeenable->setChecked(p.netgen.closeedgeenable);
	netgen_dialog_->doubleSpinBox_closeedgefact->setValue(p.netgen.closeedgefact);
	netgen_dialog_->checkBox_minedgelenenable->setChecked(p.netgen.minedgelenenable);
	netgen_dialog_->doubleSpinBox_minedgelen->setValue(p.netgen.minedgelen);
	netgen_dialog_->checkBox_second_order->setChecked(p.netgen.second_order);
	netgen_dialog_->checkBox_quad_dominated->setChecked(p.netgen.quad_dominated);
	netgen_dialog_->spinBox_optsteps_3d->setValue(p.netgen.optsteps_3d);

	updating_ui_ = false;
}


} // namespace plugin_meshgen
} // namespace schnapps
