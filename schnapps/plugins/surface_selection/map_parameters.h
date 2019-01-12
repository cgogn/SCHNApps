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

#ifndef SCHNAPPS_PLUGIN_SURFACE_SELECTION_MAP_PARAMETERS_H_
#define SCHNAPPS_PLUGIN_SURFACE_SELECTION_MAP_PARAMETERS_H_

#include <schnapps/plugins/surface_selection/dll.h>

#include <schnapps/plugins/cmap_provider/cmap_provider.h>
#include <schnapps/plugins/cmap_provider/cmap_cells_set.h>

#include <schnapps/core/types.h>
#include <schnapps/core/view.h>

#include <cgogn/rendering/shaders/shader_flat.h>
#include <cgogn/rendering/shaders/shader_point_sprite.h>
#include <cgogn/rendering/shaders/shader_bold_line.h>
#include <cgogn/rendering/shaders/shader_simple_color.h>

namespace schnapps
{

namespace plugin_surface_selection
{

class Plugin_SurfaceSelection;

using CMap2Handler = plugin_cmap_provider::CMap2Handler;
using CMapCellsSetGen = plugin_cmap_provider::CMapCellsSetGen;
template <typename CellType>
using CMap2CellsSet = CMap2Handler::CMap2CellsSet<CellType>;

enum SelectionMethod: unsigned int
{
	SingleCell = 0,
	WithinSphere,
	NormalAngle
};

struct SCHNAPPS_PLUGIN_SURFACE_SELECTION_API MapParameters : public QObject
{
	Q_OBJECT

	friend class Plugin_SurfaceSelection;

public:

	MapParameters() :
		mh_(nullptr),
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
		selection_sphere_vbo_ = new cgogn::rendering::VBO(3);
		selected_vertices_vbo_ = new cgogn::rendering::VBO(3);
		selection_edge_vbo_ = new cgogn::rendering::VBO(3);
		selected_edges_vbo_ = new cgogn::rendering::VBO(3);
		selection_face_vbo_ = new cgogn::rendering::VBO(3);
		selected_faces_vbo_ = new cgogn::rendering::VBO(3);
		initialize_gl();
	}

	~MapParameters()
	{
		delete selection_sphere_vbo_;
		delete selected_vertices_vbo_;
		delete selection_edge_vbo_;
		delete selected_edges_vbo_;
		delete selection_face_vbo_;
		delete selected_faces_vbo_;
	}

	CGOGN_NOT_COPYABLE_NOR_MOVABLE(MapParameters);

	const CMap2::VertexAttribute<VEC3>& position_attribute() const { return position_; }
	const CMap2::VertexAttribute<VEC3>& normal_attribute() const { return normal_; }
	const QColor& color() const { return color_; }
	float32 vertex_base_size() const { return vertex_base_size_; }
	float32 vertex_scale_factor() const { return vertex_scale_factor_; }
	float32 selection_radius_scale_factor() const { return selection_radius_scale_factor_; }
	CMapCellsSetGen* cells_set() const { return cells_set_; }
	SelectionMethod selection_method() const { return selection_method_; }

private:

	void set_position_attribute(const QString& attribute_name)
	{
		position_ = mh_->map()->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name.toStdString());
		update_selected_cells_rendering();
	}

	void set_normal_attribute(const QString& attribute_name)
	{
		normal_ = mh_->map()->get_attribute<VEC3, CMap2::Vertex::ORBIT>(attribute_name.toStdString());
	}

	void set_color(const QColor& c)
	{
		color_ = c;
		shader_point_sprite_param_selected_vertices_->color_ = color_;
		shader_bold_line_param_selected_edges_->color_ = color_;
		shader_flat_param_selected_faces_->front_color_ = color_;
		shader_flat_param_selected_faces_->back_color_ = color_;
	}

	void set_vertex_base_size(float32 bs)
	{
		vertex_base_size_ = bs;
		shader_point_sprite_param_selection_sphere_->size_ = vertex_base_size_ * vertex_scale_factor_;
		shader_point_sprite_param_selected_vertices_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	void set_vertex_scale_factor(float32 sf)
	{
		vertex_scale_factor_ = sf;
		shader_point_sprite_param_selection_sphere_->size_ = vertex_base_size_ * vertex_scale_factor_;
		shader_point_sprite_param_selected_vertices_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	void set_cells_set(CMapCellsSetGen* cs)
	{
		if (cells_set_)
			disconnect(cells_set_, SIGNAL(selected_cells_changed()), this, SLOT(update_selected_cells_rendering()));
		cells_set_ = cs;
		if (cells_set_)
			connect(cells_set_, SIGNAL(selected_cells_changed()), this, SLOT(update_selected_cells_rendering()));
		update_selected_cells_rendering();
	}

	void initialize_gl()
	{
		shader_point_sprite_param_selection_sphere_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_selection_sphere_->color_ = QColor(60, 60, 220, 128);
		shader_point_sprite_param_selection_sphere_->set_position_vbo(selection_sphere_vbo_);

		shader_point_sprite_param_selected_vertices_ = cgogn::rendering::ShaderPointSprite::generate_param();
		shader_point_sprite_param_selected_vertices_->color_ = color_;
		shader_point_sprite_param_selected_vertices_->set_position_vbo(selected_vertices_vbo_);

		shader_bold_line_param_selection_edge_ = cgogn::rendering::ShaderBoldLine::generate_param();
		shader_bold_line_param_selection_edge_->color_ = QColor(60, 60, 220, 128);
		shader_bold_line_param_selection_edge_->width_ = 4.0f;
		shader_bold_line_param_selection_edge_->set_position_vbo(selection_edge_vbo_);

		shader_bold_line_param_selected_edges_ = cgogn::rendering::ShaderBoldLine::generate_param();
		shader_bold_line_param_selected_edges_->color_ = color_;
		shader_bold_line_param_selected_edges_->width_ = 2.0f;
		shader_bold_line_param_selected_edges_->set_position_vbo(selected_edges_vbo_);

		shader_simple_color_param_selection_face_ = cgogn::rendering::ShaderSimpleColor::generate_param();
		shader_simple_color_param_selection_face_->color_ = QColor(60, 60, 220, 128);
		shader_simple_color_param_selection_face_->set_position_vbo(selection_face_vbo_);

		shader_flat_param_selected_faces_ = cgogn::rendering::ShaderFlat::generate_param();
		shader_flat_param_selected_faces_->front_color_ = color_;
		shader_flat_param_selected_faces_->back_color_ = color_;
		shader_flat_param_selected_faces_->set_position_vbo(selected_faces_vbo_);

		drawer_selected_volumes_ = cgogn::make_unique<cgogn::rendering::DisplayListDrawer>();
		drawer_rend_selected_volumes_ = drawer_selected_volumes_->generate_renderer();
	}

private slots:

	void update_selected_cells_rendering()
	{
		if (cells_set_)
		{
			switch (cells_set_->orbit())
			{
				case CMap2::Vertex::ORBIT:
				{
					std::vector<VEC3> selected_points;
					if (position_.is_valid())
					{
						static_cast<CMap2CellsSet<CMap2::Vertex>*>(cells_set_)->foreach_cell([&] (CMap2::Vertex v)
						{
							selected_points.push_back(position_[v]);
						});
					}
					cgogn::rendering::update_vbo(selected_points, selected_vertices_vbo_);
				}
					break;
				case CMap2::Edge::ORBIT:
				{
					std::vector<VEC3> selected_segments;
					if (position_.is_valid())
					{
						static_cast<CMap2CellsSet<CMap2::Edge>*>(cells_set_)->foreach_cell([&] (CMap2::Edge e)
						{
							auto vertices = mh_->map()->vertices(e);
							selected_segments.push_back(position_[vertices.first]);
							selected_segments.push_back(position_[vertices.second]);
						});
					}
					cgogn::rendering::update_vbo(selected_segments, selected_edges_vbo_);
				}
					break;
				case CMap2::Face::ORBIT:
				{
					std::vector<VEC3> selected_polygons;
					if (position_.is_valid())
					{
						std::vector<uint32> ears;
						static_cast<CMap2CellsSet<CMap2::Face>*>(cells_set_)->foreach_cell([&] (CMap2::Face f)
						{
							cgogn::geometry::append_ear_triangulation(*mh_->map(), f, position_, ears);
						});

						for (uint32 i : ears)
							selected_polygons.push_back(position_[i]);
						selected_faces_nb_indices_ = selected_polygons.size();
					}
					cgogn::rendering::update_vbo(selected_polygons, selected_faces_vbo_);
				}
					break;
				default:
					break;
			}

			for (View* view : mh_->linked_views())
				view->update();
		}
	}

private:

	CMap2Handler* mh_;

	CMap2::VertexAttribute<VEC3> position_;
	CMap2::VertexAttribute<VEC3> normal_;

	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_selection_sphere_;
	cgogn::rendering::VBO* selection_sphere_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderPointSprite::Param>	shader_point_sprite_param_selected_vertices_;
	cgogn::rendering::VBO* selected_vertices_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderBoldLine::Param> shader_bold_line_param_selection_edge_;
	cgogn::rendering::VBO* selection_edge_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderBoldLine::Param> shader_bold_line_param_selected_edges_;
	cgogn::rendering::VBO* selected_edges_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderSimpleColor::Param> shader_simple_color_param_selection_face_;
	cgogn::rendering::VBO* selection_face_vbo_;

	std::unique_ptr<cgogn::rendering::ShaderFlat::Param> shader_flat_param_selected_faces_;
	cgogn::rendering::VBO* selected_faces_vbo_;

	std::unique_ptr<cgogn::rendering::DisplayListDrawer> drawer_selected_volumes_;
	std::unique_ptr<cgogn::rendering::DisplayListDrawer::Renderer> drawer_rend_selected_volumes_;

	QColor color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;
	float32 selection_radius_scale_factor_;

	bool selecting_;
	CMap2::Vertex selecting_vertex_;
	CMap2::Edge selecting_edge_;
	CMap2::Face selecting_face_;

	std::size_t selecting_face_nb_indices_;
	std::size_t selected_faces_nb_indices_;

	CMapCellsSetGen* cells_set_;

	SelectionMethod selection_method_;
};

} // namespace plugin_surface_selection

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_SELECTION_MAP_PARAMETERS_H_
