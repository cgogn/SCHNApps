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

#include <schnapps/plugins/render_transparency/render_transparency.h>

#include <schnapps/plugins/cmap_provider/cmap2_handler.h>
#include <schnapps/plugins/cmap_provider/cmap3_handler.h>

#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_render_transparency
{

Plugin_RenderTransparency::Plugin_RenderTransparency()
{
	this->name_ = SCHNAPPS_PLUGIN_NAME;
}

QString Plugin_RenderTransparency::plugin_name()
{
	return SCHNAPPS_PLUGIN_NAME;
}

bool Plugin_RenderTransparency::enable()
{
	return true;
}

void Plugin_RenderTransparency::disable()
{}

void Plugin_RenderTransparency::draw_object(View*, Object*, const QMatrix4x4&, const QMatrix4x4&)
{}

void Plugin_RenderTransparency::draw(View* view, const QMatrix4x4& proj, const QMatrix4x4& mv)
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
		// volumes
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
	});
}

void Plugin_RenderTransparency::resizeGL(View* view, int width, int height)
{
	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr != transp_drawer_set_.end())
		it_trdr->second->resize(view->devicePixelRatio() * width, view->devicePixelRatio() * height);
}

void Plugin_RenderTransparency::view_linked(View* view)
{
	connect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));
}

void Plugin_RenderTransparency::view_unlinked(View* view)
{
	disconnect(view, SIGNAL(viewerInitialized()), this, SLOT(viewer_initialized()));

	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr != transp_drawer_set_.end())
	{
		delete it_trdr->second;
		transp_drawer_set_.erase(it_trdr);
	}
}

void Plugin_RenderTransparency::viewer_initialized()
{
	View* view = qobject_cast<View*>(sender());
	auto it_trdr = transp_drawer_set_.find(view);
	if (it_trdr != transp_drawer_set_.end())
	{
		delete it_trdr->second;
		transp_drawer_set_.erase(it_trdr);
	}
}

void Plugin_RenderTransparency::add_tr_flat(View* view, CMap2Handler* mh, cgogn::rendering::ShaderFlatTransp::Param* param)
{
	auto& pairs = tr2maps_flat_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it == pairs.end())
		pairs.push_back(p);
}

void Plugin_RenderTransparency::add_tr_phong(View* view, CMap2Handler* mh, cgogn::rendering::ShaderPhongTransp::Param* param)
{
	auto& pairs = tr2maps_phong_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it == pairs.end())
		pairs.push_back(p);
}

void Plugin_RenderTransparency::add_tr_vol(View* view, CMap3Handler* mh, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend)
{
	auto& pairs = tr3maps_[view];
	auto p = std::make_pair(mh, rend);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it == pairs.end())
		pairs.push_back(p);
}

void Plugin_RenderTransparency::remove_tr_flat(View* view, CMap2Handler* mh, cgogn::rendering::ShaderFlatTransp::Param* param)
{
	auto& pairs = tr2maps_flat_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it != pairs.end())
		pairs.erase(it);
}

void Plugin_RenderTransparency::remove_tr_phong(View* view, CMap2Handler* mh, cgogn::rendering::ShaderPhongTransp::Param* param)
{
	auto& pairs = tr2maps_phong_[view];
	auto p = std::make_pair(mh, param);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it != pairs.end())
		pairs.erase(it);
}

void Plugin_RenderTransparency::remove_tr_vol(View* view, CMap3Handler* mh, cgogn::rendering::VolumeTransparencyDrawer::Renderer* rend)
{
	auto& pairs = tr3maps_[view];
	auto p = std::make_pair(mh, rend);
	auto it = std::find(pairs.begin(), pairs.end(), p);
	if (it != pairs.end())
		pairs.erase(it);
}

} // namespace plugin_render_transparency

} // namespace schnapps
