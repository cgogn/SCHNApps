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

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/plugins/cmap_provider/cmap0_provider_dock_tab.h>
#include <schnapps/plugins/cmap_provider/cmap1_provider_dock_tab.h>
#include <schnapps/plugins/cmap_provider/cmap2_provider_dock_tab.h>
#include <schnapps/plugins/cmap_provider/cmap3_provider_dock_tab.h>

#include <schnapps/core/schnapps.h>

namespace schnapps
{

namespace plugin_cmap_provider
{

Plugin_CMapProvider::Plugin_CMapProvider()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_CMapProvider::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_CMapProvider::enable()
{
	cmap0_dock_tab_ = new CMap0Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, cmap0_dock_tab_, "CMap0");

	cmap1_dock_tab_ = new CMap1Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, cmap1_dock_tab_, "CMap1");

	cmap2_dock_tab_ = new CMap2Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, cmap2_dock_tab_, "CMap2");

	cmap3_dock_tab_ = new CMap3Provider_DockTab(this->schnapps_, this);
	schnapps_->add_control_dock_tab(this, cmap3_dock_tab_, "CMap3");

	return true;
}

void Plugin_CMapProvider::disable()
{
	schnapps_->remove_control_dock_tab(this, cmap0_dock_tab_);
	delete cmap0_dock_tab_;

	schnapps_->remove_control_dock_tab(this, cmap1_dock_tab_);
	delete cmap1_dock_tab_;

	schnapps_->remove_control_dock_tab(this, cmap2_dock_tab_);
	delete cmap2_dock_tab_;

	schnapps_->remove_control_dock_tab(this, cmap3_dock_tab_);
	delete cmap3_dock_tab_;
}

/*********************************************************
 * CMAP0
 *********************************************************/

CMap0Handler* Plugin_CMapProvider::add_cmap0(const QString& name)
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

	cmap0_dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMapProvider::remove_cmap0(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap0Handler* mh = dynamic_cast<CMap0Handler*>(objects_.at(name));
		if (mh)
		{
			cmap0_dock_tab_->remove_map(mh);
			schnapps_->notify_object_removed(mh);

			objects_.erase(name);
			delete mh;
		}
	}
}

CMap0Handler* Plugin_CMapProvider::duplicate_cmap0(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap0Handler* mh = dynamic_cast<CMap0Handler*>(objects_.at(name));
		if (mh)
		{
			CMap0Handler* duplicate = add_cmap0(QString("copy_") + name);
			CMap0::DartMarker dm(*mh->map());
			duplicate->map()->merge(*mh->map(), dm);
			return duplicate;
		}
		return nullptr;
	}
	else
		return nullptr;
}

CMap0Handler* Plugin_CMapProvider::cmap0(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return dynamic_cast<CMap0Handler*>(objects_.at(name));
	else
		return nullptr;
}

/*********************************************************
 * CMAP1
 *********************************************************/

CMap1Handler* Plugin_CMapProvider::add_cmap1(const QString& name)
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

	cmap1_dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMapProvider::remove_cmap1(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap1Handler* mh = dynamic_cast<CMap1Handler*>(objects_.at(name));

		if (mh)
		{
			cmap1_dock_tab_->remove_map(mh);
			schnapps_->notify_object_removed(mh);

			objects_.erase(name);
			delete mh;
		}
	}
}

CMap1Handler* Plugin_CMapProvider::duplicate_cmap1(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap1Handler* mh = dynamic_cast<CMap1Handler*>(objects_.at(name));
		if (mh)
		{
			CMap1Handler* duplicate = add_cmap1(QString("copy_") + name);
			CMap1::DartMarker dm(*mh->map());
			duplicate->map()->merge(*mh->map(), dm);
			return duplicate;
		}
		return nullptr;
	}
	else
		return nullptr;
}

CMap1Handler* Plugin_CMapProvider::cmap1(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return dynamic_cast<CMap1Handler*>(objects_.at(name));
	else
		return nullptr;
}

/*********************************************************
 * CMAP2
 *********************************************************/

CMap2Handler* Plugin_CMapProvider::add_cmap2(const QString& name)
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

	cmap2_dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMapProvider::remove_cmap2(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(objects_.at(name));

		if (mh)
		{
			cmap2_dock_tab_->remove_map(mh);
			schnapps_->notify_object_removed(mh);

			objects_.erase(name);
			delete mh;
		}
	}
}

CMap2Handler* Plugin_CMapProvider::duplicate_cmap2(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap2Handler* mh = dynamic_cast<CMap2Handler*>(objects_.at(name));
		if (mh)
		{
			CMap2Handler* duplicate = add_cmap2(QString("copy_") + name);
			CMap2::DartMarker dm(*mh->map());
			duplicate->map()->merge(*mh->map(), dm);
			return duplicate;
		}
		return nullptr;
	}
	else
		return nullptr;
}

CMap2Handler* Plugin_CMapProvider::cmap2(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return dynamic_cast<CMap2Handler*>(objects_.at(name));
	else
		return nullptr;
}

/*********************************************************
 * CMAP3
 *********************************************************/

CMap3Handler* Plugin_CMapProvider::add_cmap3(const QString& name)
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

	CMap3Handler* mh = new CMap3Handler(final_name, this);
	objects_.insert(std::make_pair(final_name, mh));

	cmap3_dock_tab_->add_map(mh);
	schnapps_->notify_object_added(mh);

	return mh;
}

void Plugin_CMapProvider::remove_cmap3(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap3Handler* mh = dynamic_cast<CMap3Handler*>(objects_.at(name));

		if (mh)
		{
			cmap3_dock_tab_->remove_map(mh);
			schnapps_->notify_object_removed(mh);

			objects_.erase(name);
			delete mh;
		}
	}
}

CMap3Handler* Plugin_CMapProvider::duplicate_cmap3(const QString& name)
{
	if (objects_.count(name) > 0ul)
	{
		CMap3Handler* mh = dynamic_cast<CMap3Handler*>(objects_.at(name));
		if (mh)
		{
			CMap3Handler* duplicate = add_cmap3(QString("copy_") + name);
			CMap3::DartMarker dm(*mh->map());
			duplicate->map()->merge(*mh->map(), dm);
			return duplicate;
		}
		return nullptr;
	}
	else
		return nullptr;
}

CMap3Handler* Plugin_CMapProvider::cmap3(const QString& name) const
{
	if (objects_.count(name) > 0ul)
		return dynamic_cast<CMap3Handler*>(objects_.at(name));
	else
		return nullptr;
}

void Plugin_CMapProvider::schnapps_closing()
{

}

} // namespace plugin_cmap_provider

} // namespace schnapps
