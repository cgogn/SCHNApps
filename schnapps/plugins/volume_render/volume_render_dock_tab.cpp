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

#include <schnapps/plugins/cmap_provider/cmap_provider.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/view.h>

namespace schnapps
{

namespace plugin_volume_render
{

VolumeRender_DockTab::VolumeRender_DockTab(SCHNApps* s, Plugin_VolumeRender* p) :
	schnapps_(s),
	plugin_(p),
	plugin_cmap_provider_(nullptr),
	selected_map_(nullptr),
	updating_ui_(false),
	color_dial_(nullptr),
	current_color_dial_(0)
{
	setupUi(this);

	connect(list_maps, SIGNAL(itemSelectionChanged()), this, SLOT(selected_map_changed()));

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(position_vbo_changed(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(render_vertices_changed(bool)));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(render_edges_changed(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(render_faces_changed(bool)));
	connect(check_renderTopology, SIGNAL(toggled(bool)), this, SLOT(render_topology_changed(bool)));
	connect(check_clippingPlane, SIGNAL(toggled(bool)), this, SLOT(apply_clipping_plane_changed(bool)));
	connect(slider_vertexScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(vertex_scale_factor_changed(int)));
	connect(slider_volumeExplodeFactor, SIGNAL(valueChanged(int)), this, SLOT(volume_explode_factor_changed(int)));

    connect(list_scalarAttribute, SIGNAL(itemSelectionChanged()), this, SLOT(selected_volume_scalar_changed()));
    connect(combo_colorMap, SIGNAL(currentIndexChanged(int)), this, SLOT(color_map_changed(int)));
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

	connect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));

	View* v = schnapps_->selected_view();
	connect(v, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
	connect(v, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	for (Object* o : v->linked_objects())
		object_linked(o);

	plugin_cmap_provider_ = static_cast<plugin_cmap_provider::Plugin_CMapProvider*>(schnapps_->enable_plugin(plugin_cmap_provider::Plugin_CMapProvider::plugin_name()));
}

VolumeRender_DockTab::~VolumeRender_DockTab()
{
	disconnect(schnapps_, SIGNAL(selected_view_changed(View*, View*)), this, SLOT(selected_view_changed(View*, View*)));
}

/*****************************************************************************/
// slots called from UI signals
/*****************************************************************************/

void VolumeRender_DockTab::selected_map_changed()
{
	if (selected_map_)
	{
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));

        disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, QString)));
        disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));
    }

	selected_map_ = nullptr;

	QList<QListWidgetItem*> currentItems = list_maps->selectedItems();
	if (!currentItems.empty())
	{
		const QString& map_name = currentItems[0]->text();
		selected_map_ = plugin_cmap_provider_->cmap3(map_name);
	}

	if (selected_map_)
	{
		connect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)), Qt::UniqueConnection);
		connect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)), Qt::UniqueConnection);

        connect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, QString)), Qt::UniqueConnection);
        connect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)), Qt::UniqueConnection);
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

void VolumeRender_DockTab::position_vbo_changed(int)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_position_vbo(schnapps_->selected_view(), selected_map_, selected_map_->vbo(combo_positionVBO->currentText()), false);
}

void VolumeRender_DockTab::color_map_changed(int)
{
    if (!updating_ui_ && selected_map_)
        plugin_->set_color_map(schnapps_->selected_view(), selected_map_, combo_colorMap->currentText(), false);
}

void VolumeRender_DockTab::render_vertices_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_vertices(schnapps_->selected_view(), selected_map_, b, false);
}

void VolumeRender_DockTab::render_edges_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_edges(schnapps_->selected_view(), selected_map_, b, false);
}

void VolumeRender_DockTab::render_faces_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_faces(schnapps_->selected_view(), selected_map_, b, false);
}

void VolumeRender_DockTab::render_topology_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_render_topology(schnapps_->selected_view(), selected_map_, b, false);
}

void VolumeRender_DockTab::apply_clipping_plane_changed(bool b)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_apply_clipping_plane(schnapps_->selected_view(), selected_map_, b, false);
}

void VolumeRender_DockTab::vertex_scale_factor_changed(int i)
{
	if (!updating_ui_)
		plugin_->set_vertex_scale_factor(schnapps_->selected_view(), selected_map_, i / 50.0, false);
}

void VolumeRender_DockTab::volume_explode_factor_changed(int i)
{
	if (!updating_ui_ && selected_map_)
		plugin_->set_volume_explode_factor(schnapps_->selected_view(), selected_map_, i / 100.0, false);
}

void VolumeRender_DockTab::transparency_enabled_changed(bool b)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_ && selected_map_)
	{
		plugin_->set_transparency_enabled(schnapps_->selected_view(), selected_map_, b, false);
		update_after_use_transparency_changed();
	}
#endif
}

void VolumeRender_DockTab::transparency_factor_changed(int n)
{
#ifdef USE_TRANSPARENCY
	if (!updating_ui_ && selected_map_)
		plugin_->set_transparency_factor(schnapps_->selected_view(), selected_map_, n, false);
#endif
}

void VolumeRender_DockTab::selected_volume_scalar_changed()
{
    QList<QListWidgetItem*> currentItems = list_scalarAttribute->selectedItems();
    if (currentItems.empty() || currentItems[0]->text() == previousSelection_)
    {
//        if ( item->text().operator==( previousSelection->text() ) ) {
//            previousSelection = NULL;
            list_scalarAttribute->clearSelection();
            list_scalarAttribute->clearFocus();
            previousSelection_.clear();
            plugin_->set_color_per_volume(schnapps_->selected_view(), selected_map_, false, false);
//        }
    }
    else if (!currentItems.empty())
    {
        previousSelection_ = currentItems[0]->text();
        plugin_->set_volume_attribute(schnapps_->selected_view(), selected_map_, previousSelection_, false);
    }
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

	View* view = schnapps_->selected_view();
	CMap3Handler* mh = selected_map_;

	if (current_color_dial_ == 1)
	{
		vertex_color_ = col;
		vertexColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_vertex_color(view, mh, vertex_color_, false);
	}

	if (current_color_dial_ == 2)
	{
		edge_color_ = col;
		edgeColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_edge_color(view, mh, edge_color_, false);
	}

	if (current_color_dial_ == 3)
	{
		face_color_ = col;
		faceColorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		plugin_->set_face_color(view, mh, face_color_, false);
        list_scalarAttribute->selectionModel()->clear();
	}
}

/*****************************************************************************/
// slots called from SCHNApps signals
/*****************************************************************************/

void VolumeRender_DockTab::selected_view_changed(View* old, View* cur)
{
	updating_ui_ = true;
	list_maps->clear();
	updating_ui_ = false;

	if (old)
	{
		disconnect(old, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
		disconnect(old, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
	}
	if (cur)
	{
		connect(cur, SIGNAL(object_linked(Object*)), this, SLOT(object_linked(Object*)));
		connect(cur, SIGNAL(object_unlinked(Object*)), this, SLOT(object_unlinked(Object*)));
		for (Object* o : cur->linked_objects())
			object_linked(o);
	}

	if (plugin_->check_docktab_activation())
		refresh_ui();
}

/*****************************************************************************/
// slots called from View signals
/*****************************************************************************/

void VolumeRender_DockTab::object_linked(Object* o)
{
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
	if (mh)
		map_linked(mh);
}

void VolumeRender_DockTab::map_linked(CMap3Handler *mh)
{
	updating_ui_ = true;
	list_maps->addItem(mh->name());
	updating_ui_ = false;
}

void VolumeRender_DockTab::object_unlinked(Object* o)
{
	CMap3Handler* mh = qobject_cast<CMap3Handler*>(o);
	if (mh)
		map_unlinked(mh);
}

void VolumeRender_DockTab::map_unlinked(CMap3Handler *mh)
{
	if (selected_map_ == mh)
	{
		disconnect(selected_map_, SIGNAL(vbo_added(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_added(cgogn::rendering::VBO*)));
		disconnect(selected_map_, SIGNAL(vbo_removed(cgogn::rendering::VBO*)), this, SLOT(selected_map_vbo_removed(cgogn::rendering::VBO*)));
        disconnect(selected_map_, SIGNAL(attribute_added(cgogn::Orbit, QString)), this, SLOT(selected_map_attribute_added(cgogn::Orbit, QString)));
        disconnect(selected_map_, SIGNAL(attribute_removed(cgogn::Orbit, const QString&)), this, SLOT(selected_map_attribute_removed(cgogn::Orbit, const QString&)));

        selected_map_ = nullptr;
	}

	QList<QListWidgetItem*> items = list_maps->findItems(mh->name(), Qt::MatchExactly);
	if (!items.empty())
	{
		updating_ui_ = true;
		delete items[0];
		updating_ui_ = false;
	}

    list_scalarAttribute->clear();
}

/*****************************************************************************/
// slots called from MapHandlerGen signals
/*****************************************************************************/

void VolumeRender_DockTab::selected_map_vbo_added(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		updating_ui_ = true;
		combo_positionVBO->addItem(vbo_name);
		updating_ui_ = false;
	}
}

void VolumeRender_DockTab::selected_map_vbo_removed(cgogn::rendering::VBO* vbo)
{
	const QString vbo_name = QString::fromStdString(vbo->name());
	if (vbo->vector_dimension() == 3)
	{
		updating_ui_ = true;
		int index = combo_positionVBO->findText(vbo_name);
		if (index > 0)
			combo_positionVBO->removeItem(index);
		updating_ui_ = false;
	}
}

void VolumeRender_DockTab::selected_map_attribute_added(cgogn::Orbit o, QString name)
{
    updating_ui_ = true;
    std::cout << name.toStdString() << std::endl;
//    if (o == CMap3::Vertex::ORBIT)
//           combo_positionAttribute->addItem(name);
    if (o == CMap3::Volume::ORBIT)
        list_scalarAttribute->addItem(name);
    updating_ui_ = false;
}

void VolumeRender_DockTab::selected_map_attribute_removed(cgogn::Orbit o, const QString& name)
{
    /*if (o == CMap2::Vertex::ORBIT)
    {
        int index = combo_positionAttribute->findText(name);
        if (index > 0)
            combo_positionAttribute->removeItem(index);
    }
    else */
    if (o == CMap3::Volume::ORBIT)
    {
        QList<QListWidgetItem*> items = list_scalarAttribute->findItems(name, Qt::MatchExactly);
        if (!items.empty())
            delete items[0];
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
	CMap3Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	if (!mh || !view)
		return;

	const MapParameters& p = plugin_->parameters(view, mh);

	updating_ui_ = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	uint32 i = 1;
	mh->foreach_vbo([&] (cgogn::rendering::VBO* vbo)
	{
		if (vbo->vector_dimension() == 3)
		{
			combo_positionVBO->addItem(QString::fromStdString(vbo->name()));
			if (vbo == p.position_vbo())
				combo_positionVBO->setCurrentIndex(i);

			++i;
		}
	});

    list_scalarAttribute->clear();
    list_scalarAttribute->selectionModel()->clear();
    uint32 j = 0;
    const auto& cacF = mh->map()->attribute_container<CMap3::Volume::ORBIT>();

    for (auto* ptr: cacF.chunk_arrays())
    {
        if(ptr->nb_components() == 1)
            list_scalarAttribute->addItem(QString::fromStdString(ptr->name()));
            ++j;
    }

    combo_colorMap->clear();
    for(int i=0; i!=cgogn::ColorMapType::NB_COLOR_MAP_TYPES; ++i)
    {
        combo_colorMap->addItem(QString::fromStdString(cgogn::color_map_name(cgogn::ColorMapType(i))));
    }
    combo_colorMap->setEnabled(true);

	check_renderVertices->setChecked(p.render_vertices());
	slider_vertexScaleFactor->setSliderPosition(p.vertex_scale_factor() * 50.0);
	check_renderEdges->setChecked(p.render_edges());
	check_renderFaces->setChecked(p.render_faces());
	slider_volumeExplodeFactor->setValue(std::round(p.volume_explode_factor() * 100.0));
	check_renderTopology->setChecked(p.render_topology());

	vertex_color_ = p.vertex_color();
	vertexColorButton->setStyleSheet("QPushButton { background-color:" + vertex_color_.name() + " }");

	edge_color_ = p.edge_color();
	edgeColorButton->setStyleSheet("QPushButton { background-color:" + edge_color_.name() + " }");

	face_color_ = p.face_color();
	faceColorButton->setStyleSheet("QPushButton { background-color:" + face_color_.name() + " }");

#ifdef USE_TRANSPARENCY
	check_useTransparency->setChecked(p.transparency_enabled());
	slider_transparency->setValue(p.transparency_factor());
	slider_transparency->setEnabled(p.transparency_enabled());
#endif

	check_clippingPlane->setChecked(p.apply_clipping_plane());

	updating_ui_ = false;
}

/*****************************************************************************/
// internal UI cascading updates
/*****************************************************************************/

void VolumeRender_DockTab::update_after_use_transparency_changed()
{
	updating_ui_ = true;
	CMap3Handler* mh = selected_map_;
	View* view = schnapps_->selected_view();

	const MapParameters& p = plugin_->parameters(view, mh);

	slider_transparency->setEnabled(p.transparency_enabled());
	if (p.transparency_enabled())
		slider_transparency->setValue(p.transparency_factor());

	updating_ui_ = false;
}

} // namespace plugin_volume_render

} // namespace schnapps
