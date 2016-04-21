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

#include <surface_render.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

namespace schnapps
{

bool Plugin_SurfaceRender::enable()
{
//	magic line that init static variables of GenericMap in the plugins
//	GenericMap::copyAllStatics(m_schnapps->getStaticPointers());

	shader_flat_ = new cgogn::rendering::ShaderFlat();

	return true;
}

void Plugin_SurfaceRender::draw_map(View *view, MapHandlerGen *map)
{
}

Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")

} // namespace schnapps
