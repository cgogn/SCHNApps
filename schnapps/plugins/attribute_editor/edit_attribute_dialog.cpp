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
	connect(name_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(attribute_changed(QString)));

	schnapps_->foreach_map([&](MapHandlerGen* mhg)
	{
		map_added(mhg);
	});

//	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(add_attribute_validated()));
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
	name_comboBox->clear();
	name_comboBox->addItem("-select attribute-");
	MapHandlerGen* mhg = schnapps_->get_map(map_name);
	if (mhg)
	{
		const auto& att_names = mhg->get_attribute_names(cell_type(orbit_comboBox->currentText().toStdString()));
		name_comboBox->addItems(att_names);
	}
}

void EditAttributeDialog::orbit_changed(const QString& orbit_name)
{
	name_comboBox->clear();
	name_comboBox->addItem("-select attribute-");
	MapHandlerGen* mhg = schnapps_->get_map(map_comboBox->currentText());
	if (mhg)
	{
		const auto& att_names = mhg->get_attribute_names(cell_type(orbit_name.toStdString()));
		name_comboBox->addItems(att_names);
	}
}

void EditAttributeDialog::attribute_changed(const QString& attribute_name)
{
	MapHandlerGen* mhg = schnapps_->get_map(map_comboBox->currentText());
	if (mhg)
	{
		CellType cell_t = cell_type(orbit_comboBox->currentText().toStdString());
		if (cell_t != CellType::Unknown)
		{
			const auto& ca_cont = mhg->const_attribute_container(cell_t);
			auto* ca = ca_cont.get_chunk_array(attribute_name.toStdString());
			if (ca)
			{
				const uint32 nb_cols = ca->nb_components() + 1u;

				attribute_tableWidget->clearContents();
				attribute_tableWidget->setColumnCount(int32(nb_cols));
				attribute_tableWidget->setRowCount(0);

				{
					QTableWidgetItem *header_emb = new QTableWidgetItem;
					header_emb->setText("embedding");
					attribute_tableWidget->setHorizontalHeaderItem(0,header_emb);
					for (int i =1; i < nb_cols; ++i)
					{
						if (!attribute_tableWidget->horizontalHeaderItem(i))
						{
							QTableWidgetItem *item = new QTableWidgetItem;
							item->setText(QString::number(i-1));
							attribute_tableWidget->setHorizontalHeaderItem(i, item);
						}
					}
				}


				int r = 0;
				mhg->foreach_cell(cell_t, [&](cgogn::Dart d)
				{
					int c = 0;
					attribute_tableWidget->insertRow(r);
					std::vector<QTableWidgetItem*> items(nb_cols);
					for (auto& item : items)
						item = new QTableWidgetItem;

					const uint32 emb = mhg->embedding(d, cell_t);
					items[0]->setText(QString::number(mhg->embedding(d, cell_t)));
					items[0]->setFlags(Qt::NoItemFlags);
					attribute_tableWidget->setItem(r,c++, items[0]);

					std::stringstream sstream;
					ca->export_element(emb, sstream, false, false);
					std::string val;

					for (uint32 i = 1u; i < nb_cols ; ++i)
					{
						sstream >> val;
						items[i]->setText(QString::fromStdString(val));
						attribute_tableWidget->setItem(r,c++, items[i]);
					}
					++r;
				});
			}
		}
	}
}

void EditAttributeDialog::edit_attribute_validated()
{

}


} // namespace plugin_attribute_editor
} // namespace schnapps
