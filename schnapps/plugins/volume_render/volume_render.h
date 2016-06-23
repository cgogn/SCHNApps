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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_H_

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>

#include <volume_render_dock_tab.h>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/shaders/shader_explode_volumes.h>
#include <cgogn/rendering/shaders/shader_explode_volumes_line.h>

#include <QAction>
#include <map>

namespace schnapps
{

class MapHandlerGen;
class Plugin_VolumeRender;

struct MapParameters
{
	friend class Plugin_VolumeRender;

	MapParameters() :
//		shader_flat_param_(nullptr),
//		shader_flat_color_param_(nullptr),
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
//		explode_faces_(1),
		volume_explode_factor_(0.5f)
	{
//		shader_flat_param_ = cgogn::rendering::ShaderFlat::generate_param();
//		shader_flat_param_->front_color_ = face_color_;

//		shader_flat_color_param_ = cgogn::rendering::ShaderFlatColor::generate_param();

		shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_->color_ = edge_color_;

		shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_->color_ = vertex_color_;
		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;

		shader_expl_vol_ = cgogn::rendering::ShaderExplodeVolumes::generate_param();
		shader_expl_vol_->color_ = face_color_;
		shader_expl_vol_->explode_factor_ = volume_explode_factor_;

		shader_expl_vol_line_ = cgogn::rendering::ShaderExplodeVolumesLine::generate_param();
		shader_expl_vol_line_->color_ = edge_color_;
		shader_expl_vol_line_->explode_factor_ = volume_explode_factor_;

		shader_expl_vol_col_= cgogn::rendering::ShaderExplodeVolumesColor::generate_param();
		shader_expl_vol_col_->explode_factor_ = volume_explode_factor_;

	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }

	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_)
		{
//			shader_flat_param_->set_position_vbo(position_vbo_);
//			shader_flat_color_param_->set_position_vbo(position_vbo_);
			shader_expl_vol_->set_position_vbo(position_vbo_);
			shader_expl_vol_col_->set_position_vbo(position_vbo_);
			shader_expl_vol_line_->set_position_vbo(position_vbo_);
			shader_simple_color_param_->set_position_vbo(position_vbo_);
			shader_point_sprite_param_->set_position_vbo(position_vbo_);
		}
	}

	cgogn::rendering::VBO* get_color_vbo() const { return color_vbo_; }
	void set_color_vbo(cgogn::rendering::VBO* v)
	{
		color_vbo_ = v;
		if (color_vbo_)
		{
//			shader_flat_color_param_->set_color_vbo(color_vbo_);
			shader_expl_vol_col_->set_color_vbo(color_vbo_);
		}

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
//		shader_flat_param_->front_color_ = face_color_;
		shader_expl_vol_->color_ = face_color_;
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
		shader_expl_vol_->explode_factor_ = volume_explode_factor_;
		shader_expl_vol_line_->explode_factor_ = volume_explode_factor_;
		shader_expl_vol_col_->explode_factor_ = volume_explode_factor_;
	}

private:

//	std::unique_ptr<cgogn::rendering::ShaderFlat::Param>		shader_flat_param_;
//	std::unique_ptr<cgogn::rendering::ShaderFlatColor::Param>	shader_flat_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param>	shader_simple_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_;
	std::unique_ptr<cgogn::rendering::ShaderExplodeVolumes::Param>		shader_expl_vol_;
	std::unique_ptr<cgogn::rendering::ShaderExplodeVolumesColor::Param>	shader_expl_vol_col_;
	std::unique_ptr<cgogn::rendering::ShaderExplodeVolumesLine::Param>	shader_expl_vol_line_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* color_vbo_;

	QColor vertex_color_;
	QColor edge_color_;
	QColor face_color_;

	float32 vertex_scale_factor_;
	float32 vertex_base_size_;

//	float64 explode_faces_;
	float32 volume_explode_factor_;

public:

	bool render_vertices_;
	bool render_edges_;
	bool render_faces_;
	bool render_boundary_;

private:


};

/**
* @brief Plugin for surface rendering
*/
class Plugin_VolumeRender : public PluginInteraction
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

	inline void view_linked(View*) override {}
	inline void view_unlinked(View*) override {}

private slots:

	// slots called from SCHNApps signals
	void selected_view_changed(View*, View*);
	void selected_map_changed(MapHandlerGen*, MapHandlerGen*);
	void map_added(MapHandlerGen* map);
	void map_removed(MapHandlerGen* map);
	void schnapps_closing();

	// slots called from MapHandler signals
	void vbo_added(cgogn::rendering::VBO* vbo);
	void vbo_removed(cgogn::rendering::VBO* vbo);
	void bb_changed();

public slots:



private:

	VolumeRender_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
