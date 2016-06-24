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

#ifndef SCHNAPPS_PLUGIN_SELECTION_H_
#define SCHNAPPS_PLUGIN_SELECTION_H_

#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>

#include <cgogn/rendering/shaders/shader_point_sprite.h>

#include <selection_dock_tab.h>

namespace schnapps
{

class Plugin_Selection;

struct MapParameters
{
	friend class Plugin_Selection;

	enum SelectionMethod
	{
		SingleCell = 0,
		WithinSphere,
		NormalAngle
	};

	MapParameters() :
		map_(nullptr),
		shader_point_sprite_param_(nullptr),
		selection_sphere_vbo_(nullptr),
		color_(220, 60, 60),
		vertex_scale_factor_(1.0f),
		vertex_base_size_(1.0f),
		selecting_(false),
		cells_set_(nullptr),
		selection_method_(SingleCell)
	{
		selection_sphere_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_point_sprite_param_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_->color_ = QColor(60, 60, 220, 128);
		shader_point_sprite_param_->set_position_vbo(selection_sphere_vbo_.get());
	}

	const typename MapHandler<CMap2>::VertexAttribute<VEC3>& get_position_attribute() const { return position_; }
	QString get_position_attribute_name() const { return QString::fromStdString(position_.name()); }
	void set_position_attribute(const QString& attribute_name)
	{
		position_ = map_->get_attribute<VEC3, MapHandler<CMap2>::Vertex::ORBIT>(attribute_name);
	}

	const typename MapHandler<CMap2>::VertexAttribute<VEC3>& get_normal_attribute() const { return normal_; }
	QString get_normal_attribute_name() const { return QString::fromStdString(normal_.name()); }
	void set_normal_attribute(const QString& attribute_name)
	{
		normal_ = map_->get_attribute<VEC3, MapHandler<CMap2>::Vertex::ORBIT>(attribute_name);
	}

	const QColor& get_color() const { return color_; }
	void set_color(const QColor& c)
	{
		color_ = c;
		shader_point_sprite_param_->color_ = color_;
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

	void update_selected_cells_rendering()
	{

	}

private:

	MapHandler<CMap2>* map_;
	typename MapHandler<CMap2>::VertexAttribute<VEC3> position_;
	typename MapHandler<CMap2>::VertexAttribute<VEC3> normal_;

	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_;
	std::unique_ptr<cgogn::rendering::VBO> selection_sphere_vbo_;

	QColor color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;

	bool selecting_;
	MapHandler<CMap2>::Vertex selecting_vertex_;

public:

	CellsSet<CMap2, MapHandler<CMap2>::Vertex>* cells_set_;
	SelectionMethod selection_method_;
};

/**
* @brief Plugin for cells selection
*/
class Plugin_Selection : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class Selection_DockTab;

public:

	Plugin_Selection();

	~Plugin_Selection() {}

private:

	MapParameters& get_parameters(View* view, MapHandlerGen* map);

	bool enable() override;
	void disable() override;

	inline void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override {}
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;

	void keyPress(View*, QKeyEvent*) override;
	void keyRelease(View*, QKeyEvent*) override;
	void mousePress(View*, QMouseEvent*) override;
	inline void mouseRelease(View*, QMouseEvent*) override {}
	void mouseMove(View*, QMouseEvent*) override;
	void wheelEvent(View*, QWheelEvent*) override;

	inline void view_linked(View*) override {}
	inline void view_unlinked(View*) override {}

private slots:

	// slots called from SCHNApps signals
	void selected_view_changed(View*, View*);
	void selected_map_changed(MapHandlerGen*, MapHandlerGen*);

	// slots called from MapHandlerGen signals
	void selected_map_attribute_changed(cgogn::Orbit orbit, const QString& name);
	void selected_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void selected_map_connectivity_changed();
	void selected_map_bb_changed();

private:

	std::unique_ptr<Selection_DockTab> dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SELECTION_H_
