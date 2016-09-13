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
#include <add_attribute_dialog.h>
#include <attribute_editor.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/schnapps.h>
#include <attribute_factory.h>

namespace schnapps
{

namespace plugin_attribute_editor
{

AddAttributeDialog::AddAttributeDialog(SCHNApps* s, AttributeEditorPlugin* p)
{
	schnapps_ = s;
	plugin_ = p;
	updating_ui_ = false;
	setupUi(this);

	schnapps_->foreach_map([&](MapHandlerGen* mhg)
	{
		map_added(mhg);
	});

	connect(schnapps_, SIGNAL(map_added(MapHandlerGen*)), this, SLOT(map_added(MapHandlerGen*)));
	connect(schnapps_, SIGNAL(map_removed(MapHandlerGen*)), this, SLOT(map_removed(MapHandlerGen*)));
	connect(this->buttonBox,SIGNAL(accepted()), this, SLOT(add_attribute_validated()));

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
}

void AddAttributeDialog::map_added(MapHandlerGen* mhg)
{
	if (mhg)
		mapSelectionComboBox->addItem(mhg->get_name());
}

void AddAttributeDialog::map_removed(MapHandlerGen* mhg)
{
	if (mhg)
		mapSelectionComboBox->removeItem(this->mapSelectionComboBox->findText(mhg->get_name()));
}

void AddAttributeDialog::add_attribute_validated()
{
	MapHandlerGen* mhg = schnapps_->get_map(mapSelectionComboBox->currentText());
	if (mhg)
	{
		if (mhg->dimension() == 2u)
			AttributeFactory<CMap2>::get_instance().create_attribute(dynamic_cast<CMap2Handler*>(mhg),
												type_comboBox->currentText().toStdString(),
												cell_type(orbit_combobox->currentText().toStdString()),
												name_lineEdit->text().toStdString());
		else
			AttributeFactory<CMap3>::get_instance().create_attribute(dynamic_cast<CMap3Handler*>(mhg),
												type_comboBox->currentText().toStdString(),
												cell_type(orbit_combobox->currentText().toStdString()),
												name_lineEdit->text().toStdString());
	}
}

} // namespace plugin_attribute_editor
} // namespace schnapps
