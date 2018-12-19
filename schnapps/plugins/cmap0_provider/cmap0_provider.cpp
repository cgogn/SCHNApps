/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#include <schnapps/plugins/cmap0_provider/cmap0_provider.h>
#include <schnapps/plugins/cmap0_provider/cmap0_provider_dock_tab.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_cmap0_provider
{

Plugin_CMap0Provider::Plugin_CMap0Provider()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_CMap0Provider::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_CMap0Provider::enable()
{
	dock_tab_ = new CMap0Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, dock_tab_, "CMap0");

	return true;
}

void Plugin_CMap0Provider::disable()
{
	schnapps_->remove_control_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

CMap0Handler* Plugin_CMap0Provider::add_map(const QString& name)
{
	QString final_name = name;
	if (objects_.count(name) > 0ul)
	{
		int i = 1;
		do
		{
			final_name = name + QString("_") + QString::number(i);
			++i;
		} while (objects_.count(final_name) > 0ul);
	}

	CMap0Handler* mh = new CMap0Handler(final_name, this);
	objects_.insert(std::make_pair(final_name, mh));

	dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMap0Provider::remove_map(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap0Handler* mh = qobject_cast<CMap0Handler*>(objects_.at(name));

		dock_tab_->remove_map(mh);
		schnapps_->notify_object_removed(mh);

		objects_.erase(name);
		delete mh;
	}
}

CMap0Handler* Plugin_CMap0Provider::duplicate_map(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap0Handler* mh = qobject_cast<CMap0Handler*>(objects_.at(name));
		CMap0Handler* duplicate = this->add_map(QString("copy_") + name);
		CMap0::DartMarker dm(*mh->map());
		duplicate->map()->merge(*mh->map(), dm);
		return duplicate;
//		return nullptr;
	}
	else
		return nullptr;
}

CMap0Handler* Plugin_CMap0Provider::map(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return qobject_cast<CMap0Handler*>(objects_.at(name));
	else
		return nullptr;
}

void Plugin_CMap0Provider::schnapps_closing()
{

}

} // namespace plugin_cmap0_provider

} // namespace schnapps
