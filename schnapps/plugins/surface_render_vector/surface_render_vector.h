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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <surface_render_vector_dock_tab.h>

#include <cgogn/rendering/shaders/shader_vector_per_vertex.h>

#include <QAction>

namespace schnapps
{
namespace plugin_surface_render_vector
{

class Plugin_SurfaceRenderVector;

struct SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_API MapParameters
{
	friend class Plugin_SurfaceRenderVector;

	inline MapParameters() :
		position_vbo_(nullptr)
	{}

	MapParameters(const MapParameters&) = delete;
	MapParameters& operator=(const MapParameters&) = delete;

	inline MapParameters(MapParameters&& param) : 
		map_(param.map_),
		shader_vector_per_vertex_param_list_(std::move(param.shader_vector_per_vertex_param_list_)),
		position_vbo_(param.position_vbo_),
		vector_vbo_list_(std::move(param.vector_vbo_list_)),
		vector_scale_factor_list_(std::move(param.vector_scale_factor_list_)),
		vector_color_list_(std::move(param.vector_color_list_))
	{
		param.map_ = nullptr;
		param.position_vbo_ = nullptr;
	}

	MapParameters& operator=(MapParameters&& param)
	{
		if (this != &param)
		{
			map_ = param.map_;
			shader_vector_per_vertex_param_list_ = std::move(param.shader_vector_per_vertex_param_list_);
			position_vbo_ = param.position_vbo_;
			vector_vbo_list_ = std::move(param.vector_vbo_list_);
			vector_scale_factor_list_ = std::move(param.vector_scale_factor_list_);
			vector_color_list_ = std::move(param.vector_color_list_);
			param.map_ = nullptr;
			param.position_vbo_ = nullptr;
		}
		return *this;
	}


	const std::vector<std::unique_ptr<cgogn::rendering::ShaderVectorPerVertex::Param>>& get_shader_params() const
	{
		return shader_vector_per_vertex_param_list_;
	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_)
		{
			for (auto& p : shader_vector_per_vertex_param_list_)
				p->set_position_vbo(position_vbo_);
		}
	}

	uint32 get_vector_vbo_index(cgogn::rendering::VBO* v) const
	{
		const uint32 index = std::find(vector_vbo_list_.begin(), vector_vbo_list_.end(), v) - vector_vbo_list_.begin();
		return index >= vector_vbo_list_.size() ? UINT32_MAX : index;
	}
	void add_vector_vbo(cgogn::rendering::VBO* v)
	{
		if (std::find(vector_vbo_list_.begin(), vector_vbo_list_.end(), v) == vector_vbo_list_.end())
		{
			vector_vbo_list_.push_back(v);
			vector_scale_factor_list_.push_back(1.0f);
			vector_color_list_.push_back(QColor("green"));
			auto p = cgogn::rendering::ShaderVectorPerVertex::generate_param();
			if (position_vbo_)
				p->set_position_vbo(position_vbo_);
			p->set_vector_vbo(vector_vbo_list_.back());
			p->length_ = map_->get_bb_diagonal_size() / 100.0f * vector_scale_factor_list_.back();
			p->color_ = vector_color_list_.back();
			shader_vector_per_vertex_param_list_.push_back(std::move(p));
		}
	}
	void remove_vector_vbo(cgogn::rendering::VBO* v)
	{
		const uint32 idx = get_vector_vbo_index(v);
		if (idx != UINT32_MAX)
		{
			vector_vbo_list_[idx] = vector_vbo_list_.back();
			vector_vbo_list_.pop_back();
			vector_scale_factor_list_[idx] = vector_scale_factor_list_.back();
			vector_scale_factor_list_.pop_back();
			vector_color_list_[idx] = vector_color_list_.back();
			vector_color_list_.pop_back();
			shader_vector_per_vertex_param_list_[idx].swap(shader_vector_per_vertex_param_list_.back());
			shader_vector_per_vertex_param_list_.pop_back();
		}
	}

	float32 get_vector_scale_factor(uint32 i) const { return vector_scale_factor_list_[i]; }
	void set_vector_scale_factor(uint32 i, float32 sf)
	{
		vector_scale_factor_list_[i] = sf;
		shader_vector_per_vertex_param_list_[i]->length_ = map_->get_bb_diagonal_size() / 100.0f * vector_scale_factor_list_[i];
	}

	const QColor& get_vector_color(uint32 i) const { return vector_color_list_[i]; }
	void set_vector_color(uint32 i, const QColor& c)
	{
		vector_color_list_[i] = c;
		shader_vector_per_vertex_param_list_[i]->color_ = vector_color_list_[i];
	}

private:
	void initialize_gl()
	{
		shader_vector_per_vertex_param_list_.clear();
		std::vector<cgogn::rendering::VBO*> vbos;
		std::vector<float32> scale_factors;
		std::vector<QColor> colors_list;

		vbos.swap(vector_vbo_list_);
		scale_factors.swap(vector_scale_factor_list_);
		colors_list.swap(vector_color_list_);

		for (uint32 i = 0u ; i < vbos.size(); ++i)
		{
			add_vector_vbo(vbos[i]);
			set_vector_scale_factor(i, scale_factors[i]);
			set_vector_color(i, colors_list[i]);
		}
	}

private:

	CMap2Handler* map_;

	std::vector<std::unique_ptr<cgogn::rendering::ShaderVectorPerVertex::Param>> shader_vector_per_vertex_param_list_;

	cgogn::rendering::VBO* position_vbo_;
	std::vector<cgogn::rendering::VBO*> vector_vbo_list_;
	std::vector<float32> vector_scale_factor_list_;
	std::vector<QColor> vector_color_list_;
};

/**
* @brief Plugin that renders vectors on surface vertices
*/
class SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_API Plugin_SurfaceRenderVector : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class SurfaceRenderVector_DockTab;

public:

	inline Plugin_SurfaceRenderVector() {}

	~Plugin_SurfaceRenderVector() {}

private:

	MapParameters& get_parameters(View* view, MapHandlerGen* map);

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

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();
	void viewer_initialized();

	void update_dock_tab();

public slots:

	void set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void set_position_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_position_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
	}

	void add_vector_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void add_vector_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			add_vector_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
	}

	void remove_vector_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void remove_vector_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			remove_vector_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
	}

	void set_vector_scale_factor(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vector_vbo, float32 sf);
	void set_vector_scale_factor(const QString& view_name, const QString& map_name, const QString& vector_vbo_name, float32 sf)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_vector_scale_factor(schnapps_->get_view(view_name), map, map->get_vbo(vector_vbo_name), sf);
	}

	void set_vector_color(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vector_vbo, const QColor& color);
	void set_vector_color(const QString& view_name, const QString& map_name, const QString& vector_vbo_name, const QColor& color)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_vector_color(schnapps_->get_view(view_name), map, map->get_vbo(vector_vbo_name), color);
	}

private:

	SurfaceRenderVector_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace plugin_surface_render_vector
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_
