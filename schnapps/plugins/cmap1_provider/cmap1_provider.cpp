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

#include <schnapps/plugins/cmap1_provider/cmap1_provider.h>
#include <schnapps/plugins/cmap1_provider/cmap1_provider_dock_tab.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_cmap1_provider
{

Plugin_CMap1Provider::Plugin_CMap1Provider()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_CMap1Provider::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_CMap1Provider::enable()
{
	dock_tab_ = new CMap1Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, dock_tab_, "CMap1");

	return true;
}

void Plugin_CMap1Provider::disable()
{
	schnapps_->remove_control_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

CMap1Handler* Plugin_CMap1Provider::add_map(const QString& name)
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

	CMap1Handler* mh = new CMap1Handler(final_name, this);
	objects_.insert(std::make_pair(final_name, mh));

	dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMap1Provider::remove_map(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap1Handler* mh = reinterpret_cast<CMap1Handler*>(objects_.at(name));

		dock_tab_->remove_map(mh);
		schnapps_->notify_object_removed(mh);

		objects_.erase(name);
		delete mh;
	}
}

CMap1Handler* Plugin_CMap1Provider::duplicate_map(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
//		CMap2Handler* mh = objects_.at(name);
//		CMap1Handler* duplicate = this->add_map(QString("copy_") + name);
//		duplicate->map_->merge(mh->map_);
//		return duplicate;
		return nullptr;
	}
	else
		return nullptr;
}

CMap1Handler* Plugin_CMap1Provider::map(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return reinterpret_cast<CMap1Handler*>(objects_.at(name));
	else
		return nullptr;
}

void Plugin_CMap1Provider::schnapps_closing()
{

}

} // namespace plugin_cmap1_provider

} // namespace schnapps
