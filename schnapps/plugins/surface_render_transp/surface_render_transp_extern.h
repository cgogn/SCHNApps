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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_EXT_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_EXT_H_

#include <schnapps/plugins/surface_render_transp/dll.h>

#include <cgogn/rendering/transparency_shaders/shader_transparent_flat.h>
#include <cgogn/rendering/transparency_shaders/shader_transparent_phong.h>
#include <cgogn/rendering/transparency_volume_drawer.h>

namespace schnapps
{

class Plugin;
class View;
class MapHandlerGen;

namespace plugin_surface_render_transp
{

SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API void add_tr_flat(Plugin* plug, View* view, MapHandlerGen* map, cgogn::rendering::ShaderFlatTransp::Param* param);
SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API void add_tr_phong(Plugin* plug, View* view, MapHandlerGen* map, cgogn::rendering::ShaderPhongTransp::Param* param);
SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API void add_tr_vol(Plugin* plug, View* view, MapHandlerGen* map, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend);
SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API void remove_tr_flat(Plugin* plug, View* view, MapHandlerGen* map, cgogn::rendering::ShaderFlatTransp::Param* param);
SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API void remove_tr_phong(Plugin* plug, View* view, MapHandlerGen* map, cgogn::rendering::ShaderPhongTransp::Param* param);
SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API void remove_tr_vol(Plugin* plug, View* view, MapHandlerGen* map, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend);

} // namespace plugin_surface_render_transp_transp

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_H_
