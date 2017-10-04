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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_MAP_PARAMETERS__H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_MAP_PARAMETERS__H_

#include "dll.h"

#include <schnapps/core/types.h>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>
#include <cgogn/rendering/shaders/shader_phong.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#ifdef USE_TRANSPARENCY
#include <cgogn/rendering/transparency_shaders/shader_transparent_flat.h>
#include <cgogn/rendering/transparency_shaders/shader_transparent_phong.h>
#endif

namespace schnapps
{

namespace plugin_surface_render
{

class Plugin_SurfaceRender;

struct SCHNAPPS_PLUGIN_SURFACE_RENDER_API MapParameters
{
	friend class Plugin_SurfaceRender;

	enum FaceShadingStyle
	{
		FLAT = 0,
		PHONG
	};

	MapParameters() :
		shader_flat_param_(nullptr),
		shader_flat_color_param_(nullptr),
		shader_simple_color_param_(nullptr),
		shader_simple_color_param_boundary_(nullptr),
		shader_phong_param_(nullptr),
		shader_phong_color_param_(nullptr),
		shader_point_sprite_param_(nullptr),
#ifdef USE_TRANSPARENCY
		shader_transp_flat_param_(nullptr),
		shader_transp_phong_param_(nullptr),
#endif
		position_vbo_(nullptr),
		normal_vbo_(nullptr),
		color_vbo_(nullptr),
		vertex_color_(190, 85, 168),
		edge_color_(0, 0, 0),
		front_color_(85, 168, 190, 127),
		back_color_(85, 168, 190, 127),
		render_backfaces_(true),
		vertex_scale_factor_(1.0f),
		vertex_base_size_(1.0f),
		transparency_factor_(127),
		render_vertices_(false),
		render_edges_(false),
		render_faces_(true),
		render_boundary_(false),
		use_transparency_(false),
		face_style_(FLAT)
	{
		initialize_gl();
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	inline cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	inline cgogn::rendering::VBO* get_normal_vbo() const { return normal_vbo_; }
	inline cgogn::rendering::VBO* get_color_vbo() const { return color_vbo_; }
	inline bool get_render_vertices() const { return render_vertices_; }
	inline bool get_render_edges() const { return render_edges_; }
	inline bool get_render_faces() const { return render_faces_; }
	inline bool get_render_backfaces() const { return render_backfaces_; }
	inline FaceShadingStyle get_face_style() const { return face_style_; }
	inline bool get_render_boundary() const { return render_boundary_; }
	inline const QColor& get_vertex_color() const { return vertex_color_; }
	inline const QColor& get_edge_color() const { return edge_color_; }
	inline const QColor& get_front_color() const { return front_color_; }
	inline const QColor& get_back_color() const { return back_color_; }
	inline float32 get_vertex_base_size() const { return vertex_base_size_; }
	inline float32 get_vertex_scale_factor() const { return vertex_scale_factor_; }
	inline bool get_transparency_enabled() const { return use_transparency_; }
	inline int32 get_transparency_factor() const { return transparency_factor_; }

#ifdef USE_TRANSPARENCY
	inline cgogn::rendering::ShaderFlatTransp::Param* get_transp_flat_param() const
	{
		return shader_transp_flat_param_.get();
	}

	inline cgogn::rendering::ShaderPhongTransp::Param* get_transp_phong_param() const
	{
		return shader_transp_phong_param_.get();
	}
#endif

private:

	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_ && position_vbo_->vector_dimension() == 3)
		{
			shader_flat_param_->set_position_vbo(position_vbo_);
			shader_flat_color_param_->set_position_vbo(position_vbo_);
			shader_simple_color_param_->set_position_vbo(position_vbo_);
			shader_simple_color_param_boundary_->set_position_vbo(position_vbo_);
			shader_phong_param_->set_position_vbo(position_vbo_);
			shader_phong_color_param_->set_position_vbo(position_vbo_);
			shader_point_sprite_param_->set_position_vbo(position_vbo_);
#ifdef USE_TRANSPARENCY
			shader_transp_flat_param_->set_position_vbo(position_vbo_);
			shader_transp_phong_param_->set_position_vbo(position_vbo_);
#endif
		}
		else
			position_vbo_ = nullptr;
	}

	void set_normal_vbo(cgogn::rendering::VBO* v)
	{
		normal_vbo_ = v;
		if (normal_vbo_ && normal_vbo_->vector_dimension() == 3)
		{
			shader_phong_param_->set_normal_vbo(normal_vbo_);
			shader_phong_color_param_->set_normal_vbo(normal_vbo_);
#ifdef USE_TRANSPARENCY
			shader_transp_phong_param_->set_normal_vbo(normal_vbo_);
#endif
		}
		else
			normal_vbo_ = nullptr;
	}

	void set_color_vbo(cgogn::rendering::VBO* v)
	{
		color_vbo_ = v;
		if (color_vbo_ && color_vbo_->vector_dimension() == 3)
		{
			shader_flat_color_param_->set_color_vbo(color_vbo_);
			shader_phong_color_param_->set_color_vbo(color_vbo_);
		}
		else
			color_vbo_ = nullptr;
	}

	void set_render_vertices(bool b) { render_vertices_ = b; }

	void set_render_edges(bool b) { render_edges_ = b; }

	void set_render_faces(bool b) { render_faces_ = b; }

	void set_render_backfaces(bool b)
	{
		render_backfaces_ = b;
		shader_phong_param_->double_side_ = b;
		shader_phong_color_param_->double_side_ = b;
		shader_flat_param_->bf_culling_ = !b;
#ifdef USE_TRANSPARENCY
		shader_transp_phong_param_->bf_culling_ = !b;
		shader_transp_flat_param_->bf_culling_ = !b;
#endif
	}

	void set_face_style(FaceShadingStyle fs) { face_style_ = fs; }

	void set_render_boundary(bool b) { render_boundary_ = b; }

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

	void set_front_color(const QColor& c)
	{
		front_color_ = c;
		shader_flat_param_->front_color_ = front_color_;
		shader_phong_param_->front_color_ = front_color_;
#ifdef USE_TRANSPARENCY
		shader_transp_flat_param_->front_color_ = front_color_;
		shader_transp_phong_param_->front_color_ = front_color_;
		shader_transp_flat_param_->set_alpha(transparency_factor_);
		shader_transp_phong_param_->set_alpha(transparency_factor_);
#endif
	}

	void set_back_color(const QColor& c)
	{
		back_color_ = c;
		shader_flat_param_->back_color_ = back_color_;
		shader_phong_param_->back_color_ = back_color_;
#ifdef USE_TRANSPARENCY
		shader_transp_flat_param_->back_color_ = back_color_;
		shader_transp_phong_param_->back_color_ = back_color_;
		shader_transp_flat_param_->set_alpha(transparency_factor_);
		shader_transp_phong_param_->set_alpha(transparency_factor_);
#endif
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

	inline void set_transparency_enabled(bool b)
	{
#ifdef USE_TRANSPARENCY
		use_transparency_ = b;
		if (use_transparency_)
		{
			transparency_factor_ = transparency_factor_ % 255;
			shader_transp_flat_param_->set_alpha(transparency_factor_);
			shader_transp_phong_param_->set_alpha(transparency_factor_);
		}
#endif
	}

	void set_transparency_factor(int32 n)
	{
#ifdef USE_TRANSPARENCY
		n = n % 255;
		transparency_factor_ = n;
		if (use_transparency_)
		{
			shader_transp_flat_param_->set_alpha(transparency_factor_);
			shader_transp_phong_param_->set_alpha(transparency_factor_);
		}
#endif
	}

	inline void initialize_gl()
	{
		shader_flat_param_ = cgogn::rendering::ShaderFlat::generate_param();
		shader_flat_param_->front_color_ = front_color_;
		shader_flat_param_->back_color_ = back_color_;

		shader_flat_color_param_ = cgogn::rendering::ShaderFlatColor::generate_param();

		shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_->color_ = edge_color_;

		shader_simple_color_param_boundary_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_boundary_->color_ = QColor(200, 200, 25);

		shader_phong_param_ = cgogn::rendering::ShaderPhong::generate_param();
		shader_phong_param_->front_color_ = front_color_;
		shader_phong_param_->back_color_ = back_color_;
		shader_phong_param_->double_side_ = render_backfaces_;

		shader_phong_color_param_ = cgogn::rendering::ShaderPhongColor::generate_param();
		shader_phong_color_param_->double_side_ = render_backfaces_;

		shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_->color_ = vertex_color_;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;

#ifdef USE_TRANSPARENCY
		shader_transp_flat_param_ = cgogn::rendering::ShaderFlatTransp::generate_param();
		shader_transp_flat_param_->front_color_ = front_color_;
		shader_transp_flat_param_->back_color_ = back_color_;
		shader_transp_flat_param_->set_alpha(transparency_factor_);

		shader_transp_phong_param_ = cgogn::rendering::ShaderPhongTransp::generate_param();
		shader_transp_phong_param_->front_color_ = front_color_;
		shader_transp_phong_param_->back_color_ = back_color_;
		shader_transp_flat_param_->set_alpha(transparency_factor_);
#endif

		set_position_vbo(position_vbo_);
		set_normal_vbo(normal_vbo_);
		set_color_vbo(color_vbo_);
	}

	std::unique_ptr<cgogn::rendering::ShaderFlat::Param>        shader_flat_param_;
	std::unique_ptr<cgogn::rendering::ShaderFlatColor::Param>   shader_flat_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param> shader_simple_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param> shader_simple_color_param_boundary_;
	std::unique_ptr<cgogn::rendering::ShaderPhong::Param>       shader_phong_param_;
	std::unique_ptr<cgogn::rendering::ShaderPhongColor::Param>  shader_phong_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param> shader_point_sprite_param_;
#ifdef USE_TRANSPARENCY
	std::unique_ptr<cgogn::rendering::ShaderFlatTransp::Param>  shader_transp_flat_param_;
	std::unique_ptr<cgogn::rendering::ShaderPhongTransp::Param> shader_transp_phong_param_;
#endif

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* normal_vbo_;
	cgogn::rendering::VBO* color_vbo_;
	bool render_vertices_;
	bool render_edges_;
	bool render_faces_;
	bool render_backfaces_;
	FaceShadingStyle face_style_;
	bool render_boundary_;
	QColor vertex_color_;
	QColor edge_color_;
	QColor front_color_;
	QColor back_color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;
	bool use_transparency_;
	int32 transparency_factor_;
};

} // namespace plugin_surface_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_MAP_PARAMETERS__H_
