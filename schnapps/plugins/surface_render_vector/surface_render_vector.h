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

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>

#include <surface_render_vector_dock_tab.h>

#include <cgogn/rendering/shaders/shader_vector_per_vertex.h>

#include <QAction>

namespace schnapps
{

class MapHandlerGen;
class Plugin_SurfaceRenderVector;

struct MapParameters
{
	friend class Plugin_SurfaceRenderVector;

	MapParameters() :
		position_vbo_(nullptr)
	{}

	const QList<cgogn::rendering::ShaderVectorPerVertex::Param*>& get_shader_params() const { return shader_vector_per_vertex_param_list_; }

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (position_vbo_)
		{
			for (cgogn::rendering::ShaderVectorPerVertex::Param* p : shader_vector_per_vertex_param_list_)
				p->set_position_vbo(position_vbo_);
		}
	}

	int get_vector_vbo_index(cgogn::rendering::VBO* v) const { return vector_vbo_list_.indexOf(v); }
	void add_vector_vbo(cgogn::rendering::VBO* v)
	{
		if (!vector_vbo_list_.contains(v))
		{
			vector_vbo_list_.append(v);
			vector_scale_factor_list_.append(1.0f);
			vector_color_list_.append(QColor("green"));
			cgogn::rendering::ShaderVectorPerVertex::Param* p = cgogn::rendering::ShaderVectorPerVertex::generate_param();
			if (position_vbo_)
				p->set_position_vbo(position_vbo_);
			p->set_vector_vbo(vector_vbo_list_.last());
			p->length_ = vector_scale_factor_list_.last();
			p->color_ = vector_color_list_.last();
			shader_vector_per_vertex_param_list_.append(p);
		}
	}
	void remove_vector_vbo(cgogn::rendering::VBO* v)
	{
		int idx = vector_vbo_list_.indexOf(v);
		if (idx >= 0)
		{
			vector_vbo_list_.removeAt(idx);
			vector_scale_factor_list_.removeAt(idx);
			vector_color_list_.removeAt(idx);
			delete shader_vector_per_vertex_param_list_[idx];
			shader_vector_per_vertex_param_list_.removeAt(idx);
		}
	}

	float32 get_vector_scale_factor(int i) const { return vector_scale_factor_list_[i]; }
	void set_vector_scale_factor(int i, float32 sf)
	{
		vector_scale_factor_list_[i] = sf;
		shader_vector_per_vertex_param_list_[i]->length_ = vector_scale_factor_list_[i];
	}

	const QColor& get_vector_color(int i) const { return vector_color_list_[i]; }
	void set_vector_color(int i, const QColor& c)
	{
		vector_color_list_[i] = c;
		shader_vector_per_vertex_param_list_[i]->color_ = vector_color_list_[i];
	}

private:

	QList<cgogn::rendering::ShaderVectorPerVertex::Param*> shader_vector_per_vertex_param_list_;

	cgogn::rendering::VBO* position_vbo_;
	QList<cgogn::rendering::VBO*> vector_vbo_list_;
	QList<float32> vector_scale_factor_list_;
	QList<QColor> vector_color_list_;
};

/**
* @brief Plugin for surface rendering
*/
class Plugin_SurfaceRenderVector : public PluginInteraction
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

	SurfaceRenderVector_DockTab* dock_tab_;
	QHash<View*, QHash<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_VECTOR_H_
