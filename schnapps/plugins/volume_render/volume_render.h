/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2015, IGG Group, ICube, University of Strasbourg, France       *
* Plugin Volume Render                                                         *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
* Inspired by the surface render plugin                                        *
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

#ifndef SCHNAPPS_PLUGIN_VOLUME_RENDER_H_
#define SCHNAPPS_PLUGIN_VOLUME_RENDER_H_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>

#include <volume_render_dock_tab.h>

#include <QAction>
#include <map>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/volume_drawer.h>
#include <cgogn/rendering/frame_manipulator.h>
#include <cgogn/rendering/topo_drawer.h>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
#include <cgogn/rendering/transparency_volume_drawer.h>
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))


namespace schnapps
{

class MapHandlerGen;
class Plugin_VolumeRender;

namespace plugin_volume_render
{

struct SCHNAPPS_PLUGIN_VOLUME_RENDER_API MapParameters
{
	friend class Plugin_VolumeRender;

	MapParameters();

	cgogn::rendering::VBO* get_position_vbo() const { return position_vbo_; }
	void set_position_vbo(cgogn::rendering::VBO* v);

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
		volume_drawer_rend_->set_face_color(c);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
		volume_transparency_drawer_rend_->set_color(c);
#endif
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
		volume_drawer_rend_->set_explode_volume(vef);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
		volume_transparency_drawer_rend_->set_explode_volume(vef);
#endif
		topo_drawer_->set_explode_volume(vef);
		auto pos_attr = map_->get_attribute<VEC3, CMap3::Vertex::ORBIT>(QString::fromStdString(position_vbo_->name()));
		if (pos_attr.is_valid())
			topo_drawer_->update<VEC3>(*map_->get_map(),pos_attr);
	}

	int32 get_transparency_factor() const { return transparency_factor_; }
	void set_transparency_factor(int32 n)
	{
		transparency_factor_ = n;
		face_color_.setAlpha(n);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
		volume_transparency_drawer_rend_->set_color(face_color_);
#endif
	}

	bool get_apply_clipping_plane() const { return apply_clipping_plane_; }
	void set_apply_clipping_plane(bool b)
	{
		apply_clipping_plane_ = b;
		if (b)
		{
			VEC3 position;
			VEC3 axis_z;
			frame_manip_->get_position(position);
			frame_manip_->get_axis(cgogn::rendering::FrameManipulator::Zt, axis_z);
			float32 d = -(position.dot(axis_z));
			volume_drawer_rend_->set_clipping_plane(QVector4D(axis_z[0], axis_z[1], axis_z[2], d));
			topo_drawer_rend_->set_clipping_plane(QVector4D(axis_z[0], axis_z[1], axis_z[2], d));
		} else {
			volume_drawer_rend_->set_clipping_plane(QVector4D(0, 0, 0, 0));
			topo_drawer_rend_->set_clipping_plane(QVector4D(0, 0, 0, 0));
		}
	}

	void plane_clip_from_frame()
	{
		VEC3D position;
		VEC3D axis_z;
		frame_manip_->get_position(position);
		frame_manip_->get_axis(cgogn::rendering::FrameManipulator::Zt,axis_z);
		const double d = -(position.dot(axis_z));
		plane_clipping_ = QVector4D(axis_z[0],axis_z[1],axis_z[2],d);
	}

private:

	MapHandler<CMap3>* map_;

	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param>	shader_simple_color_param_;
	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_;

	cgogn::rendering::VBO* position_vbo_;

	QColor vertex_color_;
	QColor edge_color_;
	QColor face_color_;

	float32 vertex_scale_factor_;
	float32 vertex_base_size_;

	float32 volume_explode_factor_;
	int32 transparency_factor_;
	QVector4D plane_clipping_;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	std::unique_ptr<cgogn::rendering::VolumeTransparencyDrawer> volume_transparency_drawer_;
	std::unique_ptr<cgogn::rendering::VolumeTransparencyDrawer::Renderer> volume_transparency_drawer_rend_;
#endif // (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
	std::unique_ptr<cgogn::rendering::VolumeDrawer> volume_drawer_;
	std::unique_ptr<cgogn::rendering::VolumeDrawer::Renderer> volume_drawer_rend_;

	std::unique_ptr<cgogn::rendering::TopoDrawer> topo_drawer_;
	std::unique_ptr<cgogn::rendering::TopoDrawer::Renderer> topo_drawer_rend_;

	std::unique_ptr<cgogn::rendering::FrameManipulator> frame_manip_;
	bool apply_clipping_plane_;

public:

	bool render_vertices_;
	bool render_edges_;
	bool render_faces_;
	bool render_boundary_;
	bool render_topology_;
	bool use_transparency_;
};

/**
* @brief Plugin for surface rendering
*/
class SCHNAPPS_PLUGIN_VOLUME_RENDER_API Plugin_VolumeRender : public PluginInteraction
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
	void mousePress(View*, QMouseEvent*) override;
	void mouseRelease(View*, QMouseEvent*) override;
	void mouseMove(View*, QMouseEvent*) override;
	inline void wheelEvent(View*, QWheelEvent*) override {}
	void resizeGL(View* view, int width, int height) override;

	void view_linked(View*) override;
	void view_unlinked(View*) override;

	void connectivity_changed(MapHandlerGen* mh);

private slots:

	// slots called from View signals
	void map_linked(MapHandlerGen* map);
	void map_unlinked(MapHandlerGen* map);

	// slots called from MapHandler signals
	void linked_map_vbo_added(cgogn::rendering::VBO* vbo);
	void linked_map_vbo_removed(cgogn::rendering::VBO* vbo);
	void linked_map_bb_changed();
	void linked_map_connectivity_changed();
	void linked_attribute_changed(cgogn::Orbit,QString);

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

	void set_position_vbo(View* view, MapHandlerGen* map, cgogn::rendering::VBO* vbo);
	inline void set_position_vbo(const QString& view_name, const QString& map_name, const QString& vbo_name)
	{
		MapHandlerGen* map = schnapps_->get_map(map_name);
		if (map)
			set_position_vbo(schnapps_->get_view(view_name), map, map->get_vbo(vbo_name));
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

	void set_face_color(View* view, MapHandlerGen* map, const QColor& color);
	inline void set_face_color(const QString& view_name, const QString& map_name, const QColor& color)
	{
		set_face_color(schnapps_->get_view(view_name), schnapps_->get_map(map_name), color);
	}

	void set_vertex_scale_factor(View* view, MapHandlerGen* map, float32 sf);
	void set_vertex_scale_factor(const QString& view_name, const QString& map_name, float32 sf)
	{
		set_vertex_scale_factor(schnapps_->get_view(view_name), schnapps_->get_map(map_name), sf);
	}

	void set_volume_explode_factor(View* view, MapHandlerGen* map, float32 vef);
	void set_volume_explode_factor(const QString& view_name, const QString& map_name, float32 vef)
	{
		set_volume_explode_factor(schnapps_->get_view(view_name), schnapps_->get_map(map_name), vef);
	}

	void set_apply_clipping_plane(View* view, MapHandlerGen* map, bool b);
	void set_apply_clipping_plane(const QString& view_name, const QString& map_name, bool b)
	{
		set_apply_clipping_plane(schnapps_->get_view(view_name), schnapps_->get_map(map_name), b);
	}

private:

	VolumeRender_DockTab* dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace plugin_volume_render
} // namespace schnapps


#endif // SCHNAPPS_PLUGIN_VOLUME_RENDER_H_
