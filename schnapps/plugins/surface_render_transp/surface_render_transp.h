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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_H_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/rendering/transparency_shaders/shader_transparent_flat.h>
#include <cgogn/rendering/transparency_shaders/shader_transparent_phong.h>
#include <cgogn/rendering/transparency_volume_drawer.h>
#include <cgogn/rendering/transparency_drawer.h>
#include <QAction>
#include <map>

namespace schnapps
{

namespace plugin_surface_render_transp
{

/**
* @brief Plugin for surface rendering
*/
class SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API Plugin_SurfaceRenderTransp: public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_SurfaceRenderTransp();

	~Plugin_SurfaceRenderTransp() override;

	inline bool auto_activate() override { return true; }

	void add_tr_flat(View* view, MapHandlerGen* map, cgogn::rendering::ShaderFlatTransp::Param* param);
	void add_tr_phong(View* view, MapHandlerGen* map, cgogn::rendering::ShaderPhongTransp::Param* param);
	void add_tr_vol(View* view, MapHandlerGen* map, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend);

	void remove_tr_flat(View* view, MapHandlerGen* map, cgogn::rendering::ShaderFlatTransp::Param* param);
	void remove_tr_phong(View* view, MapHandlerGen* map, cgogn::rendering::ShaderPhongTransp::Param* param);
	void remove_tr_vol(View* view, MapHandlerGen* map, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend);

private:

	bool enable() override;
	void disable() override;

	void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	void keyPress(View*, QKeyEvent*) override {}
	void keyRelease(View*, QKeyEvent*) override {}
	void mousePress(View*, QMouseEvent*) override {}
	void mouseRelease(View*, QMouseEvent*) override {}
	void mouseMove(View*, QMouseEvent*) override {}
	void wheelEvent(View*, QWheelEvent*) override {}
	void resizeGL(View* view, int width, int height) override;

	void view_linked(View*) override;
	void view_unlinked(View*) override;

private slots:
	void viewer_initialized();

private:

	std::map<View*, cgogn::rendering::SurfaceTransparencyDrawer*> transp_drawer_set_;
	std::map<View*,std::vector<std::pair<MapHandlerGen*, cgogn::rendering::ShaderFlatTransp::Param*>>> tr2maps_flat_;
	std::map<View*,std::vector<std::pair<MapHandlerGen*, cgogn::rendering::ShaderPhongTransp::Param*>>> tr2maps_phong_;
	std::map<View*,std::vector<std::pair<MapHandlerGen*, cgogn::rendering::VolumeTransparencyDrawer::Renderer*>>> tr3maps_;

};

} // namespace plugin_surface_render_transp_transp

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_H_
