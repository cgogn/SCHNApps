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
		position_vbo_(nullptr),
		normal_vbo_(nullptr),
		color_vbo_(nullptr),
		verticesScaleFactor(1.0f),
		renderVertices(false),
		renderEdges(false),
		renderFaces(true),
		faceStyle(FLAT)
	{
		shader_flat_param_ = cgogn::rendering::ShaderFlat::generate_param();
	}

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v)
	{
		position_vbo_ = v;
		if (v)
			shader_flat_param_->set_position_vbo(position_vbo_);
	}

	cgogn::rendering::VBO* get_normal_vbo() const { return normal_vbo_; }
	void set_normal_vbo(cgogn::rendering::VBO* v)
	{
		normal_vbo_ = v;
	}

	cgogn::rendering::VBO* get_color_vbo() const { return color_vbo_; }
	void set_color_vbo(cgogn::rendering::VBO* v)
	{
		color_vbo_ = v;
	}

	const QColor& get_diffuse_color() const { return diffuse_color_; }
	void set_diffuse_color(const QColor& c)
	{
		diffuse_color_ = c;
		shader_flat_param_->ambiant_color_ = diffuse_color_;
	}

	const QColor& get_simple_color() const { return simple_color_; }
	void set_simple_color(const QColor& c)
	{
		simple_color_ = c;
	}

	const QColor& get_vertex_color() const { return vertex_color_; }
	void set_vertex_color(const QColor& c)
	{
		vertex_color_ = c;
	}

	const QColor& get_back_color() const { return back_color_; }
	void set_back_color(const QColor& c)
	{
		back_color_ = c;
	}

private:

	cgogn::rendering::ShaderFlat::Param* shader_flat_param_;

	cgogn::rendering::VBO* position_vbo_;
	cgogn::rendering::VBO* normal_vbo_;
	cgogn::rendering::VBO* color_vbo_;

	QColor diffuse_color_;
	QColor simple_color_;
	QColor vertex_color_;
	QColor back_color_;

public:

	float verticesScaleFactor;
	float basePSradius;
	bool renderVertices;
	bool renderEdges;
	bool renderFaces;
	bool renderBoundary;
	bool renderBackfaces;
	FaceShadingStyle faceStyle;
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

	bool enable() override;
	void disable() override;

	inline void draw(View*) override {}
	void draw_map(View* view, MapHandlerGen* map) override;

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
	QHash<View*, QHash<MapHandlerGen*, MapParameters> > parameter_set_;

	cgogn::rendering::ShaderFlat* shader_flat_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
