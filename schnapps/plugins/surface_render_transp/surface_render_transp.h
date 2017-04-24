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

#include <surface_render_transp_dock_tab.h>

#include <cgogn/rendering/transparency_drawer.h>
#include <cgogn/rendering/transparency_shaders/shader_transparent_flat.h>
#include <cgogn/rendering/transparency_shaders/shader_transparent_phong.h>

#include <QAction>
#include <map>

namespace schnapps
{

namespace plugin_surface_render_transp
{

class Plugin_SurfaceRenderTransp;

struct SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API MapParameters
{
	friend class Plugin_SurfaceRenderTransp;

	enum FaceShadingStyle
	{
		FLAT = 0,
		PHONG
	};

	MapParameters() :
		shader_flat_param_(nullptr),
		shader_phong_param_(nullptr),
		position_vbo_(nullptr),
		normal_vbo_(nullptr),
		front_color_(85, 168, 190),
		back_color_(85, 168, 190),
		face_style_(FLAT)
	{
		initialize_gl();
	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_ && position_vbo_->vector_dimension() == 3)
		{
			shader_flat_param_->set_position_vbo(position_vbo_);
			shader_phong_param_->set_position_vbo(position_vbo_);
		}
		else
			position_vbo_ = nullptr;
	}

	cgogn::rendering::VBO* get_normal_vbo() const { return normal_vbo_; }
	void set_normal_vbo(cgogn::rendering::VBO* v)
	{
		normal_vbo_ = v;
		if (normal_vbo_ && normal_vbo_->vector_dimension() == 3)
		{
			shader_phong_param_->set_normal_vbo(normal_vbo_);
		}
		else
			normal_vbo_ = nullptr;
	}

	const QColor& get_front_color() const { return front_color_; }
	void set_front_color(const QColor& c)
	{
		front_color_ = c;
		shader_flat_param_->front_color_ = front_color_;
		shader_phong_param_->front_color_ = front_color_;
	}

	const QColor& get_back_color() const { return back_color_; }
	void set_back_color(const QColor& c)
	{
		back_color_ = c;
		shader_flat_param_->back_color_ = back_color_;
		shader_phong_param_->back_color_ = back_color_;
	}

private:

	inline void initialize_gl()
	{
		shader_flat_param_ = cgogn::rendering::ShaderFlatTransp::generate_param();
		shader_flat_param_->front_color_ = front_color_;
		shader_flat_param_->back_color_ = back_color_;

		shader_phong_param_ = cgogn::rendering::ShaderPhongTransp::generate_param();
		shader_phong_param_->front_color_ = front_color_;
		shader_phong_param_->back_color_ = back_color_;

		set_position_vbo(position_vbo_);
		set_normal_vbo(normal_vbo_);
	}

	std::unique_ptr<cgogn::rendering::ShaderFlatTransp::Param>		shader_flat_param_;
	std::unique_ptr<cgogn::rendering::ShaderPhongTransp::Param>		shader_phong_param_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* normal_vbo_;

	QColor front_color_;
	QColor back_color_;

public:
	FaceShadingStyle face_style_;
};

/**
* @brief Plugin for surface rendering
*/
class SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_API Plugin_SurfaceRenderTransp: public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class SurfaceRenderTransp_DockTab;

public:

	inline Plugin_SurfaceRenderTransp() {}

	~Plugin_SurfaceRenderTransp() override;

private:

	MapParameters& get_parameters(View* view, MapHandlerGen* map);

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

	// slots called from View signals
	void map_linked(MapHandlerGen* map);
	void map_unlinked(MapHandlerGen* map);

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void viewer_initialized();
	void enable_on_selected_view(Plugin* p);

	void update_dock_tab();

public slots:

	void set_render_vertices(View* view, MapHandlerGen* map, bool b);
	inline void set_render_vertices(const QString& view_name, const QString& map_name, bool b)
	{
		set_render_vertices(schnapps_->get_view(view_name), schnapps_->get_map(map_name), b);
	}

	void set_render_edges(View* view, MapHandlerGen* map, bool b);
	inline void set_render_edges(const QString& view_name, const QString& map_name, bool b)
	{
		set_render_edges(schnapps_->get_view(view_name), schnapps_->get_map(map_name), b);
	}

	void set_render_faces(View* view, MapHandlerGen* map, bool b);
	inline void set_render_faces(const QString& view_name, const QString& map_name, bool b)
	{
		set_render_faces(schnapps_->get_view(view_name), schnapps_->get_map(map_name), b);
	}

	void set_render_boundary(View* view, MapHandlerGen* map, bool b);
	inline void set_render_boundary(const QString& view_name, const QString& map_name, bool b)
	{
		set_render_boundary(schnapps_->get_view(view_name), schnapps_->get_map(map_name), b);
	}

	void set_face_style(View* view, MapHandlerGen* map, MapParameters::FaceShadingStyle s);
	inline void set_face_style(const QString& view_name, const QString& map_name, MapParameters::FaceShadingStyle s)
	{
		set_face_style(schnapps_->get_view(view_name), schnapps_->get_map(map_name), s);
	}

	void set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void set_position_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_position_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
	}

	void set_normal_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void set_normal_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_normal_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
	}

	void set_color_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void set_color_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_color_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
	}

	void set_vertex_color(View* view, MapHandlerGen* map, const QColor& color);
	inline void set_vertex_color(const QString& view_name, const QString& map_name, const QColor& color)
	{
		set_vertex_color(schnapps_->get_view(view_name), schnapps_->get_map(map_name), color);
	}

	void set_edge_color(View* view, MapHandlerGen* map, const QColor& color);
	inline void set_edge_color(const QString& view_name, const QString& map_name, const QColor& color)
	{
		set_edge_color(schnapps_->get_view(view_name), schnapps_->get_map(map_name), color);
	}

	void set_front_color(View* view, MapHandlerGen* map, const QColor& color);
	inline void set_front_color(const QString& view_name, const QString& map_name, const QColor& color)
	{
		set_front_color(schnapps_->get_view(view_name), schnapps_->get_map(map_name), color);
	}

	void set_back_color(View* view, MapHandlerGen* map, const QColor& color);
	inline void set_back_color(const QString& view_name, const QString& map_name, const QColor& color)
	{
		set_back_color(schnapps_->get_view(view_name), schnapps_->get_map(map_name), color);
	}

	void set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf);
	void set_vertex_scale_factor(const QString& view_name, const QString& map_name, float32 sf)
	{
		set_vertex_scale_factor(schnapps_->get_view(view_name), schnapps_->get_map(map_name), sf);
	}

private:

	SurfaceRenderTransp_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;

	std::map<View*, cgogn::rendering::SurfaceTransparencyDrawer*> transp_drawer_set_;

	bool setting_auto_enable_on_selected_view_;
	QString setting_auto_load_position_attribute_;
	QString setting_auto_load_normal_attribute_;
	QString setting_auto_load_color_attribute_;
};

} // namespace plugin_surface_render_transp_transp

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_TRANSP_H_
