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
		selection_method_(SingleCell)
	{}

	const typename MapHandler<CMap2>::VertexAttribute<VEC3>& get_position_attribute() const { return position_; }
	QString get_position_attribute_name() const { return QString::fromStdString(position_.name()); }
	void set_position_attribute(const QString& attribute_name)
	{
	}

	const typename MapHandler<CMap2>::VertexAttribute<VEC3>& get_normal_attribute() const { return normal_; }
	QString get_normal_attribute_name() const { return QString::fromStdString(normal_.name()); }
	void set_normal_attribute(const QString& attribute_name)
	{
	}

	const QColor& get_color() const { return color_; }
	void set_color(const QColor& c)
	{
		color_ = c;
//		shader_point_sprite_param_->color_ = color_;
	}

	float32 get_vertex_base_size() const { return vertex_base_size_; }
	void set_vertex_base_size(float32 bs)
	{
		vertex_base_size_ = bs;
//		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

	float32 get_vertex_scale_factor() const { return vertex_scale_factor_; }
	void set_vertex_scale_factor(float32 sf)
	{
		vertex_scale_factor_ = sf;
//		shader_point_sprite_param_->size_ = vertex_base_size_ * vertex_scale_factor_;
	}

private:

	MapHandler<CMap2>* map_;
	typename MapHandler<CMap2>::VertexAttribute<VEC3> position_;
	typename MapHandler<CMap2>::VertexAttribute<VEC3> normal_;

	QColor color_;
	float32 vertex_scale_factor_;
	float32 vertex_base_size_;

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

	inline Plugin_Selection() {}

	~Plugin_Selection() {}

private:

	MapParameters& get_parameters(MapHandlerGen* map);

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
	void selected_map_changed(MapHandlerGen*, MapHandlerGen*);

public slots:


private:

	Selection_DockTab* dock_tab_;
	std::map<MapHandlerGen*, MapParameters> parameter_set_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SELECTION_H_
