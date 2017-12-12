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

#include <schnapps/plugins/volume_render/volume_render_dock_tab.h>
#include <schnapps/plugins/volume_render/volume_render.h>

#ifdef USE_TRANSPARENCY
#include <schnapps/plugins/surface_render_transp/surface_render_transp_extern.h>
#endif

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_volume_render
{

VolumeRender_DockTab::VolumeRender_DockTab(SCHNApps* s, Plugin_VolumeRender* p) :
	schnapps_(s),
	plugin_(p),
	current_color_dial_(0),
	updating_ui_(false)
{
	setupUi(this);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(render_vertices_changed(bool)));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(render_faces_changed(bool)));
	connect(check_renderTopology, SIGNAL(toggled(bool)), this, SLOT(render_topology_changed(bool)));
	connect(check_clippingPlane, SIGNAL(toggled(bool)), this, SLOT(apply_clipping_plane_changed(bool)));
	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));
	connect(slider_volumeExplodeFactor, SIGNAL(valueChanged(int)), this, SLOT(volume_explode_factor_changed(int)));

	color_dial_ = new QColorDialog(face_color_, nullptr);
	connect(vertexColorButton, SIGNAL(clicked()), this, SLOT(vertex_color_clicked()));
	connect(edgeColorButton, SIGNAL(clicked()), this, SLOT(edge_color_clicked()));
	connect(faceColorButton, SIGNAL(clicked()), this, SLOT(face_color_clicked()));
	connect(color_dial_, SIGNAL(accepted()), this, SLOT(color_selected()));

	check_useTransparency->setChecked(false);
	slider_transparency->setDisabled(true);
#ifdef USE_TRANSPARENCY
	connect(check_useTransparency, SIGNAL(toggled(bool)), this, SLOT(transparency_enabled_changed(bool)));
	connect(slider_transparency, SIGNAL(valueChanged(int)), this, SLOT(transparency_factor_changed(int)));
#else
	check_useTransparency->setDisabled(true);
#endif

	selected_map_ = schnapps_->get_selected_map();
	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
	}

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
	connect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(selected_map_changed(MapHandlerGen*, MapHandlerGen*)));
}

VolumeRender_DockTab::~VolumeRender_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(update_ui()));
	disconnect(schnapps_, SIGNAL(selected_map_changed(MapHandlerGen*, MapHandlerGen*)), this, SLOT(update_ui()));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void VolumeRender_DockTab::position_vbo_changed(int index)
{
	if (!updating_ui_)
	{
		MapHandlerGen* map = schnapps_->get_selected_map();
		if (map)
			plugin_->set_position_vbo(schnapps_->get_selected_view(), map, map->get_vbo(combo_positionVBO->currentText()), false);
	}
}

void VolumeRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_vertices(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void VolumeRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_edges(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void VolumeRender_DockTab::render_faces_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_faces(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void VolumeRender_DockTab::render_topology_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_render_topology(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void VolumeRender_DockTab::apply_clipping_plane_changed(bool b)
{
	if (!updating_ui_)
		plugin_->set_apply_clipping_plane(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
}

void VolumeRender_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_vertex_scale_factor(schnapps_->get_selected_view(), schnapps_->get_selected_map(), i / 50.0, false);
}

void VolumeRender_DockTab::volume_explode_factor_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_volume_explode_factor(schnapps_->get_selected_view(), schnapps_->get_selected_map(), i / 100.0, false);
}

void VolumeRender_DockTab::transparency_enabled_changed(bool b)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_)
	{
		plugin_->set_transparency_enabled(schnapps_->get_selected_view(), schnapps_->get_selected_map(), b, false);
		update_after_use_transparency_changed();
	}
#endif
}

void VolumeRender_DockTab::transparency_factor_changed(int n)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_)
		plugin_->set_transparency_factor(schnapps_->get_selected_view(), schnapps_->get_selected_map(), n, false);
#endif
}

void VolumeRender_DockTab::vertex_color_clicked()
{
	current_color_dial_ = 1;
	color_dial_->show();
	color_dial_->setCurrentColor(vertex_color_);
}

void VolumeRender_DockTab::edge_color_clicked()
{
	current_color_dial_ = 2;
	color_dial_->show();
	color_dial_->setCurrentColor(edge_color_);
}

void VolumeRender_DockTab::face_color_clicked()
{
	current_color_dial_ = 3;
	color_dial_->show();
	color_dial_->setCurrentColor(face_color_);
}


void VolumeRender_DockTab::color_selected()
{
	QColor col = color_dial_->currentColor();

	View* view = schnapps_->get_selected_view();
	MapHandlerGen* map = schnapps_->get_selected_map();

	if (current_color_dial_ == 1)
	{
		vertex_color_ = col;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_vertex_color(view, map, vertex_color_, false);
	}

	if (current_color_dial_ == 2)
	{
		edge_color_ = col;
		edgeColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_edge_color(view, map, edge_color_, false);
	}

	if (current_color_dial_ == 3)
	{
		face_color_ = col;
		faceColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_face_color(view, map, edge_color_, false);
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void VolumeRender_DockTab::selected_view_changed(View* old, View* cur)
{
	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void VolumeRender_DockTab::selected_map_changed(MapHandlerGen* old, MapHandlerGen* cur)
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
	}
	selected_map_ = cur;
	connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
	connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from MapHandlerGen signals
/*****************************************************************************/

void VolumeRender_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
		combo_positionVBO->addItem(vbo_name);
}

void VolumeRender_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		int index = combo_positionVBO->findText(vbo_name);
		if (index > 0)
			combo_positionVBO->removeItem(index);
	}
}

/*****************************************************************************/
// methods used to update the UI from the plugin
/*****************************************************************************/

void VolumeRender_DockTab::set_position_vbo(cgogn::rendering::VBO* vbo)
{
	updating_ui_ = true;
	if (vbo && vbo->vector_dimension() == 3)
	{
		const QString vbo_name = QString::fromStdString(vbo->name());
		int index = combo_positionVBO->findText(vbo_name);
		if (index > 0)
			combo_positionVBO->setCurrentIndex(index);
	}
	else
		combo_positionVBO->setCurrentIndex(0);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_render_vertices(bool b)
{
	updating_ui_ = true;
	check_renderVertices->setChecked(b);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_render_edges(bool b)
{
	updating_ui_ = true;
	check_renderEdges->setChecked(b);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_render_faces(bool b)
{
	updating_ui_ = true;
	check_renderFaces->setChecked(b);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_render_topology(bool b)
{
	updating_ui_ = true;
	check_renderTopology->setChecked(b);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_apply_clipping_plane(bool b)
{
	updating_ui_ = true;
	check_clippingPlane->setChecked(b);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_vertex_color(const QColor& color)
{
	updating_ui_ = true;
	vertex_color_ = color;
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_edge_color(const QColor& color)
{
	updating_ui_ = true;
	edge_color_ = color;
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_face_color(const QColor& color)
{
	updating_ui_ = true;
	face_color_ = color;
	faceColorButton->setStyleSheet("QPushButton { background-color:" + color.name() + "}");
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_vertex_scale_factor(float sf)
{
	updating_ui_ = true;
	slider_vertexScaleFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_volume_explode_factor(float sf)
{
	updating_ui_ = true;
	slider_volumeExplodeFactor->setSliderPosition(sf * 50.0);
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_transparency_enabled(bool b)
{
	updating_ui_ = true;
	check_useTransparency->setChecked(b);
	update_after_use_transparency_changed();
	updating_ui_ = false;
}

void VolumeRender_DockTab::set_transparency_factor(int tf)
{
	updating_ui_ = true;
	slider_transparency->setValue(tf);
	updating_ui_ = false;
}

void VolumeRender_DockTab::refresh_ui()
{
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	if (!map || !view)
		return;

	const MapParameters& p = plugin_->get_parameters(view, map);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	uint32 i = 1;
	for (const auto& vbo_it : map->get_vbo_set())
	{
		auto& vbo = vbo_it.second;
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo.get() == p.get_position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			++i;
		}
	}

	check_renderVertices->setChecked(p.get_render_vertices());
	slider_vertexScaleFactor->setSliderPosition(p.get_vertex_scale_factor() * 50.0);
	check_renderEdges->setChecked(p.get_render_edges());
	check_renderFaces->setChecked(p.get_render_faces());
	slider_volumeExplodeFactor->setValue(std::round(p.get_volume_explode_factor() * 100.0));
	check_renderTopology->setChecked(p.get_render_topology());

	vertex_color_ = p.get_vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

	edge_color_ = p.get_edge_color();
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + edge_color_.name() + " }");

	face_color_ = p.get_face_color();
	faceColorButton->setStyleSheet("QPushButton { background-color:" + face_color_.name() + " }");

#ifdef USE_TRANSPARENCY
	check_useTransparency->setChecked(p.get_transparency_enabled());
	slider_transparency->setValue(p.get_transparency_factor());
	slider_transparency->setEnabled(p.get_transparency_enabled());
#endif

	check_clippingPlane->setChecked(p.get_apply_clipping_plane());

	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void VolumeRender_DockTab::update_after_use_transparency_changed()
{
	updating_ui_ = true;
	MapHandlerGen* map = schnapps_->get_selected_map();
	View* view = schnapps_->get_selected_view();

	const MapParameters& p = plugin_->get_parameters(view, map);

	slider_transparency->setEnabled(p.get_transparency_enabled());
	if (p.get_transparency_enabled())
		slider_transparency->setValue(p.get_transparency_factor());

	updating_ui_ = false;
}

} // namespace plugin_volume_render

} // namespace schnapps
