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

#ifndef SCHNAPPS_PLUGIN_POLYLINE_RENDER_MAP_PARAMETERS__H_
#define SCHNAPPS_PLUGIN_POLYLINE_RENDER_MAP_PARAMETERS__H_

#include <schnapps/plugins/polyline_render/plugin_polyline_render_export.h>

#include <schnapps/core/types.h>

#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>

namespace schnapps
{

namespace plugin_polyline_render
{

class Plugin_PolylineRender;

struct PLUGIN_POLYLINE_RENDER_EXPORT MapParameters
{
	friend class Plugin_PolylineRender;

	MapParameters() :
		shader_point_sprite_param_(nullptr),
		shader_simple_color_param_(nullptr),
		position_vbo_(nullptr),
		color_vbo_(nullptr),
		render_vertices_(true),
		render_edges_(true),
		vertex_color_(190, 85, 168),
		edge_color_(0, 0, 0),
		vertex_scale_factor_(1.0f),
		vertex_base_size_(1.0f)
	{
		initialize_gl();
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	inline cgogn::rendering::VBO* position_vbo() const { return position_vbo_; }
	inline cgogn::rendering::VBO* color_vbo() const { return color_vbo_; }
	inline bool render_vertices() const { return render_vertices_; }
	inline bool render_edges() const { return render_edges_; }
	inline const QColor& vertex_color() const { return vertex_color_; }
	inline const QColor& edge_color() const { return edge_color_; }
	inline float32 vertex_base_size() const { return vertex_base_size_; }
	inline float32 vertex_scale_factor() const { return vertex_scale_factor_; }

private:

	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_ && position_vbo_->vector_dimension() == 3)
		{
			shader_point_sprite_param_->set_position_vbo(position_vbo_);
			shader_simple_color_param_->set_position_vbo(position_vbo_);
		}
		else
			position_vbo_ = nullptr;
	}

	void set_color_vbo(cgogn::rendering::VBO* v)
	{
		color_vbo_ = v;
	}

	void set_render_vertices(bool b) { render_vertices_ = b; }

	void set_render_edges(bool b) { render_edges_ = b; }

	void set_vertex_color(const QColor& c)
	{
		vertex_color_ = c;
		shader_point_sprite_param_->color_ = vertex_color_;
	}

	void set_edge_color(const QColor& c)
	{
		edge_color_ = c;
		shader_simple_color_param_->color_ = edge_color_;
	}

	void set_vertex_base_size(float32 bs)
	{
		vertex_base_size_ = bs;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	void set_vertex_scale_factor(float32 sf)
	{
		vertex_scale_factor_ = sf;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	inline void initialize_gl()
	{
		shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_->color_ = vertex_color_;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;

		shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_->color_ = edge_color_;

		set_position_vbo(position_vbo_);
		set_color_vbo(color_vbo_);
	}

	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param> shader_point_sprite_param_;
	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param> shader_simple_color_param_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* color_vbo_;
	bool render_vertices_;
	bool render_edges_;
	QColor vertex_color_;
	QColor edge_color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;
};

} // namespace plugin_polyline_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_POLYLINE_RENDER_MAP_PARAMETERS__H_
