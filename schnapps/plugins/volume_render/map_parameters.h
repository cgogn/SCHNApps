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

#ifndef SCHNAPPS_PLUGIN_VOLUME_RENDER_MAP_PARAMETERS_H_
#define SCHNAPPS_PLUGIN_VOLUME_RENDER_MAP_PARAMETERS_H_

#include <schnapps/plugins/volume_render/dll.h>

#include <schnapps/plugins/cmap3_provider/cmap3_provider.h>

#include <schnapps/core/types.h>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/volume_drawer.h>
#include <cgogn/rendering/frame_manipulator.h>
#include <cgogn/rendering/topo_drawer.h>
#ifdef USE_TRANSPARENCY
#include <cgogn/rendering/transparency_volume_drawer.h>
#endif

namespace schnapps
{

namespace plugin_volume_render
{

class Plugin_VolumeRender;
using CMap3Handler = plugin_cmap3_provider::CMap3Handler;

struct SCHNAPPS_PLUGIN_VOLUME_RENDER_API MapParameters
{
	friend class Plugin_VolumeRender;

	MapParameters() :
		mh_(nullptr),
	#ifdef USE_TRANSPARENCY
		volume_transparency_drawer_(nullptr),
		volume_transparency_drawer_rend_(nullptr),
	#endif
		position_vbo_(nullptr),
		render_vertices_(false),
		render_edges_(false),
		render_faces_(true),
		render_topology_(false),
		apply_clipping_plane_(false),
		clipping_plane_initialized_(false),
		vertex_color_(190, 85, 168),
		edge_color_(0, 0, 0),
		face_color_(85, 168, 190),
		vertex_scale_factor_(1),
		vertex_base_size_(1),
		volume_explode_factor_(0.8f),
		use_transparency_(false),
		transparency_factor_(127),
		filter_(nullptr)
	{
		initialize_gl();
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	inline cgogn::rendering::VBO* position_vbo() const { return position_vbo_; }
	inline bool render_vertices() const { return render_vertices_; }
	inline bool render_edges() const { return render_edges_; }
	inline bool render_faces() const { return render_faces_; }
	inline bool render_topology() const { return render_topology_; }
	inline bool apply_clipping_plane() const { return apply_clipping_plane_; }
	inline const QColor& vertex_color() const { return vertex_color_; }
	inline const QColor& edge_color() const { return edge_color_; }
	inline const QColor& face_color() const { return face_color_; }
	inline float32 vertex_base_size() const { return vertex_base_size_; }
	inline float32 vertex_scale_factor() const { return vertex_scale_factor_; }
	inline float32 volume_explode_factor() const { return volume_explode_factor_; }
	inline bool transparency_enabled() const { return use_transparency_; }
	inline int32 transparency_factor() const { return transparency_factor_; }

#ifdef USE_TRANSPARENCY
	inline cgogn::rendering::VolumeTransparencyDrawer::Renderer* transp_drawer_rend()
	{
		return volume_transparency_drawer_rend_.get();
	}
#endif

private:

	void set_filter(cgogn::CellFilters* f)
	{
		filter_ = f;
	}

	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_ && position_vbo_->vector_dimension() == 3)
		{
			shader_simple_color_param_->set_position_vbo(position_vbo_);
			shader_point_sprite_param_->set_position_vbo(position_vbo_);
			update_volume_drawer();
			update_topo_drawer();
		}
		else
			position_vbo_ = nullptr;
	}

	void set_render_vertices(bool b) { render_vertices_ = b; }

	void set_render_edges(bool b) { render_edges_ = b; }

	void set_render_faces(bool b) { render_faces_ = b; }

	void set_render_topology(bool b)
	{
		render_topology_ = b;
		if (render_topology_)
			update_topo_drawer();
	}

	void set_vertex_color(const QColor& c)
	{
		vertex_color_ = c;
		shader_point_sprite_param_->color_ = vertex_color_;
	}

	void set_edge_color(const QColor& c)
	{
		edge_color_ = c;
		shader_simple_color_param_->color_ = edge_color_;
		volume_drawer_rend_->set_edge_color(c);
	}

	void set_face_color(const QColor& c)
	{
		face_color_ = c;
		volume_drawer_rend_->set_face_color(face_color_);
#ifdef USE_TRANSPARENCY
		face_color_.setAlpha(transparency_factor_);
		volume_transparency_drawer_rend_->set_color(face_color_);
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

	void set_volume_explode_factor(float32 vef)
	{
		volume_explode_factor_ = vef;
		volume_drawer_rend_->set_explode_volume(vef);
#ifdef USE_TRANSPARENCY
		volume_transparency_drawer_rend_->set_explode_volume(vef);
#endif
		topo_drawer_->set_explode_volume(vef);
		if (render_topology_)
			update_topo_drawer();
	}

	void set_transparency_enabled(bool b)
	{
#ifdef USE_TRANSPARENCY
		use_transparency_ = b;
		if (use_transparency_)
		{
			transparency_factor_ = transparency_factor_ % 255;
			face_color_.setAlpha(transparency_factor_);
		}
		else
			face_color_.setAlpha(255);
		set_face_color(face_color_);
#endif
	}

	void set_transparency_factor(int32 n)
	{
#ifdef USE_TRANSPARENCY
		n = n % 255;
		transparency_factor_ = n;
		if (use_transparency_)
		{
			face_color_.setAlpha(n);
			volume_transparency_drawer_rend_->set_color(face_color_);
		}
#endif
	}

	void set_apply_clipping_plane(bool b)
	{
		apply_clipping_plane_ = b;
		if (apply_clipping_plane_)
			update_clipping_plane();
		else
		{
			volume_drawer_rend_->set_clipping_plane(QVector4D(0, 0, 0, 0));
			topo_drawer_rend_->set_clipping_plane(QVector4D(0, 0, 0, 0));
#ifdef USE_TRANSPARENCY
			volume_transparency_drawer_rend_->set_clipping_plane(QVector4D(0, 0, 0, 0));
#endif
		}
	}

	void update_clipping_plane()
	{
		if (!clipping_plane_initialized_)
		{
			clipping_plane_initialized_ = true;
			frame_manip_->set_size(mh_->bb_diagonal_size() / 12.0f);
			frame_manip_->set_position(mh_->bb().max());
			frame_manip_->z_plane_param(QColor(200, 200, 200), 0.0f, 0.0f, 3.0f);
		}

		VEC3F position;
		frame_manip_->get_position(position);
		VEC3F z_axis;
		frame_manip_->get_axis(cgogn::rendering::FrameManipulator::Zt, z_axis);
		float32 d = -(position.dot(z_axis));

		volume_drawer_rend_->set_clipping_plane(QVector4D(z_axis[0], z_axis[1], z_axis[2], d));
		topo_drawer_rend_->set_clipping_plane(QVector4D(z_axis[0], z_axis[1], z_axis[2], d));
#ifdef USE_TRANSPARENCY
		volume_transparency_drawer_rend_->set_clipping_plane(QVector4D(z_axis[0], z_axis[1], z_axis[2], d));
#endif
	}

	void update_topo_drawer()
	{
		if (position_vbo_)
		{
			const CMap3::VertexAttribute<VEC3>& pos_attr = mh_->map()->get_attribute<VEC3, CMap3::Vertex::ORBIT>(position_vbo_->name());
			if (!pos_attr.is_valid())
			{
				cgogn_log_warning("plugin_volume_render|MapParameters::update_topo_drawer") << "The attribute \"" << position_vbo_->name() << "\" is not valid. Its data should be of type " << cgogn::name_of_type(VEC3()) << ".";
				position_vbo_ = nullptr;
				return;
			}

			topo_drawer_->update(*mh_->map(), pos_attr);
		}
	}

	void update_volume_drawer()
	{
		if (position_vbo_)
		{
			const CMap3::VertexAttribute<VEC3>& pos_attr = mh_->map()->get_attribute<VEC3, CMap3::Vertex::ORBIT>(position_vbo_->name());
			if (!pos_attr.is_valid())
			{
				cgogn_log_warning("plugin_volume_render|MapParameters::update_volume_drawer") << "The attribute \"" << position_vbo_->name() << "\" is not valid. Its data should be of type " << cgogn::name_of_type(VEC3()) << ".";
				position_vbo_ = nullptr;
				return;
			}

			if(filter_ == nullptr)
			{
				volume_drawer_->update_edge(*mh_->map(), pos_attr);
				volume_drawer_->update_face(*mh_->map(), pos_attr);
			}
			else
			{
				volume_drawer_->update_edge(*mh_->map(), *filter_, pos_attr);
				volume_drawer_->update_face(*mh_->map(), *filter_, pos_attr);
			}

#ifdef USE_TRANSPARENCY
			volume_transparency_drawer_->update_face(*mh_->map(), pos_attr);
#endif
		}
	}

	void initialize_gl()
	{
		shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_->color_ = edge_color_;

		shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_->color_ = vertex_color_;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;

		volume_drawer_ = cgogn::make_unique<cgogn::rendering::VolumeDrawer>();
		volume_drawer_rend_ = volume_drawer_->generate_renderer();

		topo_drawer_ =  cgogn::make_unique<cgogn::rendering::TopoDrawer>();
		topo_drawer_rend_ = topo_drawer_->generate_renderer();

#ifdef USE_TRANSPARENCY
		volume_transparency_drawer_ = cgogn::make_unique<cgogn::rendering::VolumeTransparencyDrawer>();
		volume_transparency_drawer_rend_ = volume_transparency_drawer_->generate_renderer();
		volume_transparency_drawer_rend_->set_explode_volume(volume_explode_factor_);
		volume_transparency_drawer_rend_->set_lighted(true);
#endif

		frame_manip_ = cgogn::make_unique<cgogn::rendering::FrameManipulator>();

		set_position_vbo(position_vbo_);
		set_vertex_color(vertex_color_);
		set_edge_color(edge_color_);
		set_face_color(face_color_);
		set_vertex_scale_factor(vertex_scale_factor_);
		set_vertex_base_size(vertex_base_size_);
		set_volume_explode_factor(volume_explode_factor_);
		set_transparency_factor(transparency_factor_);
	}

	CMap3Handler* mh_;

	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param>	shader_simple_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_;
#ifdef USE_TRANSPARENCY
	std::unique_ptr<cgogn::rendering::VolumeTransparencyDrawer> volume_transparency_drawer_;
	std::unique_ptr<cgogn::rendering::VolumeTransparencyDrawer::Renderer> volume_transparency_drawer_rend_;
#endif

	std::unique_ptr<cgogn::rendering::VolumeDrawer> volume_drawer_;
	std::unique_ptr<cgogn::rendering::VolumeDrawer::Renderer> volume_drawer_rend_;

	std::unique_ptr<cgogn::rendering::TopoDrawer> topo_drawer_;
	std::unique_ptr<cgogn::rendering::TopoDrawer::Renderer> topo_drawer_rend_;

	std::unique_ptr<cgogn::rendering::FrameManipulator> frame_manip_;

	cgogn::rendering::VBO* position_vbo_;
	bool render_vertices_;
	bool render_edges_;
	bool render_faces_;
	bool render_topology_;
	bool apply_clipping_plane_;
	bool clipping_plane_initialized_;
	QColor vertex_color_;
	QColor edge_color_;
	QColor face_color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;
	float32 volume_explode_factor_;
	bool use_transparency_;
	int32 transparency_factor_;
	cgogn::CellFilters* filter_;
};

} // namespace plugin_volume_render

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_RENDER_MAP_PARAMETERS_H_
