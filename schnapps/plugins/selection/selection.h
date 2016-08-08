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

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/types.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

#include <cgogn/geometry/algos/selection.h>
#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/shaders/shader_bold_line.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>


#include <selection_dock_tab.h>

namespace schnapps
{

namespace plugin_selection
{

class Plugin_Selection;

struct SCHNAPPS_PLUGIN_SELECTION_API MapParameters : public QObject
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
		position_(cgogn::make_unique<MapHandlerGen::Attribute_T<VEC3>>()),
		normal_(cgogn::make_unique<MapHandlerGen::Attribute_T<VEC3>>()),
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
		shader_flat_param_selected_faces_(nullptr),
		selected_faces_vbo_(nullptr),
		color_("red"),
		vertex_scale_factor_(1.0f),
		vertex_base_size_(1.0f),
		selection_radius_scale_factor_(1.0f),
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
		shader_bold_line_param_selection_edge_->width_ = 4.0f;
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

		shader_flat_param_selected_faces_ = cgogn::rendering::ShaderFlat::generate_param();
		shader_flat_param_selected_faces_->front_color_ = color_;
		shader_flat_param_selected_faces_->back_color_ = color_;
		shader_flat_param_selected_faces_->set_position_vbo(selected_faces_vbo_.get());
	}

	const MapHandlerGen::Attribute_T<VEC3>& get_position_attribute() const { return *position_; }
	QString get_position_attribute_name() const { return QString::fromStdString(position_->name()); }
	void set_position_attribute(const QString& attribute_name)
	{
		if (map_->dimension() == 2)
			position_ = cgogn::make_unique<CMap2Handler::VertexAttribute<VEC3>>(dynamic_cast<CMap2Handler*>(map_)->get_attribute<VEC3, MapHandler<CMap2>::Vertex::ORBIT>(attribute_name));
		else
			position_ = cgogn::make_unique<CMap3Handler::VertexAttribute<VEC3>>(dynamic_cast<CMap3Handler*>(map_)->get_attribute<VEC3, MapHandler<CMap3>::Vertex::ORBIT>(attribute_name));
	}

	const MapHandlerGen::Attribute_T<VEC3>& get_normal_attribute() const { return *normal_; }
	QString get_normal_attribute_name() const { return QString::fromStdString(normal_->name()); }
	void set_normal_attribute(const QString& attribute_name)
	{
		if (map_->dimension() == 2)
			normal_ = cgogn::make_unique<CMap2Handler::VertexAttribute<VEC3>>(dynamic_cast<CMap2Handler*>(map_)->get_attribute<VEC3, MapHandler<CMap2>::Vertex::ORBIT>(attribute_name));
		else
			normal_ = cgogn::make_unique<CMap3Handler::VertexAttribute<VEC3>>(dynamic_cast<CMap3Handler*>(map_)->get_attribute<VEC3, MapHandler<CMap3>::Vertex::ORBIT>(attribute_name));
	}

	const QColor& get_color() const { return color_; }
	void set_color(const QColor& c)
	{
		color_ = c;
		shader_point_sprite_param_selected_vertices_->color_ = color_;
		shader_bold_line_param_selected_edges_->color_ = color_;
		shader_flat_param_selected_faces_->front_color_ = color_;
		shader_flat_param_selected_faces_->back_color_ = color_;
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

	float32 get_selection_radius_scale_factor_() const { return selection_radius_scale_factor_; }

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
					if (position_->is_valid())
					{
						cells_set_->foreach_cell([&] (cgogn::Dart v)
						{
							selected_points.push_back(position_->operator[](v));
						});
					}
					cgogn::rendering::update_vbo(selected_points, selected_vertices_vbo_.get());
				}
					break;
				case Edge_Cell:
				{
					std::vector<VEC3> selected_segments;
					if (position_->is_valid())
					{
						cells_set_->foreach_cell([&] (cgogn::Dart e)
						{
							auto vertices = map_->vertices(e);
							selected_segments.push_back(position_->operator[](vertices.first));
							selected_segments.push_back(position_->operator[](vertices.second));
						});
					}
					cgogn::rendering::update_vbo(selected_segments, selected_edges_vbo_.get());
				}
					break;
				case Face_Cell:
				{
					std::vector<VEC3> selected_polygons;
					if (position_->is_valid())
					{
						std::vector<uint32> ears;
						if (map_->dimension() == 2) {
							CMap2* map2 = dynamic_cast<CMap2Handler*>(map_)->get_map();
							CMap2Handler::VertexAttribute<VEC3>* pos = dynamic_cast<CMap2Handler::VertexAttribute<VEC3>*>(position_.get());
							cells_set_->foreach_cell([&] (cgogn::Dart f)
							{
								cgogn::geometry::append_ear_triangulation<VEC3>(*map2, CMap2::Face(f), *pos, ears);
							});
						} else {
							CMap3* map3 = dynamic_cast<CMap3Handler*>(map_)->get_map();
							CMap3Handler::VertexAttribute<VEC3>* pos = dynamic_cast<CMap3Handler::VertexAttribute<VEC3>*>(position_.get());
							cells_set_->foreach_cell([&] (cgogn::Dart f)
							{
								cgogn::geometry::append_ear_triangulation<VEC3>(*map3, CMap3::Face(f), *pos, ears);
							});
						}

						for (uint32 i : ears)
							selected_polygons.push_back(position_->operator[](i));
						selected_faces_nb_indices_ = selected_polygons.size();
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

	MapHandlerGen* map_;
	std::unique_ptr<MapHandlerGen::Attribute_T<VEC3>> position_;
	std::unique_ptr<MapHandlerGen::Attribute_T<VEC3>> normal_;
//	MapHandler<CMap2>* map_;
//	MapHandler<CMap2>::VertexAttribute<VEC3> position_;
//	MapHandler<CMap2>::VertexAttribute<VEC3> normal_;

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

	std::unique_ptr<cgogn::rendering::ShaderFlat::Param> shader_flat_param_selected_faces_;
	std::unique_ptr<cgogn::rendering::VBO> selected_faces_vbo_;

	QColor color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;
	float32 selection_radius_scale_factor_;

	bool selecting_;
	cgogn::Dart selecting_vertex_;
	cgogn::Dart selecting_edge_;
	cgogn::Dart selecting_face_;
	std::size_t selecting_face_nb_indices_;
	std::size_t selected_faces_nb_indices_;

	CellsSetGen* cells_set_;

public:

	SelectionMethod selection_method_;
};

/**
* @brief Plugin for cells selection
*/
class SCHNAPPS_PLUGIN_SELECTION_API Plugin_Selection : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class Selection_DockTab;

public:
	using Vertex2 = CMap2::Vertex;
	using Edge2 = CMap2::Edge;
	using Face2 = CMap2::Face;
	using Vertex3 = CMap3::Vertex;
	using Edge3 = CMap3::Edge;
	using Face3 = CMap3::Face;
	using Volume = CMap3::Volume;
	using CollectorGen = cgogn::geometry::CollectorGen<VEC3>;

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

	void view_linked(View*) override;
	void view_unlinked(View*) override;

	std::unique_ptr<CollectorGen> collector_within_sphere(MapHandlerGen* map, float64 radius, const MapHandlerGen::Attribute_T<VEC3>& position_att);
	std::vector<cgogn::Dart> get_picked_cells(MapHandlerGen* map, CellType ct, const MapHandlerGen::Attribute_T<VEC3>& position_att, VEC3& A, VEC3& B);

private slots:

	// slots called from View signals
	void map_linked(MapHandlerGen* map);
	void map_unlinked(MapHandlerGen* map);

	// slots called from MapHandlerGen signals
	void linked_map_cells_set_added(CellType ct, const QString& name);
	void linked_map_cells_set_removed(CellType ct, const QString& name);
	void linked_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void linked_map_attribute_changed(cgogn::Orbit orbit, const QString& name);
	void linked_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void linked_map_bb_changed();

	void update_dock_tab();

private:

	std::unique_ptr<Selection_DockTab> dock_tab_;
	std::map<View*, std::map<MapHandlerGen*, MapParameters>> parameter_set_;
};

} // namespace plugin_selection
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SELECTION_H_
