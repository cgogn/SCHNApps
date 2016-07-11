/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Export                                                                *
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

#include "export_dialog.h"
#include "export.h"
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <QFileDialog>

namespace schnapps
{

namespace plugin_export
{

ExportDialog::ExportDialog(SCHNApps* s, Plugin_Export* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);
	schnapps_->foreach_map([&](MapHandlerGen* mhg)
	{
		this->comboBoxMapSelection->addItem(mhg->get_name());
	});

	connect(this->comboBoxMapSelection, SIGNAL(currentIndexChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(this->comboBoxPositionSelection, SIGNAL(currentIndexChanged(QString)), this, SLOT(position_att_changed(QString)));
	connect(this->pushButtonOutputSelection, SIGNAL(pressed()),this, SLOT(choose_file()));
	connect(this->checkBoxBinary, SIGNAL(toggled(bool)), this, SLOT(binary_option_changed(bool)));
	connect(this->checkBoxCompress, SIGNAL(toggled(bool)), this, SLOT(compress_option_changed(bool)));
	connect(this->lineEditOutput, SIGNAL(textEdited(QString)), this, SLOT(output_file_changed(QString)));
	connect(this->buttonBox,SIGNAL(rejected()), this, SLOT(reinit()));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(export_validated()));

	connect(this->listWidgetCellAttributes,SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(cell_attribute_changed(QListWidgetItem*)));
	connect(this->listWidgetVertexAttributes, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(vertex_attribute_changed(QListWidgetItem*)));

	this->checkBoxBinary->setChecked(p->export_params_.binary_);
	this->checkBoxCompress->setChecked(p->export_params_.compress_);

}

void ExportDialog::selected_map_changed(QString map_name)
{
	this->comboBoxPositionSelection->clear();
	this->comboBoxPositionSelection->addItem("-select attribute-");
	this->listWidgetVertexAttributes->clear();
	this->listWidgetCellAttributes->clear();

	if (MapHandlerGen* mhg = schnapps_->get_map(map_name))
	{
		plugin_->export_params_.map_name_ = map_name.toStdString();
		const auto& vert_att_cont = mhg->const_attribute_container(CellType::Vertex_Cell);
		for (const auto& att_name : vert_att_cont.names())
		{
			this->comboBoxPositionSelection->addItem(QString::fromStdString(att_name));
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), this->listWidgetVertexAttributes);
			item->setCheckState(Qt::Unchecked);
		}

		const CellType cell_type = mhg->dimension() == 2u ? CellType::Face_Cell : CellType::Volume_Cell;
		const auto& cell_att_cont = mhg->const_attribute_container(cell_type);
		for (const auto& att_name : cell_att_cont.names())
		{
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), this->listWidgetCellAttributes);
			item->setCheckState(Qt::Unchecked);
		}
	} else {
		this->comboBoxPositionSelection->clear();
		this->comboBoxPositionSelection->addItem("-select attribute-");
		plugin_->export_params_.map_name_ = std::string();
	}
}

void ExportDialog::position_att_changed(QString pos_name)
{
	if (!pos_name.isEmpty())
	{
		plugin_->export_params_.position_attribute_name_= pos_name.toStdString();
		for (int i = 0; i < this->listWidgetVertexAttributes->count(); ++i)
		{
			QListWidgetItem* item = this->listWidgetVertexAttributes->item(i);
			if (item->text() == pos_name)
			{
				item->setCheckState(Qt::Unchecked);
				item->setHidden(true);
			} else
				item->setHidden(false);
		}
	}
}

void ExportDialog::output_file_changed(QString output)
{
		plugin_->export_params_.output_ = output.toStdString();
}

void ExportDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg)
		this->comboBoxMapSelection->addItem(mhg->get_name());
}

void ExportDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg)
		this->comboBoxMapSelection->removeItem(this->comboBoxMapSelection->findText(mhg->get_name()));
}

void ExportDialog::choose_file()
{
		QString filename = QFileDialog::getSaveFileName(nullptr, "file name", schnapps_->get_app_path(), "Surface Mesh Files (*.ply *.off *.stl *.vtk *.vtp *.obj);; Volume Mesh Files (*.vtk *.vtu *.tet *.nas)");
		if (!filename.isEmpty())
		{
			this->lineEditOutput->setText(filename);
			plugin_->export_params_.output_ = filename.toStdString();
		} else
			this->lineEditOutput->setText("-select output-");
}

void ExportDialog::binary_option_changed(bool b)
{
	plugin_->export_params_.binary_ = b;
}

void ExportDialog::compress_option_changed(bool b)
{
	plugin_->export_params_.compress_ = b;
}

void ExportDialog::reinit()
{
	ExportParams& p = plugin_->export_params_;
	p.reset();
	this->checkBoxBinary->setChecked(p.binary_);
	this->checkBoxCompress->setChecked(p.compress_);
	this->comboBoxMapSelection->setCurrentIndex(0);
	this->comboBoxPositionSelection->clear();
	this->comboBoxPositionSelection->addItem("-select attribute-");
	this->lineEditOutput->setText("-select output-");
}

void ExportDialog::export_validated()
{
	plugin_->export_mesh();
	reinit();
}

void ExportDialog::vertex_attribute_changed(QListWidgetItem* item)
{
	auto& vertex_atts = plugin_->export_params_.other_exported_attributes_[CellType::Vertex_Cell];
	auto it = std::find(vertex_atts.begin(), vertex_atts.end(), item->text().toStdString());
	if (item->checkState() == Qt::Checked)
	{
		if (it == vertex_atts.end())
			vertex_atts.push_back(item->text().toStdString());
	} else {
		if (it != vertex_atts.end())
			vertex_atts.erase(it);
	}
}

void ExportDialog::cell_attribute_changed(QListWidgetItem* item)
{
	MapHandlerGen* mhg = schnapps_->get_map(QString::fromStdString(plugin_->export_params_.map_name_));
	if (!mhg)
		return;
	const CellType cell_type = mhg->dimension() == 2u ? CellType::Face_Cell : CellType::Volume_Cell;
	auto& cell_atts = plugin_->export_params_.other_exported_attributes_[cell_type];
	auto it = std::find(cell_atts.begin(), cell_atts.end(), item->text().toStdString());

	if (item->checkState() == Qt::Checked)
	{
		if (it == cell_atts.end())
			cell_atts.push_back(item->text().toStdString());
	} else {
		if (it != cell_atts.end())
			cell_atts.erase(it);
	}
}

} // namespace plugin_export
} // namespace schnapps
