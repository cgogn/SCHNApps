/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Render                                                         *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* Inspired by the surface render plugin                                        *
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

#ifndef SCHNAPPS_PLUGIN_VOLUME_RENDER_H_
#define SCHNAPPS_PLUGIN_VOLUME_RENDER_H_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>

#include <volume_render_dock_tab.h>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/volume_drawer.h>

#include <QAction>
#include <map>

namespace schnapps
{

class MapHandlerGen;
class Plugin_VolumeRender;

namespace plugin_volume_render
{


struct SCHNAPPS_PLUGIN_VOLUME_RENDER_API MapParameters
{
	friend class Plugin_VolumeRender;

	MapParameters() :
		shader_simple_color_param_(nullptr),
		shader_point_sprite_param_(nullptr),
		position_vbo_(nullptr),
		color_vbo_(nullptr),
		vertex_scale_factor_(1.0f),
		vertex_base_size_(1.0f),
		render_vertices_(false),
		render_edges_(false),
		render_faces_(true),
		render_boundary_(false),
		vertex_color_(190, 85, 168),
		edge_color_(0, 0, 0),
		face_color_(85, 168, 190),
		volume_explode_factor_(0.8f)
	{
		shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_->color_ = edge_color_;

		shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_->color_ = vertex_color_;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;

		volume_drawer_ = cgogn::make_unique<cgogn::rendering::VolumeDrawer>();

		volume_drawer_rend_ = volume_drawer_->generate_renderer();
		volume_drawer_rend_->set_explode_volume(volume_explode_factor_);
	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }

	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_)
		{
			shader_simple_color_param_->set_position_vbo(position_vbo_);
			shader_point_sprite_param_->set_position_vbo(position_vbo_);
		}
	}

	cgogn::rendering::VBO* get_color_vbo() const { return color_vbo_; }
	void set_color_vbo(cgogn::rendering::VBO* v)
	{
		color_vbo_ = v;
	}

	const QColor& get_vertex_color() const { return vertex_color_; }
	void set_vertex_color(const QColor& c)
	{
		vertex_color_ = c;
		shader_point_sprite_param_->color_ = vertex_color_;
	}

	const QColor& get_edge_color() const { return edge_color_; }
	void set_edge_color(const QColor& c)
	{
		edge_color_ = c;
		shader_simple_color_param_->color_ = edge_color_;
	}

	const QColor& get_face_color() const { return face_color_; }
	void set_face_color(const QColor& c)
	{
		face_color_ = c;
		volume_drawer_rend_->set_face_color(c);
	}

	float32 get_vertex_base_size() const { return vertex_base_size_; }
	void set_vertex_base_size(float32 bs)
	{
		vertex_base_size_ = bs;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	float32 get_vertex_scale_factor() const { return vertex_scale_factor_; }
	void set_vertex_scale_factor(float32 sf)
	{
		vertex_scale_factor_ = sf;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	float32 get_volume_explode_factor() const { return volume_explode_factor_; }
	void set_volume_explode_factor(float32 vef)
	{
		volume_explode_factor_ = vef;
		volume_drawer_rend_->set_explode_volume(vef);
	}

	cgogn::rendering::VolumeDrawer* get_volume_drawer()
	{
		return volume_drawer_.get();
	}

private:

	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param>	shader_simple_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* color_vbo_;

	QColor vertex_color_;
	QColor edge_color_;
	QColor face_color_;

	float32 vertex_scale_factor_;
	float32 vertex_base_size_;

	float32 volume_explode_factor_;

	std::unique_ptr<cgogn::rendering::VolumeDrawer> volume_drawer_;
	std::unique_ptr<cgogn::rendering::VolumeDrawer::Renderer> volume_drawer_rend_;

public:

	bool render_vertices_;
	bool render_edges_;
	bool render_faces_;
	bool render_boundary_;
};

/**
* @brief Plugin for surface rendering
*/
class SCHNAPPS_PLUGIN_VOLUME_RENDER_API Plugin_VolumeRender : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class VolumeRender_DockTab;

public:

	inline Plugin_VolumeRender() {}

	~Plugin_VolumeRender() {}

private:

	MapParameters& get_parameters(View* view, MapHandlerGen* map);

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override {}
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	inline void keyPress(View*, QKeyEvent*) override {}
	inline void keyRelease(View*, QKeyEvent*) override {}
	inline void mousePress(View*, QMouseEvent*) override {}
	inline void mouseRelease(View*, QMouseEvent*) override {}
	inline void mouseMove(View*, QMouseEvent*) override {}
	inline void wheelEvent(View*, QWheelEvent*) override {}

	void view_linked(View*) override;
	void view_unlinked(View*) override;

private slots:

	// slots called from View signals
	void map_linked(MapHandlerGen* map);
	void map_unlinked(MapHandlerGen* map);

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();

	void update_dock_tab();

public slots:



private:

	VolumeRender_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace plugin_volume_render
} // namespace schnapps


#endif // SCHNAPPS_PLUGIN_VOLUME_RENDER_H_
