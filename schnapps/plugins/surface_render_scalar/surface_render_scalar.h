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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>

#include <surface_render_scalar_dock_tab.h>

#include <cgogn/rendering/shaders/shader_scalar_per_vertex.h>

#include <QAction>

namespace schnapps
{

class Plugin_SurfaceRenderScalar;

struct MapParameters
{
	friend class Plugin_SurfaceRenderScalar;

	MapParameters() :
		position_vbo_(nullptr),
		scalar_vbo_(nullptr),
		color_map_(cgogn::rendering::ShaderScalarPerVertex::BWR),
		expansion_(0),
		show_iso_lines_(false)
	{
		shader_scalar_per_vertex_param_ = cgogn::rendering::ShaderScalarPerVertex::generate_param();
		shader_scalar_per_vertex_param_->color_map_ = color_map_;
		shader_scalar_per_vertex_param_->expansion_ = expansion_;
		shader_scalar_per_vertex_param_->min_value_ = 0.0f;
		shader_scalar_per_vertex_param_->max_value_ = 1.0f;
		shader_scalar_per_vertex_param_->show_iso_lines_ = show_iso_lines_;
	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_)
			shader_scalar_per_vertex_param_->set_position_vbo(position_vbo_);
	}

	cgogn::rendering::VBO* get_scalar_vbo() const { return scalar_vbo_; }
	void set_scalar_vbo(cgogn::rendering::VBO* v)
	{
		scalar_vbo_ = v;
		if (scalar_vbo_)
		{
			const typename MapHandler<CMap2>::VertexAttribute<SCALAR>& attr = map_->template get_attribute<SCALAR, MapHandler<CMap2>::Vertex::ORBIT>(QString::fromStdString(scalar_vbo_->get_name()));
			float32 scalar_min = std::numeric_limits<float>::max();
			float32 scalar_max = std::numeric_limits<float>::min();
			for (const SCALAR& s : attr)
			{
				scalar_min = s < scalar_min ? s : scalar_min;
				scalar_max = s > scalar_max ? s : scalar_max;
			}
			shader_scalar_per_vertex_param_->set_scalar_vbo(scalar_vbo_);
			shader_scalar_per_vertex_param_->min_value_ = scalar_min;
			shader_scalar_per_vertex_param_->max_value_ = scalar_max;
		}
	}

	cgogn::rendering::ShaderScalarPerVertex::ColorMap get_color_map() const { return color_map_; }
	void set_color_map(int color_map)
	{
		color_map_ = cgogn::rendering::ShaderScalarPerVertex::ColorMap(color_map);
		shader_scalar_per_vertex_param_->color_map_ = color_map_;
	}

	int32 get_expansion() const { return expansion_; }
	void set_expansion(int32 expansion)
	{
		expansion_ = expansion;
		shader_scalar_per_vertex_param_->expansion_ = expansion_;
	}

	int32 get_show_iso_lines() const { return show_iso_lines_; }
	void set_show_iso_lines(bool show_iso_lines)
	{
		show_iso_lines_ = show_iso_lines;
		shader_scalar_per_vertex_param_->show_iso_lines_ = show_iso_lines_;
	}

private:

	MapHandler<CMap2>* map_;

	std::unique_ptr<cgogn::rendering::ShaderScalarPerVertex::Param> shader_scalar_per_vertex_param_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* scalar_vbo_;

	cgogn::rendering::ShaderScalarPerVertex::ColorMap color_map_;
	int32 expansion_;
	bool show_iso_lines_;
};

/**
* @brief Plugin that renders color-coded scalar values on surface vertices
*/
class Plugin_SurfaceRenderScalar : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class SurfaceRenderScalar_DockTab;

public:

	inline Plugin_SurfaceRenderScalar() {}

	~Plugin_SurfaceRenderScalar() {}

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

	SurfaceRenderScalar_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_
