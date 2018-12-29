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

#include <schnapps/plugins/cmap2_provider/cmap2_provider.h>
#include <schnapps/plugins/cmap2_provider/cmap2_provider_dock_tab.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_cmap2_provider
{

Plugin_CMap2Provider::Plugin_CMap2Provider()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_CMap2Provider::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_CMap2Provider::enable()
{
	dock_tab_ = new CMap2Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, dock_tab_, "CMap2");

	return true;
}

void Plugin_CMap2Provider::disable()
{
	schnapps_->remove_control_dock_tab(this, dock_tab_);
	delete dock_tab_;
}

CMap2Handler* Plugin_CMap2Provider::add_map(const QString& name)
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

	CMap2Handler* mh = new CMap2Handler(final_name, this);
	objects_.insert(std::make_pair(final_name, mh));

	dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMap2Provider::remove_map(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap2Handler* mh = reinterpret_cast<CMap2Handler*>(objects_.at(name));
		dock_tab_->remove_map(mh);
		schnapps_->notify_object_removed(mh);
		objects_.erase(name);
		delete mh;
	}
}

CMap2Handler* Plugin_CMap2Provider::duplicate_map(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap2Handler* mh = static_cast<CMap2Handler*>(objects_.at(name));
		CMap2Handler* duplicate = this->add_map(QString("copy_") + name);
		CMap2::DartMarker dm(*mh->map());
		if (duplicate->map()->merge(*mh->map(), dm))
			return duplicate;
		else
			this->remove_map(QString("copy_") + name);
	}
	return nullptr;
}

CMap2Handler* Plugin_CMap2Provider::map(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return static_cast<CMap2Handler*>(objects_.at(name));
	else
		return nullptr;
}

void Plugin_CMap2Provider::schnapps_closing()
{

}

} // namespace plugin_cmap2_provider

} // namespace schnapps
