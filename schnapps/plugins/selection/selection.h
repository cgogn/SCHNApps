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
#include <schnapps/core/view.h>

#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/shaders/shader_bold_line.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>

#include <selection_dock_tab.h>

namespace schnapps
{

class Plugin_Selection;

struct MapParameters : public QObject
{
	Q_OBJECT

	friend class Plugin_Selection;

public:

	enum SelectionMethod
	{
		SingleCell = 0,
		WithinSphere,
		NormalAngle
	};

	MapParameters() :
		map_(nullptr),
		shader_point_sprite_param_selection_sphere_(nullptr),
		selection_sphere_vbo_(nullptr),
		shader_point_sprite_param_selected_vertices_(nullptr),
		selected_vertices_vbo_(nullptr),
		shader_bold_line_param_selection_edge_(nullptr),
		selection_edge_vbo_(nullptr),
		shader_bold_line_param_selected_edges_(nullptr),
		selected_edges_vbo_(nullptr),
		shader_simple_color_param_selection_face_(nullptr),
		selection_face_vbo_(nullptr),
		shader_simple_color_param_selected_faces_(nullptr),
		selected_faces_vbo_(nullptr),
		color_(220, 60, 60),
		vertex_scale_factor_(1.0f),
		vertex_base_size_(1.0f),
		selecting_(false),
		cells_set_(nullptr),
		selection_method_(SingleCell)
	{
		selection_sphere_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_point_sprite_param_selection_sphere_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_selection_sphere_->color_ = QColor(60, 60, 220, 128);
		shader_point_sprite_param_selection_sphere_->set_position_vbo(selection_sphere_vbo_.get());

		selected_vertices_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_point_sprite_param_selected_vertices_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_selected_vertices_->color_ = color_;
		shader_point_sprite_param_selected_vertices_->set_position_vbo(selected_vertices_vbo_.get());

		selection_edge_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_bold_line_param_selection_edge_ = cgogn::rendering::ShaderBoldLine::generate_param();
		shader_bold_line_param_selection_edge_->color_ = QColor(60, 60, 220, 128);
		shader_bold_line_param_selection_edge_->width_ = 2.0f;
		shader_bold_line_param_selection_edge_->set_position_vbo(selection_edge_vbo_.get());

		selected_edges_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_bold_line_param_selected_edges_ = cgogn::rendering::ShaderBoldLine::generate_param();
		shader_bold_line_param_selected_edges_->color_ = color_;
		shader_bold_line_param_selected_edges_->width_ = 2.0f;
		shader_bold_line_param_selected_edges_->set_position_vbo(selected_edges_vbo_.get());

		selection_face_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_simple_color_param_selection_face_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_selection_face_->color_ = QColor(60, 60, 220, 128);
		shader_simple_color_param_selection_face_->set_position_vbo(selection_face_vbo_.get());

		selected_faces_vbo_ = cgogn::make_unique<cgogn::rendering::VBO>(3);

		shader_simple_color_param_selected_faces_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_selected_faces_->color_ = color_;
		shader_simple_color_param_selected_faces_->set_position_vbo(selected_faces_vbo_.get());
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
		shader_point_sprite_param_selected_vertices_->color_ = color_;
	}

	float32 get_vertex_base_size() const { return vertex_base_size_; }
	void set_vertex_base_size(float32 bs)
	{
		vertex_base_size_ = bs;
		shader_point_sprite_param_selection_sphere_->size_ = vertex_base_size_ * vertex_scale_factor_;
		shader_point_sprite_param_selected_vertices_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	float32 get_vertex_scale_factor() const { return vertex_scale_factor_; }
	void set_vertex_scale_factor(float32 sf)
	{
		vertex_scale_factor_ = sf;
		shader_point_sprite_param_selection_sphere_->size_ = vertex_base_size_ * vertex_scale_factor_;
		shader_point_sprite_param_selected_vertices_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	CellsSetGen* get_cells_set() const { return cells_set_; }
	void set_cells_set(CellsSetGen* cs)
	{
		if (cells_set_)
			disconnect(cells_set_, SIGNAL(selected_cells_changed()), this, SLOT(update_selected_cells_rendering()));
		cells_set_ = cs;
		if (cells_set_)
			connect(cells_set_, SIGNAL(selected_cells_changed()), this, SLOT(update_selected_cells_rendering()));
		update_selected_cells_rendering();
	}

public slots:

	void update_selected_cells_rendering()
	{
		if (cells_set_)
		{
			switch (cells_set_->get_cell_type())
			{
				case Dart_Cell:
					break;
				case Vertex_Cell:
				{
					std::vector<VEC3> selected_points;
					if (position_.is_valid())
					{
						CellsSet<CMap2, MapHandler<CMap2>::Vertex>* tcs = static_cast<CellsSet<CMap2, MapHandler<CMap2>::Vertex>*>(cells_set_);
						tcs->foreach_cell([&] (MapHandler<CMap2>::Vertex v)
						{
							selected_points.push_back(position_[v]);
						});
					}
					cgogn::rendering::update_vbo(selected_points, selected_vertices_vbo_.get());
				}
					break;
				case Edge_Cell:
				{
					std::vector<VEC3> selected_segments;
					if (position_.is_valid())
					{
						CellsSet<CMap2, MapHandler<CMap2>::Edge>* tcs = static_cast<CellsSet<CMap2, MapHandler<CMap2>::Edge>*>(cells_set_);
						tcs->foreach_cell([&] (MapHandler<CMap2>::Edge e)
						{
							std::pair<MapHandler<CMap2>::Vertex, MapHandler<CMap2>::Vertex> vertices = map_->get_map()->vertices(e);
							selected_segments.push_back(position_[vertices.first]);
							selected_segments.push_back(position_[vertices.second]);
						});
					}
					cgogn::rendering::update_vbo(selected_segments, selected_edges_vbo_.get());
				}
					break;
				case Face_Cell:
				{
					std::vector<VEC3> selected_polygons;
					if (position_.is_valid())
					{
						CellsSet<CMap2, MapHandler<CMap2>::Face>* tcs = static_cast<CellsSet<CMap2, MapHandler<CMap2>::Face>*>(cells_set_);
						std::vector<uint32> ears;
						tcs->foreach_cell([&] (MapHandler<CMap2>::Face f)
						{
							cgogn::geometry::append_ear_triangulation<VEC3>(*map_->get_map(), f, position_, ears);
						});
						for (uint32 i : ears)
							selected_polygons.push_back(position_[i]);
					}
					cgogn::rendering::update_vbo(selected_polygons, selected_faces_vbo_.get());
				}
					break;
				case Volume_Cell:
					break;
			}

			for (View* view : map_->get_linked_views())
				view->update();
		}
	}

private:

	MapHandler<CMap2>* map_;
	typename MapHandler<CMap2>::VertexAttribute<VEC3> position_;
	typename MapHandler<CMap2>::VertexAttribute<VEC3> normal_;

	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_selection_sphere_;
	std::unique_ptr<cgogn::rendering::VBO> selection_sphere_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_selected_vertices_;
	std::unique_ptr<cgogn::rendering::VBO> selected_vertices_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderBoldLine::Param> shader_bold_line_param_selection_edge_;
	std::unique_ptr<cgogn::rendering::VBO> selection_edge_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderBoldLine::Param> shader_bold_line_param_selected_edges_;
	std::unique_ptr<cgogn::rendering::VBO> selected_edges_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param> shader_simple_color_param_selection_face_;
	std::unique_ptr<cgogn::rendering::VBO> selection_face_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param> shader_simple_color_param_selected_faces_;
	std::unique_ptr<cgogn::rendering::VBO> selected_faces_vbo_;

	QColor color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;

	bool selecting_;
	MapHandler<CMap2>::Vertex selecting_vertex_;
	MapHandler<CMap2>::Edge selecting_edge_;
	MapHandler<CMap2>::Face selecting_face_;

	CellsSetGen* cells_set_;

public:

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
