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

#include <schnapps/plugins/export/export_dialog.h>
#include <schnapps/plugins/export/export.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/io/io_utils.h>

#include <QFileDialog>

namespace schnapps
{

namespace plugin_export
{

ExportDialog::ExportDialog(SCHNApps* s, Plugin_Export* p) :
	schnapps_(s),
	plugin_(p),
	selected_map_(nullptr)
{
	setupUi(this);

	connect(combo_mapSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(selected_map_changed(const QString&)));
	connect(combo_positionSelection, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(position_attribute_changed(const QString&)));
	connect(button_outputSelection, SIGNAL(pressed()),this, SLOT(choose_file()));
	connect(buttonBox,SIGNAL(accepted()), this, SLOT(export_validated()));

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));

	schnapps_->foreach_map([&] (MapHandlerGen* mhg)
	{
		map_added(mhg);
	});
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void ExportDialog::selected_map_changed(const QString& map_name)
{
	combo_positionSelection->clear();
	combo_positionSelection->addItem("- select attribute -");
	list_vertexAttributes->clear();
	list_cellAttributes->clear();

	selected_map_ = schnapps_->get_map(map_name);

	if (selected_map_)
	{
		const auto* vert_att_cont = selected_map_->attribute_container(CellType::Vertex_Cell);
		for (const auto& att_name : vert_att_cont->names())
		{
			combo_positionSelection->addItem(QString::fromStdString(att_name));
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), list_vertexAttributes);
			item->setCheckState(Qt::Unchecked);
		}

		const CellType cell_type = selected_map_->dimension() == 2u ? CellType::Face_Cell : CellType::Volume_Cell;
		const auto* cell_att_cont = selected_map_->attribute_container(cell_type);
		for (const auto& att_name : cell_att_cont->names())
		{
			QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), list_cellAttributes);
			item->setCheckState(Qt::Unchecked);
		}
	}
}

void ExportDialog::position_attribute_changed(const QString& pos_name)
{
	if (!pos_name.isEmpty())
	{
		if (selected_map_)
		{
			for (uint32 i = 0; i < list_vertexAttributes->count(); ++i)
			{
				QListWidgetItem* item = this->list_vertexAttributes->item(i);
				if (item->text() == pos_name)
				{
					item->setCheckState(Qt::Unchecked);
					item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
				}
				else
					item->setFlags(item->flags() | Qt::ItemIsEnabled);
			}
		}
	}
}

void ExportDialog::choose_file()
{
	QString filename = QFileDialog::getSaveFileName(nullptr, "file name", schnapps_->get_app_path(), "Surface Mesh Files (*.ply *.off *.stl *.vtk *.vtp *.obj);; Volume Mesh Files (*.vtk *.vtu *.tet *.nas)");
	if (!filename.isEmpty())
		lineEdit_output->setText(filename);
	else
		lineEdit_output->setText("- select output -");
}

void ExportDialog::export_validated()
{
	if (selected_map_)
	{
		std::cout << "filename : " << lineEdit_output->text().toStdString() << std::endl;

		cgogn::io::ExportOptions export_params = cgogn::io::ExportOptions::create()
			.filename(lineEdit_output->text().toStdString())
			.binary(check_binary->isChecked())
			.compress(check_compress->isChecked());

		const cgogn::Orbit vertex_orbit = selected_map_->orbit(CellType::Vertex_Cell);
		export_params.position_attribute(vertex_orbit, combo_positionSelection->currentText().toStdString());
		for (uint32 i = 0; i < list_vertexAttributes->count(); ++i)
		{
			QListWidgetItem* item = list_vertexAttributes->item(i);
			export_params.add_attribute(vertex_orbit, item->text().toStdString());
		}

		const cgogn::Orbit cell_orbit = selected_map_->orbit(selected_map_->dimension() == 2u ? CellType::Face_Cell : CellType::Volume_Cell);
		for (uint32 i = 0; i < list_cellAttributes->count(); ++i)
		{
			QListWidgetItem* item = list_cellAttributes->item(i);
			export_params.add_attribute(cell_orbit, item->text().toStdString());
		}

		plugin_->export_mesh(selected_map_, export_params);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void ExportDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg)
		combo_mapSelection->addItem(mhg->get_name());
}

void ExportDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg)
	{
		if (mhg == selected_map_)
			selected_map_ = nullptr;
		combo_mapSelection->removeItem(combo_mapSelection->findText(mhg->get_name()));
	}
}

} // namespace plugin_export

} // namespace schnapps
