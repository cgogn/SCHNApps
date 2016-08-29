/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Plugin ExtractSurface                                                        *
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

#include <dll.h>
#include <edit_attribute_dialog.h>
#include <attribute_editor.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/schnapps.h>
#include <sstream>

namespace schnapps
{

namespace plugin_attribute_editor
{

EditAttributeDialog::EditAttributeDialog(SCHNApps* s, AttributeEditorPlugin* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(map_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(orbit_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(orbit_changed(QString)));
	connect(att_name_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(attribute_changed(QString)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(edit_attribute_validated()));
	connect(this->apply_button,SIGNAL(pressed()), this, SLOT(edit_attribute_validated()));
	connect(this->cellsSet_comboBox,SIGNAL(currentTextChanged(QString)), this, SLOT(cells_set_changed(QString)));

	schnapps_->foreach_map([&](MapHandlerGen* mhg)
	{
		map_added(mhg);
	});
}

void EditAttributeDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg)
	{
		map_comboBox->addItem(mhg->get_name());
		if (map_comboBox->count() == 1)
			selected_map_changed(mhg->get_name());
	}
}

void EditAttributeDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg)
		map_comboBox->removeItem(this->map_comboBox->findText(mhg->get_name()));
}

void EditAttributeDialog::selected_map_changed(const QString& map_name)
{
	MapHandlerGen* mhg = schnapps_->get_map(map_name);
	const CellType ct = cell_type(orbit_comboBox->currentText().toStdString());
	update_attribute_list(mhg, ct);
	update_cells_sets(mhg, ct);
}

void EditAttributeDialog::orbit_changed(const QString& orbit_name)
{

	MapHandlerGen* mhg = schnapps_->get_map(map_comboBox->currentText());
	const CellType ct = cell_type(orbit_name.toStdString());
	update_attribute_list(mhg, ct);
	update_cells_sets(mhg, ct);
}

void EditAttributeDialog::cells_set_changed(const QString& cells_set_name)
{
	attribute_changed(att_name_comboBox->currentText());
}

void EditAttributeDialog::attribute_changed(const QString& attribute_name)
{
	MapHandlerGen* mhg = schnapps_->get_map(map_comboBox->currentText());
	if (mhg)
	{
		CellType cell_t = cell_type(orbit_comboBox->currentText().toStdString());
		if (cell_t != CellType::Unknown)
		{
			const auto* ca_cont = mhg->const_attribute_container(cell_t);
			if (!ca_cont || !ca_cont->has_array(attribute_name.toStdString()))
				return;
			auto* ca = ca_cont->get_chunk_array(attribute_name.toStdString());
			attribute_tableWidget->clearContents();
			attribute_tableWidget->setColumnCount(0);
			attribute_tableWidget->setRowCount(0);
			if (ca)
			{
				const uint32 nb_cols = ca->nb_components();
				attribute_tableWidget->setColumnCount(int32(nb_cols));

				int r = 0;

				auto func = [&](cgogn::Dart d)
				{
					int c = 0;
					attribute_tableWidget->insertRow(r);
					std::vector<QTableWidgetItem*> items(nb_cols + 1);
					for (auto& item : items)
						item = new QTableWidgetItem;

					const uint32 emb = mhg->embedding(d, cell_t);
					items[0]->setText(QString::number(emb));
					attribute_tableWidget->setVerticalHeaderItem(r, items[0]);

					std::stringstream sstream;
					ca->export_element(emb, sstream, false, false);
					std::string val;

					for (uint32 i = 1u; i <= nb_cols ; ++i)
					{
						sstream >> val;
						items[i]->setText(QString::fromStdString(val));
						attribute_tableWidget->setItem(r,c++, items[i]);
					}
					++r;
				};

				CellsSetGen* csg = mhg->get_cells_set(cell_t, cellsSet_comboBox->currentText());
				if (csg)
					csg->foreach_cell(func);
				else
					mhg->foreach_cell(cell_t, func);
			}
		}
	}
}

void EditAttributeDialog::edit_attribute_validated()
{
	MapHandlerGen* mhg = schnapps_->get_map(map_comboBox->currentText());
	if (mhg)
	{
		CellType cell_t = cell_type(orbit_comboBox->currentText().toStdString());
		if (cell_t != CellType::Unknown)
		{
			const QString& attribute_name = att_name_comboBox->currentText();
			const auto* ca_cont = mhg->const_attribute_container(cell_t);
			auto* ca = ca_cont->get_chunk_array(attribute_name.toStdString());
			if (ca)
			{
				int32 r = 0, rend = attribute_tableWidget->rowCount();
				const int32 nbc = attribute_tableWidget->columnCount();

				for ( ; r < rend; ++r)
				{
					const uint32 emb = attribute_tableWidget->verticalHeaderItem(r)->text().toUInt();
					std::stringstream sstream;
					for (int32 c = 0; c < nbc; ++c)
						sstream << attribute_tableWidget->item(r,c)->text().toStdString()  << " ";

					ca->import_element(emb, sstream);
					{
						sstream = std::stringstream();
						ca->export_element(emb, sstream, false, false);
						for (int32 c = 0; c < nbc; ++c)
						{
							std::string val;
							sstream >> val;
							attribute_tableWidget->item(r,c)->setText(QString::fromStdString(val));
						}
					}

				}
				mhg->notify_attribute_change(mhg->orbit(cell_t), attribute_name);
			}
		}
	}
}

void EditAttributeDialog::update_cells_sets(MapHandlerGen* mhg, CellType ct)
{
	cellsSet_comboBox->clear();
	cellsSet_comboBox->addItem("-select set-");
	if (mhg && ct != CellType::Unknown)
	{
		mhg->foreach_cells_set(ct, [&](CellsSetGen* csg)
		{
			cellsSet_comboBox->addItem(csg->get_name());
		});
	}
}

void EditAttributeDialog::update_attribute_list(MapHandlerGen* mhg, CellType ct)
{
	att_name_comboBox->clear();
	att_name_comboBox->addItem("-select attribute-");
	if (mhg && ct != CellType::Unknown)
	{
		const auto& att_names = mhg->get_attribute_names(ct);
		att_name_comboBox->addItems(att_names);
	}
}


} // namespace plugin_attribute_editor
} // namespace schnapps
