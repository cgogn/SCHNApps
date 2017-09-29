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

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <surface_render_scalar_dock_tab.h>

#include <cgogn/rendering/shaders/shader_scalar_per_vertex.h>

namespace schnapps
{

namespace plugin_surface_render_scalar
{

class Plugin_SurfaceRenderScalar;

struct SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_API MapParameters
{
	friend class Plugin_SurfaceRenderScalar;

	MapParameters() :
		position_vbo_(nullptr),
		scalar_vbo_(nullptr),
		color_map_(cgogn::rendering::ShaderScalarPerVertex::BWR),
		scalar_min_(0.0),
		scalar_max_(1.0),
		auto_update_min_max_(true),
		expansion_(0),
		nb_iso_levels_(10),
		show_iso_lines_(false)
	{
		initialize_gl();
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	cgogn::rendering::VBO* get_scalar_vbo() const { return scalar_vbo_; }
	cgogn::rendering::ShaderScalarPerVertex::ColorMap get_color_map() const { return color_map_; }
	SCALAR get_scalar_min() const { return scalar_min_; }
	SCALAR get_scalar_max() const { return scalar_max_; }
	bool get_auto_update_min_max() const { return auto_update_min_max_; }
	int32 get_expansion() const { return expansion_; }
	bool get_show_iso_lines() const { return show_iso_lines_; }
	int32 get_nb_iso_levels() const { return nb_iso_levels_; }

private:

	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_ && position_vbo_->vector_dimension() == 3)
			shader_scalar_per_vertex_param_->set_position_vbo(position_vbo_);
		else
			position_vbo_ = nullptr;
	}

	void set_scalar_vbo(cgogn::rendering::VBO* v)
	{
		scalar_vbo_ = v;
		if (scalar_vbo_ && scalar_vbo_->vector_dimension() == 1)
		{
			if (auto_update_min_max_)
				update_min_max();
			shader_scalar_per_vertex_param_->set_scalar_vbo(scalar_vbo_);
		}
		else
			scalar_vbo_ = nullptr;
	}

	void set_color_map(cgogn::rendering::ShaderScalarPerVertex::ColorMap color_map)
	{
		color_map_ = color_map;
		shader_scalar_per_vertex_param_->color_map_ = color_map_;
	}

	void set_scalar_min(SCALAR s)
	{
		scalar_min_ = s;
		shader_scalar_per_vertex_param_->min_value_ = scalar_min_;
	}

	void set_scalar_max(SCALAR s)
	{
		scalar_max_ = s;
		shader_scalar_per_vertex_param_->max_value_ = scalar_max_;
	}

	void set_auto_update_min_max(bool update)
	{
		auto_update_min_max_ = update;
		if (auto_update_min_max_)
			update_min_max();
	}

	void set_expansion(int32 expansion)
	{
		expansion_ = expansion;
		shader_scalar_per_vertex_param_->expansion_ = expansion_;
	}

	void set_show_iso_lines(bool show_iso_lines)
	{
		show_iso_lines_ = show_iso_lines;
		shader_scalar_per_vertex_param_->show_iso_lines_ = show_iso_lines_;
	}

	void set_nb_iso_levels(int32 n)
	{
		nb_iso_levels_ = n;
		shader_scalar_per_vertex_param_->nb_iso_levels_ = nb_iso_levels_;
	}

	void initialize_gl()
	{
		shader_scalar_per_vertex_param_ = cgogn::rendering::ShaderScalarPerVertex::generate_param();
		shader_scalar_per_vertex_param_->color_map_ = color_map_;
		shader_scalar_per_vertex_param_->expansion_ = expansion_;
		shader_scalar_per_vertex_param_->min_value_ = 0.0f;
		shader_scalar_per_vertex_param_->max_value_ = 1.0f;
		shader_scalar_per_vertex_param_->show_iso_lines_ = show_iso_lines_;
		shader_scalar_per_vertex_param_->nb_iso_levels_ = nb_iso_levels_;

		set_position_vbo(position_vbo_);
		set_scalar_vbo(scalar_vbo_);
	}

	void update_min_max()
	{
		const CMap2::VertexAttribute<SCALAR>& attr = map_->template get_attribute<SCALAR, CMap2::Vertex::ORBIT>(QString::fromStdString(scalar_vbo_->name()));
		if (!attr.is_valid())
		{
			cgogn_log_warning("plugin_surface_render_scalar|MapParameters::update_min_max") << "The attribute \"" << scalar_vbo_->name() << "\" is not valid. Its data should be of type " << cgogn::name_of_type(SCALAR()) << ".";
			scalar_vbo_ = nullptr;
			return;
		}
		scalar_min_ = std::numeric_limits<SCALAR>::max();
		scalar_max_ = std::numeric_limits<SCALAR>::lowest();
		for (const SCALAR& s : attr)
		{
			scalar_min_ = s < scalar_min_ ? s : scalar_min_;
			scalar_max_ = s > scalar_max_ ? s : scalar_max_;
		}
		shader_scalar_per_vertex_param_->min_value_ = scalar_min_;
		shader_scalar_per_vertex_param_->max_value_ = scalar_max_;
	}

	CMap2Handler* map_;

	std::unique_ptr<cgogn::rendering::ShaderScalarPerVertex::Param> shader_scalar_per_vertex_param_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* scalar_vbo_;

	cgogn::rendering::ShaderScalarPerVertex::ColorMap color_map_;
	SCALAR scalar_min_;
	SCALAR scalar_max_;
	bool auto_update_min_max_;
	int32 expansion_;
	int32 nb_iso_levels_;
	bool show_iso_lines_;
};

/**
* @brief Plugin that renders color-coded scalar values on surface vertices
*/
class SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_API Plugin_SurfaceRenderScalar : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	inline Plugin_SurfaceRenderScalar() {}

	~Plugin_SurfaceRenderScalar() override {}

	MapParameters& get_parameters(View* view, MapHandlerGen* map);

private:

	bool enable() override;
	void disable() override;

	void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override {}
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	void keyPress(View*, QKeyEvent*) override {}
	void keyRelease(View*, QKeyEvent*) override {}
	void mousePress(View*, QMouseEvent*) override {}
	void mouseRelease(View*, QMouseEvent*) override {}
	void mouseMove(View*, QMouseEvent*) override {}
	void wheelEvent(View*, QWheelEvent*) override {}
	void resizeGL(View* /*view*/, int /*width*/, int /*height*/) override {}

	void view_linked(View*) override;
	void view_unlinked(View*) override;

private slots:

	// slots called from View signals
	void map_linked(MapHandlerGen* map);
	void map_unlinked(MapHandlerGen* map);

private:

	void add_linked_map(View* view, MapHandlerGen* map);
	void remove_linked_map(View* view, MapHandlerGen* map);

private slots:

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();
	void linked_map_attribute_changed(cgogn::Orbit orbit, const QString& attribute_name);

	void viewer_initialized();

public slots:

	void set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_scalar_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo, bool update_dock_tab);
	void set_color_map(View* view, MapHandlerGen* map, cgogn::rendering::ShaderScalarPerVertex::ColorMap cm, bool update_dock_tab);
	void set_auto_update_min_max(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_scalar_min(View* view, MapHandlerGen* map, double d, bool update_dock_tab);
	void set_scalar_max(View* view, MapHandlerGen* map, double d, bool update_dock_tab);
	void set_expansion(View* view, MapHandlerGen* map, int32 i, bool update_dock_tab);
	void set_show_iso_lines(View* view, MapHandlerGen* map, bool b, bool update_dock_tab);
	void set_nb_iso_levels(View* view, MapHandlerGen* map, int32 i, bool update_dock_tab);

private:

	SurfaceRenderScalar_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;

	QString setting_auto_load_position_attribute_;
};

} // namespace plugin_surface_render_scalar

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_SCALAR_H_
