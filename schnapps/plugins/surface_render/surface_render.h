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

#include <surface_render_dock_tab.h>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>
#include <cgogn/rendering/shaders/shader_phong.h>

#include <QAction>

namespace schnapps
{

class MapHandlerGen;

struct MapParameters
{
	enum FaceShadingStyle
	{
		FLAT = 0,
		PHONG
	};

	MapParameters() :
		shader_flat_param_(nullptr),
		shader_simple_color_param_(nullptr),
		position_vbo_(nullptr),
		normal_vbo_(nullptr),
		color_vbo_(nullptr),
		verticesScaleFactor(1.0f),
		basePSradius(1.0f),
		render_vertices_(false),
		render_edges_(false),
		render_faces_(true),
		render_back_faces_(true),
		render_boundary_(false),
		face_style_(FLAT),
		vertex_color_(190, 85, 168),
		edge_color_(0, 0, 0),
		front_color_(85, 168, 190),
		back_color_(85, 168, 190)
	{
		shader_flat_param_ = cgogn::rendering::ShaderFlat::generate_param();
		shader_flat_param_->front_color_ = front_color_;
		shader_flat_param_->back_color_ = back_color_;

		shader_flat_color_param_ = cgogn::rendering::ShaderFlatColor::generate_param();

		shader_simple_color_param_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_->color_ = edge_color_;

		shader_phong_param_ = cgogn::rendering::ShaderPhong::generate_param();
		shader_phong_param_->front_color_ = front_color_;
		shader_phong_param_->back_color_ = back_color_;
		shader_phong_param_->double_side_ = render_back_faces_;

		shader_phong_color_param_ = cgogn::rendering::ShaderPhongColor::generate_param();
		shader_phong_color_param_->double_side_ = render_back_faces_;
	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_)
		{
			shader_flat_param_->set_position_vbo(position_vbo_);
			shader_flat_color_param_->set_position_vbo(position_vbo_);
			shader_simple_color_param_->set_position_vbo(position_vbo_);
			shader_phong_param_->set_position_vbo(position_vbo_);
			shader_phong_color_param_->set_position_vbo(position_vbo_);
		}
	}

	cgogn::rendering::VBO* get_normal_vbo() const { return normal_vbo_; }
	void set_normal_vbo(cgogn::rendering::VBO* v)
	{
		normal_vbo_ = v;
		if (normal_vbo_)
		{
			shader_phong_param_->set_normal_vbo(normal_vbo_);
			shader_phong_color_param_->set_normal_vbo(normal_vbo_);
		}
	}

	cgogn::rendering::VBO* get_color_vbo() const { return color_vbo_; }
	void set_color_vbo(cgogn::rendering::VBO* v)
	{
		color_vbo_ = v;
		if (color_vbo_)
		{
			shader_flat_color_param_->set_color_vbo(color_vbo_);
			shader_phong_color_param_->set_color_vbo(color_vbo_);
		}
	}

	const QColor& get_vertex_color() const { return vertex_color_; }
	void set_vertex_color(const QColor& c)
	{
		vertex_color_ = c;
	}

	const QColor& get_edge_color() const { return edge_color_; }
	void set_edge_color(const QColor& c)
	{
		edge_color_ = c;
		shader_simple_color_param_->color_ = edge_color_;
	}

	const QColor& get_front_color() const { return front_color_; }
	void set_front_color(const QColor& c)
	{
		front_color_ = c;
		shader_flat_param_->front_color_ = front_color_;
	}

	const QColor& get_back_color() const { return back_color_; }
	void set_back_color(const QColor& c)
	{
		back_color_ = c;
		shader_flat_param_->back_color_ = back_color_;
	}

	void set_render_back_face(bool b)
	{
		render_back_faces_ = b;
		shader_phong_param_->double_side_ = b;
		shader_phong_color_param_->double_side_ = b;
	}

private:

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* normal_vbo_;
	cgogn::rendering::VBO* color_vbo_;

	QColor vertex_color_;
	QColor edge_color_;
	QColor front_color_;
	QColor back_color_;

public:

	cgogn::rendering::ShaderFlat::Param* shader_flat_param_;
	cgogn::rendering::ShaderFlatColor::Param* shader_flat_color_param_;
	cgogn::rendering::ShaderSimpleColor::Param* shader_simple_color_param_;
	cgogn::rendering::ShaderPhong::Param* shader_phong_param_;
	cgogn::rendering::ShaderPhongColor::Param* shader_phong_color_param_;

	float verticesScaleFactor;
	float basePSradius;

	bool render_vertices_;
	bool render_edges_;
	bool render_faces_;

	bool render_back_faces_;
	bool render_boundary_;

	FaceShadingStyle face_style_;
};

/**
* @brief Plugin for surface rendering
*/
class Plugin_SurfaceRender : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class SurfaceRender_DockTab;

public:

	inline Plugin_SurfaceRender() {}

	~Plugin_SurfaceRender() {}

private:

	MapParameters& get_parameters(View* view, MapHandlerGen* map);

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override {}
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	inline void keyPress(View* , QKeyEvent*) override {}
	inline void keyRelease(View* , QKeyEvent*) override {}
	inline void mousePress(View* , QMouseEvent*) override {}
	inline void mouseRelease(View* , QMouseEvent*) override {}
	inline void mouseMove(View* , QMouseEvent*) override {}
	inline void wheelEvent(View* , QWheelEvent*) override {}

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

	SurfaceRender_DockTab* dock_tab_;
	QHash<View*, QHash<MapHandlerGen*, MapParameters>> parameter_set_;

	cgogn::rendering::ShaderFlat* shader_flat_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
