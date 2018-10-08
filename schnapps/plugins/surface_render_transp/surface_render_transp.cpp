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

#include <schnapps/plugins/surface_render_transp/surface_render_transp.h>

//#include <schnapps/plugins/surface_render/surface_render.h>
//#include <schnapps/plugins/volume_render/volume_render.h>

#include <schnapps/plugins/cmap2_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap3_provider/cmap3_provider.h>

#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_surface_render_transp
{

Plugin_SurfaceRenderTransp::Plugin_SurfaceRenderTransp()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_SurfaceRenderTransp::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_SurfaceRenderTransp::enable()
{
	return true;
}

void Plugin_SurfaceRenderTransp::disable()
{}

void Plugin_SurfaceRenderTransp::draw_object(View*, Object*, const QMatrix4x4&, const QMatrix4x4&)
{}

void Plugin_SurfaceRenderTransp::draw(View* view, const QMatrix4x4& proj, const QMatrix4x4& mv)
{
	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr == transp_drawer_set_.end())
	{
		auto ptr = new cgogn::rendering::SurfaceTransparencyDrawer();
		it_trdr = (transp_drawer_set_.insert(std::make_pair(view, ptr))).first;
		it_trdr->second->resize(view->devicePixelRatio() * view->width(), view->devicePixelRatio() * view->height());
	}

	auto it2f = tr2maps_flat_.find(view);
	auto it2p = tr2maps_phong_.find(view);
	auto it3 = tr3maps_.find(view);

	it_trdr->second->draw([&] ()
	{
		// surfaces
//		if (view->is_linked_to_plugin(plugin_surface_render::Plugin_SurfaceRender::plugin_name()))
		if (view->is_linked_to_plugin("surface_render"))
		{
			if (it2f != tr2maps_flat_.end())
			{
				for (const auto& pm : it2f->second)
				{
					const auto& m = pm.first;
					QMatrix4x4 mmv = mv * m->frame_matrix() * m->transformation_matrix();
					pm.second->bind(proj, mmv);
					m->draw(cgogn::rendering::TRIANGLES);
					pm.second->release();
				}
			}

			if (it2p != tr2maps_phong_.end())
			{
				for (const auto& pm : it2p->second)
				{
					const auto& m = pm.first;
					QMatrix4x4 mmv = mv * m->frame_matrix() * m->transformation_matrix();
					pm.second->bind(proj, mmv);
					m->draw(cgogn::rendering::TRIANGLES);
					pm.second->release();
				}
			}
		}
		// volumes
//		if (view->is_linked_to_plugin(plugin_volume_render::Plugin_VolumeRender::plugin_name()))
		if (view->is_linked_to_plugin("volume_render"))
		{
			if (it3 != tr3maps_.end())
			{
				for (const auto& pm : it3->second)
				{
					const auto& m = pm.first;
					QMatrix4x4 mmv = mv * m->frame_matrix() * m->transformation_matrix();
					cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend = pm.second;
					rend->draw_faces(proj, mmv);
				}
			}
		}
	});
}

void Plugin_SurfaceRenderTransp::resizeGL(View* view, int width, int height)
{
	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr != transp_drawer_set_.end())
		it_trdr->second->resize(view->devicePixelRatio() * width, view->devicePixelRatio() * height);
}

void Plugin_SurfaceRenderTransp::view_linked(View* view)
{
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));
}

void Plugin_SurfaceRenderTransp::view_unlinked(View* view)
{
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr != transp_drawer_set_.end())
	{
		delete it_trdr->second;
		transp_drawer_set_.erase(it_trdr);
	}
}

void Plugin_SurfaceRenderTransp::viewer_initialized()
{
	View* view = dynamic_cast<View*>(sender());
	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr != transp_drawer_set_.end())
	{
		delete it_trdr->second;
		transp_drawer_set_.erase(it_trdr);
	}
}

void Plugin_SurfaceRenderTransp::add_tr_flat(View* view, CMap2Handler* mh, cgogn::rendering::ShaderFlatTransp::Param* param)
{
	auto& pairs = tr2maps_flat_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it == pairs.end())
		pairs.push_back(p);
}

void Plugin_SurfaceRenderTransp::add_tr_phong(View* view, CMap2Handler* mh, cgogn::rendering::ShaderPhongTransp::Param* param)
{
	auto& pairs = tr2maps_phong_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it == pairs.end())
		pairs.push_back(p);
}

void Plugin_SurfaceRenderTransp::add_tr_vol(View* view, CMap3Handler* mh, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend)
{
	auto& pairs = tr3maps_[view];
	auto p = std::make_pair(mh, rend);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it == pairs.end())
		pairs.push_back(p);
}

void Plugin_SurfaceRenderTransp::remove_tr_flat(View* view, CMap2Handler* mh, cgogn::rendering::ShaderFlatTransp::Param* param)
{
	auto& pairs = tr2maps_flat_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it != pairs.end())
		pairs.erase(it);
}

void Plugin_SurfaceRenderTransp::remove_tr_phong(View* view, CMap2Handler* mh, cgogn::rendering::ShaderPhongTransp::Param* param)
{
	auto& pairs = tr2maps_phong_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it != pairs.end())
		pairs.erase(it);
}

void Plugin_SurfaceRenderTransp::remove_tr_vol(View* view, CMap3Handler* mh, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend)
{
	auto& pairs = tr3maps_[view];
	auto p = std::make_pair(mh, rend);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it != pairs.end())
		pairs.erase(it);
}

} // namespace plugin_surface_render_transp

} // namespace schnapps
