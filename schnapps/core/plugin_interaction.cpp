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

#include <schnapps/core/plugin_interaction.h>

namespace schnapps
{

const QList<View*>& PluginInteraction::get_linked_views() const
{
	return views_;
}

bool PluginInteraction::is_linked_to_view(View* view) const
{
	return views_.contains(view);
}

void PluginInteraction::link_view(View* view)
{
	if(view && !views_.contains(view))
	{
		views_.push_back(view);
		view_linked(view);
	}
}

void PluginInteraction::unlink_view(View* view)
{
	if(views_.removeOne(view))
		view_unlinked(view);
}

void PluginInteraction::register_shader(cgogn::rendering::ShaderProgram* shader)
{
	if(shader && !shaders_.contains(shader))
		shaders_.push_back(shader);
}

void PluginInteraction::register_shaders(const std::vector<cgogn::rendering::ShaderProgram*>& shaders)
{
	for (auto shader: shaders)
		if(shader && !shaders_.contains(shader))
			shaders_.push_back(shader);
}

void PluginInteraction::unregister_shader(cgogn::rendering::ShaderProgram* shader)
{
	shaders_.removeOne(shader);
}

void PluginInteraction::unregister_shaders(const std::vector<cgogn::rendering::ShaderProgram*>& shaders)
{
	for (auto shader: shaders)
		shaders_.removeOne(shader);
}

} // namespace schnapps
