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

#ifndef SCHNAPPS_PLUGIN_RENDER_TRANSPARENCY_H_
#define SCHNAPPS_PLUGIN_RENDER_TRANSPARENCY_H_

#include <schnapps/plugins/render_transparency/dll.h>

#include <schnapps/core/plugin_interaction.h>

#include <cgogn/rendering/transparency_shaders/shader_transparent_flat.h>
#include <cgogn/rendering/transparency_shaders/shader_transparent_phong.h>
#include <cgogn/rendering/transparency_volume_drawer.h>
#include <cgogn/rendering/transparency_drawer.h>

#include <QAction>
#include <map>

namespace schnapps
{

namespace plugin_cmap_provider
{
class CMap2Handler;
class CMap3Handler;
}

namespace plugin_render_transparency
{

using CMap2Handler = plugin_cmap_provider::CMap2Handler;
using CMap3Handler = plugin_cmap_provider::CMap3Handler;

/**
* @brief Plugin for transparency rendering
*/
class SCHNAPPS_PLUGIN_RENDER_TRANSPARENCY_API Plugin_RenderTransparency : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_RenderTransparency();
	inline ~Plugin_RenderTransparency() override {}
	static QString plugin_name();

	inline bool auto_activate() override { return true; }

	void add_tr_flat(View* view, CMap2Handler* mh, cgogn::rendering::ShaderFlatTransp::Param* param);
	void add_tr_phong(View* view, CMap2Handler* mh, cgogn::rendering::ShaderPhongTransp::Param* param);
	void add_tr_vol(View* view, CMap3Handler* mh, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend);

	void remove_tr_flat(View* view, CMap2Handler* mh, cgogn::rendering::ShaderFlatTransp::Param* param);
	void remove_tr_phong(View* view, CMap2Handler* mh, cgogn::rendering::ShaderPhongTransp::Param* param);
	void remove_tr_vol(View* view, CMap3Handler* mh, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend);

private:

	bool enable() override;
	void disable() override;

	void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	void draw_object(View* view, Object* o, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	inline bool keyPress(View*, QKeyEvent*) override { return true; }
	inline bool keyRelease(View*, QKeyEvent*) override { return true; }
	inline bool mousePress(View*, QMouseEvent*) override { return true; }
	inline bool mouseRelease(View*, QMouseEvent*) override { return true; }
	inline bool mouseMove(View*, QMouseEvent*) override { return true; }
	inline bool wheelEvent(View*, QWheelEvent*) override { return true; }

	void resizeGL(View* view, int width, int height) override;

	void view_linked(View*) override;
	void view_unlinked(View*) override;

private slots:

	void viewer_initialized();

private:

	std::map<View*, cgogn::rendering::SurfaceTransparencyDrawer*> transp_drawer_set_;
	std::map<View*, std::vector<std::pair<CMap2Handler*, cgogn::rendering::ShaderFlatTransp::Param*>>> tr2maps_flat_;
	std::map<View*, std::vector<std::pair<CMap2Handler*, cgogn::rendering::ShaderPhongTransp::Param*>>> tr2maps_phong_;
	std::map<View*, std::vector<std::pair<CMap3Handler*, cgogn::rendering::VolumeTransparencyDrawer::Renderer*>>> tr3maps_;
};

} // namespace plugin_render_transparency

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_RENDER_TRANSPARENCY_H_
