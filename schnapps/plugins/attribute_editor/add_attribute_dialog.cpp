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

#include <schnapps/plugins/attribute_editor/add_attribute_dialog.h>
#include <schnapps/plugins/attribute_editor/attribute_editor.h>
#include <schnapps/plugins/attribute_editor/attribute_factory.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <schnapps/core/schnapps.h>

#include <map>
#include <string>

namespace schnapps
{

namespace plugin_attribute_editor
{

using CMapHandlerGen = plugin_cmap_provider::CMapHandlerGen;

AddAttributeDialog::AddAttributeDialog(SCHNApps* s, AttributeEditorPlugin* p)
{
	schnapps_ = s;
	plugin_ = p;
	updating_ui_ = false;
	setupUi(this);

	tableWidget_defaultValue->verticalHeader()->setVisible(false);
	tableWidget_defaultValue->horizontalHeader()->setVisible(false);
	schnapps_->foreach_object([&] (Object* mhg)
	{
		map_added(mhg);
	});

// NOTE : not good order.
//	for (const auto& pair : AttributeFactory<CMap2>::get_instance().get_map())
//		type_comboBox->addItem(QString::fromStdString(pair.first));

	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(cgogn::Dart())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(bool())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(int8())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(int16())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(int32())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(int64())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(uint8())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(uint16())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(uint32())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(uint64())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(float32())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(float64())));

	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(VEC2F())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(VEC2D())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(VEC3F())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(VEC3D())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(VEC4F())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(VEC4D())));

	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(MAT2F())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(MAT2D())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(MAT3F())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(MAT3D())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(MAT4F())));
	type_comboBox->addItem(QString::fromStdString(cgogn::name_of_type(MAT4D())));

	connect(schnapps_, SIGNAL(object_added(Object*)), this, SLOT(map_added(Object*)));
	connect(schnapps_, SIGNAL(object_removed(Object*)), this, SLOT(map_removed(Object*)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(add_attribute_validated()));
	connect(this->type_comboBox,SIGNAL(currentTextChanged(QString)), this, SLOT(data_type_changed(QString)));
	connect(this->mapSelectionComboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(selected_map_changed(QString)));
}

void AddAttributeDialog::map_added(Object* o)
{
	if (CMapHandlerGen* mhg = dynamic_cast<CMapHandlerGen*>(o))
		mapSelectionComboBox->addItem(mhg->name());
}

void AddAttributeDialog::map_removed(Object* o)
{
	if (CMapHandlerGen* mhg = dynamic_cast<CMapHandlerGen*>(o))
		mapSelectionComboBox->removeItem(this->mapSelectionComboBox->findText(mhg->name()));
}

void AddAttributeDialog::selected_map_changed(const QString& map)
{
	CMapHandlerGen* mhg = plugin_->cmap_provider()->cmap(map);
	if (mhg)
	{
		orbit_combobox->clear();
		for (auto it : available_orbits(mhg))
			orbit_combobox->addItem(QString::fromStdString(cgogn::orbit_name(it)).split("::").back());
	}
}

void AddAttributeDialog::data_type_changed(const QString& data_type)
{
	static const std::map<std::string, int> nb_components_map {
		{cgogn::name_of_type(cgogn::Dart()), 1},
		{cgogn::name_of_type(bool()), 1},
		{cgogn::name_of_type(int8()), 1},
		{cgogn::name_of_type(int16()), 1},
		{cgogn::name_of_type(int32()), 1},
		{cgogn::name_of_type(int64()), 1},
		{cgogn::name_of_type(uint8()), 1},
		{cgogn::name_of_type(uint16()), 1},
		{cgogn::name_of_type(uint32()), 1},
		{cgogn::name_of_type(uint64()), 1},
		{cgogn::name_of_type(float32()), 1},
		{cgogn::name_of_type(float64()), 1},

		{cgogn::name_of_type(VEC2F()), 2},
		{cgogn::name_of_type(VEC2D()), 2},
		{cgogn::name_of_type(VEC3F()), 3},
		{cgogn::name_of_type(VEC3D()), 3},
		{cgogn::name_of_type(VEC4F()), 4},
		{cgogn::name_of_type(VEC4D()), 4},

		{cgogn::name_of_type(MAT2F()), 4},
		{cgogn::name_of_type(MAT2D()), 4},
		{cgogn::name_of_type(MAT3F()), 9},
		{cgogn::name_of_type(MAT3D()), 9},
		{cgogn::name_of_type(MAT4F()), 16},
		{cgogn::name_of_type(MAT4D()), 16},
	};

	tableWidget_defaultValue->clearContents();
	tableWidget_defaultValue->setRowCount(1);

	auto it = nb_components_map.find(data_type.toStdString());
	if (it != nb_components_map.end())
	{
		tableWidget_defaultValue->setColumnCount(it->second);
	} else {
		tableWidget_defaultValue->setColumnCount(1);
	}
	for (int c = 0 ; c < tableWidget_defaultValue->columnCount(); ++c)
		tableWidget_defaultValue->setItem(0, c, new QTableWidgetItem());
}

void AddAttributeDialog::add_attribute_validated()
{
	CMapHandlerGen* mhg = plugin_->cmap_provider()->cmap(mapSelectionComboBox->currentText());
	QStringList defaut_value_components;
	for (int c = 0 ; c < this->tableWidget_defaultValue->columnCount(); ++c)
		defaut_value_components.push_back(this->tableWidget_defaultValue->item(0,c)->text());
	if (mhg)
	{
		if (mhg->type()== plugin_cmap_provider::CMapType::CMAP0)
			AttributeFactory<CMap0>::get_instance().create_attribute(dynamic_cast<plugin_cmap_provider::CMap0Handler*>(mhg),
												type_comboBox->currentText().toStdString(),
												orbit_from_string(orbit_combobox->currentText()),
												name_lineEdit->text().toStdString(),
												defaut_value_components);
		if (mhg->type()== plugin_cmap_provider::CMapType::CMAP1)
			AttributeFactory<CMap1>::get_instance().create_attribute(dynamic_cast<plugin_cmap_provider::CMap1Handler*>(mhg),
												type_comboBox->currentText().toStdString(),
												orbit_from_string(orbit_combobox->currentText()),
												name_lineEdit->text().toStdString(),
												defaut_value_components);
		if (mhg->type()== plugin_cmap_provider::CMapType::CMAP2)
			AttributeFactory<CMap2>::get_instance().create_attribute(dynamic_cast<plugin_cmap_provider::CMap2Handler*>(mhg),
												type_comboBox->currentText().toStdString(),
												orbit_from_string(orbit_combobox->currentText()),
												name_lineEdit->text().toStdString(),
												defaut_value_components);
		if (mhg->type()== plugin_cmap_provider::CMapType::CMAP3)
			AttributeFactory<CMap3>::get_instance().create_attribute(dynamic_cast<plugin_cmap_provider::CMap3Handler*>(mhg),
												type_comboBox->currentText().toStdString(),
												orbit_from_string(orbit_combobox->currentText()),
												name_lineEdit->text().toStdString(),
												defaut_value_components);
		if (mhg->type()== plugin_cmap_provider::CMapType::UNDIRECTED_GRAPH)
			AttributeFactory<UndirectedGraph>::get_instance().create_attribute(dynamic_cast<plugin_cmap_provider::UndirectedGraphHandler*>(mhg),
												type_comboBox->currentText().toStdString(),
												orbit_from_string(orbit_combobox->currentText()),
												name_lineEdit->text().toStdString(),
												defaut_value_components);
	}
}

} // namespace plugin_attribute_editor
} // namespace schnapps
