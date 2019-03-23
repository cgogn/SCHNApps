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

#include <schnapps/plugins/attribute_editor/edit_attribute_dialog.h>
#include <schnapps/plugins/attribute_editor/attribute_editor.h>
#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>
#include <schnapps/core/schnapps.h>

#include <sstream>

namespace schnapps
{

namespace plugin_attribute_editor
{

using CMapHandlerGen = plugin_cmap_provider::CMapHandlerGen;

EditAttributeDialog::EditAttributeDialog(SCHNApps* s, AttributeEditorPlugin* p) :
	schnapps_(s),
	plugin_(p),
	updating_ui_(false)
{
	setupUi(this);

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(map_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(map_removed(Object*)));
	connect(map_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(selected_map_changed(QString)));
	connect(orbit_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(orbit_changed(QString)));
	connect(att_name_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(attribute_changed(QString)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(edit_attribute_validated()));
	connect(this->apply_button,SIGNAL(pressed()), this, SLOT(edit_attribute_validated()));
	connect(this->cellsSet_comboBox,SIGNAL(currentTextChanged(QString)), this, SLOT(cells_set_changed(QString)));

	schnapps_->foreach_object([&](Object* mhg)
	{
		map_added(mhg);
	});
}

void EditAttributeDialog::map_added(Object* o)
{
	if (dynamic_cast<CMapHandlerGen*>(o))
	{
		map_comboBox->addItem(o->name());
		if (map_comboBox->count() == 1)
			selected_map_changed(o->name());
	}
}

void EditAttributeDialog::map_removed(Object* mhg)
{
	if (mhg)
		map_comboBox->removeItem(this->map_comboBox->findText(mhg->name()));
}

void EditAttributeDialog::selected_map_changed(const QString& map_name)
{
	CMapHandlerGen* mhg = plugin_->cmap_provider()->cmap(map_name);
	const cgogn::Orbit ct = orbit_from_string(orbit_comboBox->currentText());
	update_attribute_list(mhg, ct);
	update_cells_sets(mhg, ct);
	if (mhg)
	{
		orbit_comboBox->clear();
		for (auto it : available_orbits(mhg))
			orbit_comboBox->addItem(QString::fromStdString(cgogn::orbit_name(it)).split("::").back());
	}
}

void EditAttributeDialog::orbit_changed(const QString& orbit_name)
{

	CMapHandlerGen* mhg = plugin_->cmap_provider()->cmap(map_comboBox->currentText());
	const cgogn::Orbit ct = orbit_from_string(orbit_name);
	update_attribute_list(mhg, ct);
	update_cells_sets(mhg, ct);
}

void EditAttributeDialog::cells_set_changed(const QString& cells_set_name)
{
	attribute_changed(att_name_comboBox->currentText());
}

void EditAttributeDialog::attribute_changed(const QString& attribute_name)
{
	CMapHandlerGen* mhg = plugin_->cmap_provider()->cmap(map_comboBox->currentText());
	if (mhg)
	{
		cgogn::Orbit cell_t = orbit_from_string(orbit_comboBox->currentText());
		{
			const auto& ca_cont = mhg->map()->attribute_container(cell_t);
			if (!ca_cont.has_array(attribute_name.toStdString()))
				return;
			auto* ca = ca_cont.get_chunk_array(attribute_name.toStdString());
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

					const uint32 emb = mhg->map()->embedding(d, cell_t);
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

				plugin_cmap_provider::CMapCellsSetGen* csg = mhg->cells_set(cell_t, cellsSet_comboBox->currentText());
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
	CMapHandlerGen* mhg = plugin_->cmap_provider()->cmap(map_comboBox->currentText());
	if (mhg)
	{
		cgogn::Orbit cell_t = orbit_from_string(orbit_comboBox->currentText());
		{
			const QString& attribute_name = att_name_comboBox->currentText();
			const auto& ca_cont = mhg->map()->attribute_container(cell_t);
			// TODO Avoid this const_cast !! E.S.
			auto* ca = const_cast<cgogn::MapBaseData::ChunkArrayGen*>(ca_cont.get_chunk_array(attribute_name.toStdString()));
			if (ca)
			{
				int32 r = 0, rend = attribute_tableWidget->rowCount();
				const int32 nbc = attribute_tableWidget->columnCount();

				for ( ; r < rend; ++r)
				{
					const uint32 emb = attribute_tableWidget->verticalHeaderItem(r)->text().toUInt();
					std::stringstream sstream1;
					for (int32 c = 0; c < nbc; ++c)
						sstream1 << attribute_tableWidget->item(r,c)->text().toStdString()  << " ";

					ca->import_element(emb, sstream1);
					{
						std::stringstream sstream2; // g++-4.9 do not support sstream=std::stringstream(); so use another variable
						ca->export_element(emb, sstream2, false, false);
						for (int32 c = 0; c < nbc; ++c)
						{
							std::string val;
							sstream2 >> val;
							attribute_tableWidget->item(r,c)->setText(QString::fromStdString(val));
						}
					}

				}
				mhg->notify_attribute_change(cell_t, attribute_name);
			}
		}
	}
}

void EditAttributeDialog::update_cells_sets(Object* o, cgogn::Orbit orb)
{
	cellsSet_comboBox->clear();
	cellsSet_comboBox->addItem("-select set-");
	CMapHandlerGen* mhg = dynamic_cast<CMapHandlerGen*>(o);
	if (mhg)
	{
		mhg->foreach_cells_set(orb, [&](plugin_cmap_provider::CMapCellsSetGen* csg)
		{
			cellsSet_comboBox->addItem(csg->name());
		});
	}
}

void EditAttributeDialog::update_attribute_list(Object* o, cgogn::Orbit orb)
{
	att_name_comboBox->clear();
	att_name_comboBox->addItem("-select attribute-");
	CMapHandlerGen* mhg = dynamic_cast<CMapHandlerGen*>(o);
	if (mhg)
	{
		const auto& att_names = get_attribute_names(mhg, orb);
		att_name_comboBox->addItems(att_names);
	}
}

} // namespace plugin_attribute_editor

} // namespace schnapps
