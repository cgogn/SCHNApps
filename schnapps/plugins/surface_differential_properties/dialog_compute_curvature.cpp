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

#include <dialog_compute_curvature.h>

#include <surface_differential_properties.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

//ComputeCurvature_Dialog::ComputeCurvature_Dialog(SCHNApps* s) :
//	schnapps_(s),
//	selected_map_(nullptr)
//{
//	setupUi(this);

//	KmaxAttributeName->setText("Kmax");
//	kmaxAttributeName->setText("kmax");
//	KminAttributeName->setText("Kmin");
//	kminAttributeName->setText("kmin");
//	KnormalAttributeName->setText("Knormal");

//	connect(schnapps_, SIGNAL(mapAdded(MapHandlerGen*)), this, SLOT(addMapToList(MapHandlerGen*)));
//	connect(schnapps_, SIGNAL(mapRemoved(MapHandlerGen*)), this, SLOT(removeMapFromList(MapHandlerGen*)));

//	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selectedMapChanged()));

//	foreach(MapHandlerGen* map,  schnapps_->getMapSet().values())
//	{
//		QListWidgetItem* item = new QListWidgetItem(map->getName(), list_maps);
//		item->setCheckState(Qt::Unchecked);
//	}
//}

//void ComputeCurvature_Dialog::selectedMapChanged()
//{
//	if(selected_map_)
//		disconnect(selected_map_, SIGNAL(attributeAdded(unsigned int, const QString&)), this, SLOT(addAttributeToList(unsigned int, const QString&)));

//	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
//	if(!currentItems.empty())
//	{
//		combo_positionAttribute->clear();
//		combo_normalAttribute->clear();
//		combo_KmaxAttribute->clear();
//		combo_KminAttribute->clear();
//		combo_KnormalAttribute->clear();
//		combo_kmaxAttribute->clear();
//		combo_kminAttribute->clear();

//		const QString& mapname = currentItems[0]->text();
//		MapHandlerGen* mh = schnapps_->getMap(mapname);

//		QString vec3TypeName = QString::fromStdString(nameOfType(PFP2::VEC3()));
//		QString realTypeName = QString::fromStdString(nameOfType(PFP2::REAL()));

//		unsigned int j = 0;
//		unsigned int k = 0;
//		const AttributeSet& attribs = mh->getAttributeSet(VERTEX);
//		for(AttributeSet::const_iterator i = attribs.constBegin(); i != attribs.constEnd(); ++i)
//		{
//			if(i.value() == vec3TypeName)
//			{
//				combo_positionAttribute->addItem(i.key());
//				combo_normalAttribute->addItem(i.key());
//				combo_KmaxAttribute->addItem(i.key());
//				combo_KminAttribute->addItem(i.key());
//				combo_KnormalAttribute->addItem(i.key());

//				++j;
//			}
//			else if(i.value() == realTypeName)
//			{
//				combo_kmaxAttribute->addItem(i.key());
//				combo_kminAttribute->addItem(i.key());

//				++k;
//			}
//		}

//		selected_map_ = mh;
//		connect(selected_map_, SIGNAL(attributeAdded(unsigned int, const QString&)), this, SLOT(addAttributeToList(unsigned int, const QString&)));
//	}
//	else
//		selected_map_ = nullptr;
//}

//void ComputeCurvature_Dialog::addMapToList(MapHandlerGen* m)
//{
//	QListWidgetItem* item = new QListWidgetItem(m->getName(), list_maps);
//	item->setCheckState(Qt::Unchecked);
//}

//void ComputeCurvature_Dialog::removeMapFromList(MapHandlerGen* m)
//{
//	QList<QListWidgetItem*> items = list_maps->findItems(m->getName(), Qt::MatchExactly);
//	if(!items.empty())
//		delete items[0];

//	if(selected_map_ == m)
//	{
//		disconnect(selected_map_, SIGNAL(attributeAdded(unsigned int, const QString&)), this, SLOT(addAttributeToList(unsigned int, const QString&)));
//		selected_map_ = nullptr;
//	}
//}

//void ComputeCurvature_Dialog::addAttributeToList(unsigned int orbit, const QString& nameAttr)
//{
//	QString vec3TypeName = QString::fromStdString(nameOfType(PFP2::VEC3()));
//	QString realTypeName = QString::fromStdString(nameOfType(PFP2::REAL()));

//	const QString& typeAttr = selected_map_->getAttributeTypeName(orbit, nameAttr);

//	if(typeAttr == vec3TypeName)
//	{
//		combo_positionAttribute->addItem(nameAttr);
//		combo_normalAttribute->addItem(nameAttr);
//		combo_KmaxAttribute->addItem(nameAttr);
//		combo_KminAttribute->addItem(nameAttr);
//		combo_KnormalAttribute->addItem(nameAttr);
//	}
//	else if(typeAttr == realTypeName)
//	{
//		combo_kmaxAttribute->addItem(nameAttr);
//		combo_kminAttribute->addItem(nameAttr);
//	}
//}

} // namespace schnapps
