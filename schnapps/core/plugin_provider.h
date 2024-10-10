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

#ifndef SCHNAPPS_CORE_PLUGIN_PROVIDER_H_
#define SCHNAPPS_CORE_PLUGIN_PROVIDER_H_

#include <schnapps/core/schnapps_core_export.h>

#include <schnapps/core/types.h>
#include <schnapps/core/plugin.h>

namespace schnapps
{

class Object;

class SCHNAPPS_CORE_EXPORT PluginProvider : public Plugin
{
	Q_OBJECT

public:

	PluginProvider()
	{}

	~PluginProvider() override;

//	const std::map<QString, Object*>& objects() const { return objects_; }

	Object* object(const QString& name) const;

	template <typename FUNC>
	void foreach_object(const FUNC& f) const
	{
		static_assert(cgogn::is_func_parameter_same<FUNC, Object*>::value, "Wrong function parameter type");
		for (const auto& object_it : objects_)
			f(object_it.second);
	}

protected:

	std::map<QString, Object*> objects_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_PLUGIN_PROVIDER_H_
