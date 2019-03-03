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
#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>

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

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(map_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(map_removed(Object*)));

	schnapps_->foreach_object([&] (Object* mhg)
	{
		map_added(mhg);
	});
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void ExportDialog::selected_map_changed(const QString& map_name)
{
	selected_map_ = nullptr;
	combo_positionSelection->clear();
	combo_positionSelection->addItem("- select attribute -");
	list_vertexAttributes->clear();
	list_cellAttributes->clear();

	auto m2 = plugin_->map_provider()->cmap2(map_name);
	auto m3 = plugin_->map_provider()->cmap3(map_name);
	if (m2)
	{
		selected_map_ = m2;
		selected_map_changed(m2);
	} else {
		if (m3)
		{
			selected_map_ = m3;
			selected_map_changed(m3);
		}
	}
}

void ExportDialog::selected_map_changed(const plugin_cmap_provider::CMap2Handler* h)
{
	if (!h)
		return;

	const CMap2& m = *h->map();
	const auto& vert_att_cont = m.attribute_container<CMap2::Vertex::ORBIT>();
	for (const auto& att_name : vert_att_cont.names())
	{
		combo_positionSelection->addItem(QString::fromStdString(att_name));
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), list_vertexAttributes);
		item->setCheckState(Qt::Unchecked);
	}

	const auto& cell_att_cont = m.attribute_container<CMap2::Face::ORBIT>();
	for (const auto& att_name : cell_att_cont.names())
	{
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), list_cellAttributes);
		item->setCheckState(Qt::Unchecked);
	}
}

void ExportDialog::selected_map_changed(const plugin_cmap_provider::CMap3Handler* h)
{
	if (!h)
		return;

	const CMap3& m = *h->map();
	const auto& vert_att_cont = m.attribute_container<CMap3::Vertex::ORBIT>();
	for (const auto& att_name : vert_att_cont.names())
	{
		combo_positionSelection->addItem(QString::fromStdString(att_name));
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), list_vertexAttributes);
		item->setCheckState(Qt::Unchecked);
	}

	const auto& cell_att_cont = m.attribute_container<CMap3::Volume::ORBIT>();
	for (const auto& att_name : cell_att_cont.names())
	{
		QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(att_name), list_cellAttributes);
		item->setCheckState(Qt::Unchecked);
	}
}

void ExportDialog::position_attribute_changed(const QString& pos_name)
{
	if (!pos_name.isEmpty())
	{
		if (selected_map_)
		{
			for (int32 i = 0; i < list_vertexAttributes->count(); ++i)
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
	if (!selected_map_)
	{
		cgogn_log_warning("plugin_export") << "Unable to chose a filename when no map is selected";
		return;
	}

	QString extensions;
	if (dynamic_cast<plugin_cmap_provider::CMap0Handler*>(selected_map_))
		extensions = "Point set mesh Files ("+ QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::point_set_file_type_map, " ")) +")";
	if (dynamic_cast<plugin_cmap_provider::CMap1Handler*>(selected_map_))
		extensions = "Polyline mesh Files ("+ QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::polyline_file_type_map, " ")) +")";
	if (dynamic_cast<plugin_cmap_provider::CMap2Handler*>(selected_map_))
		extensions = "Surface mesh Files ("+ QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::surface_file_type_map, " ")) +")";
	if (dynamic_cast<plugin_cmap_provider::CMap3Handler*>(selected_map_))
		extensions = "Volume mesh Files ("+ QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::volume_file_type_map, " ")) +")";
	if (dynamic_cast<plugin_cmap_provider::UndirectedGraphHandler*>(selected_map_))
		extensions = "Graph Files ("+ QString::fromStdString(cgogn::io::file_type_filter(cgogn::io::graph_file_type_map, " ")) +")";

	QString filename = QFileDialog::getSaveFileName(nullptr, "Filename", schnapps_->app_path(), extensions);
	if (!filename.isEmpty())
		lineEdit_output->setText(filename);
	else
		lineEdit_output->setText("- select output -");
}

void ExportDialog::export_validated()
{
	if (selected_map_)
	{
		cgogn_log_info("plugin_export") << "Exporting to \"" << lineEdit_output->text().toStdString() << "\".";

		cgogn::io::ExportOptions export_params = cgogn::io::ExportOptions::create()
			.filename(lineEdit_output->text().toStdString())
			.binary(check_binary->isChecked())
			.compress(check_compress->isChecked());

		if (auto m2 = dynamic_cast<plugin_cmap_provider::CMap2Handler*>(selected_map_))
			export_map(m2, export_params);
		if (auto m3 = dynamic_cast<plugin_cmap_provider::CMap3Handler*>(selected_map_))
			export_map(m3, export_params);
	}
}

void ExportDialog::export_map(plugin_cmap_provider::CMap2Handler* h, cgogn::io::ExportOptions& opt)
{
	if (!h)
		return;

	opt.position_attribute(CMap2::Vertex::ORBIT, combo_positionSelection->currentText().toStdString());
	for (int32 i = 0; i < list_vertexAttributes->count(); ++i)
	{
		QListWidgetItem* item = list_vertexAttributes->item(i);
		if (item->flags() & Qt::ItemIsEnabled)
			opt.add_attribute(CMap2::Vertex::ORBIT, item->text().toStdString());
	}

	for (int32 i = 0; i < list_cellAttributes->count(); ++i)
	{
		QListWidgetItem* item = list_cellAttributes->item(i);
		opt.add_attribute(CMap2::Face::ORBIT, item->text().toStdString());
	}
	plugin_->export_mesh(h, opt);
}

void ExportDialog::export_map(plugin_cmap_provider::CMap3Handler* h, cgogn::io::ExportOptions& opt)
{
	if (!h)
		return;

	opt.position_attribute(CMap3::Vertex::ORBIT, combo_positionSelection->currentText().toStdString());
	for (int32 i = 0; i < list_vertexAttributes->count(); ++i)
	{
		QListWidgetItem* item = list_vertexAttributes->item(i);
		if (item->flags() & Qt::ItemIsEnabled)
			opt.add_attribute(CMap3::Vertex::ORBIT, item->text().toStdString());
	}

	for (int32 i = 0; i < list_cellAttributes->count(); ++i)
	{
		QListWidgetItem* item = list_cellAttributes->item(i);
		opt.add_attribute(CMap3::Volume::ORBIT, item->text().toStdString());
	}
	plugin_->export_mesh(h, opt);
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void ExportDialog::map_added(Object* mhg)
{
	if (mhg && (dynamic_cast<plugin_cmap_provider::CMap2Handler*>(mhg) || dynamic_cast<plugin_cmap_provider::CMap3Handler*>(mhg)))
		combo_mapSelection->addItem(mhg->name());
}

void ExportDialog::map_removed(Object* mhg)
{
	if (mhg && (dynamic_cast<plugin_cmap_provider::CMap2Handler*>(mhg) || dynamic_cast<plugin_cmap_provider::CMap3Handler*>(mhg)))
	{
		if (mhg == selected_map_)
			selected_map_ = nullptr;
		combo_mapSelection->removeItem(combo_mapSelection->findText(mhg->name()));
	}
}

} // namespace plugin_export

} // namespace schnapps
