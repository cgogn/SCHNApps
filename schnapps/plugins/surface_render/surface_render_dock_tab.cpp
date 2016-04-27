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

#include <surface_render_dock_tab.h>
#include <surface_render.h>

#include <schnapps/core/schnapps.h>
#include <schnapps/core/map_handler.h>
#include <schnapps/core/view.h>

namespace schnapps
{

SurfaceRender_DockTab::SurfaceRender_DockTab(SCHNApps* s, Plugin_SurfaceRender* p) :
	m_schnapps(s),
	m_plugin(p),
	m_currentColorDial(0),
	b_updatingUI(false)

{
	setupUi(this);

	connect(combo_positionVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(positionVBOChanged(int)));
	connect(combo_normalVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(normalVBOChanged(int)));
	connect(combo_colorVBO, SIGNAL(currentIndexChanged(int)), this, SLOT(colorVBOChanged(int)));
	connect(check_renderVertices, SIGNAL(toggled(bool)), this, SLOT(renderVerticesChanged(bool)));
	connect(slider_verticesScaleFactor, SIGNAL(valueChanged(int)), this, SLOT(verticesScaleFactorChanged(int)));
	connect(slider_verticesScaleFactor, SIGNAL(sliderPressed()), this, SLOT(verticesScaleFactorPressed()));
	connect(check_renderEdges, SIGNAL(toggled(bool)), this, SLOT(renderEdgesChanged(bool)));
	connect(check_renderFaces, SIGNAL(toggled(bool)), this, SLOT(renderFacesChanged(bool)));
	connect(group_faceShading, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(faceStyleChanged(QAbstractButton*)));
	connect(check_renderBoundary, SIGNAL(toggled(bool)), this, SLOT(renderBoundaryChanged(bool)));
	connect(check_doubleSided, SIGNAL(toggled(bool)), this, SLOT(renderBackfaceChanged(bool)));

	m_colorDial = new QColorDialog(m_diffuseColor,NULL);
	connect(dcolorButton,SIGNAL(clicked()),this,SLOT(diffuseColorClicked()));
	connect(scolorButton,SIGNAL(clicked()),this,SLOT(simpleColorClicked()));
	connect(vcolorButton,SIGNAL(clicked()),this,SLOT(vertexColorClicked()));
	connect(bfcolorButton, SIGNAL(clicked()), this, SLOT(backColorClicked()));
	connect(bothcolorButton, SIGNAL(clicked()), this, SLOT(bothColorClicked()));
	connect(m_colorDial,SIGNAL(accepted()),this,SLOT(colorSelected()));
}





void SurfaceRender_DockTab::positionVBOChanged(int index)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));
			m_plugin->parameter_set_[view][map].set_position_vbo(map->get_vbo(combo_positionVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRender_DockTab::normalVBOChanged(int index)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_normal_vbo(map->get_vbo(combo_normalVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRender_DockTab::colorVBOChanged(int index)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_color_vbo(map->get_vbo(combo_colorVBO->currentText()));
			view->update();
		}
	}
}

void SurfaceRender_DockTab::renderVerticesChanged(bool b)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			if (b)
				m_plugin->parameter_set_[view][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));

			m_plugin->parameter_set_[view][map].renderVertices = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::verticesScaleFactorPressed()
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].basePSradius = map->get_bb_diagonal_size() / (2 * std::sqrt(map->nb_edges()));
		}
	}
}

void SurfaceRender_DockTab::verticesScaleFactorChanged(int i)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].verticesScaleFactor = i / 50.0;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::renderEdgesChanged(bool b)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].renderEdges = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::renderFacesChanged(bool b)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].renderFaces = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::faceStyleChanged(QAbstractButton* b)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			if (radio_flatShading->isChecked())
				m_plugin->parameter_set_[view][map].faceStyle = MapParameters::FLAT;
			else if (radio_phongShading->isChecked())
				m_plugin->parameter_set_[view][map].faceStyle = MapParameters::PHONG;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::renderBoundaryChanged(bool b)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].renderBoundary = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::renderBackfaceChanged(bool b)
{
	if (!b_updatingUI)
	{
		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].renderBackfaces = b;
			view->update();
		}
	}
}

void SurfaceRender_DockTab::diffuseColorClicked()
{
	m_currentColorDial = 1;
	m_colorDial->show();
	m_colorDial->setCurrentColor(m_diffuseColor);
}

void SurfaceRender_DockTab::simpleColorClicked()
{
	m_currentColorDial = 2;
	m_colorDial->show();
	m_colorDial->setCurrentColor(m_simpleColor);
}

void SurfaceRender_DockTab::vertexColorClicked()
{
	m_currentColorDial = 3;
	m_colorDial->show();
	m_colorDial->setCurrentColor(m_vertexColor);
}

void SurfaceRender_DockTab::backColorClicked()
{
	m_currentColorDial = 4;
	m_colorDial->show();
	m_colorDial->setCurrentColor(m_backColor);
}

void SurfaceRender_DockTab::bothColorClicked()
{
	m_currentColorDial = 5;
	m_colorDial->show();
	m_colorDial->setCurrentColor(m_diffuseColor);
}


void SurfaceRender_DockTab::colorSelected()
{
	QColor col = m_colorDial->currentColor();
	if (m_currentColorDial == 1)
	{
		m_diffuseColor = col;
		dcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		bothcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_diffuse_color(m_diffuseColor);
			view->update();
		}
	}

	if (m_currentColorDial == 2)
	{
		m_simpleColor = col;
		scolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_simple_color(m_simpleColor);
			view->update();
		}
	}

	if (m_currentColorDial == 3)
	{
		m_vertexColor = col;
		vcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_vertex_color(m_vertexColor);
			view->update();
		}
	}

	if (m_currentColorDial == 4)
	{
		m_backColor = col;
		bfcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_back_color(m_backColor);
			view->update();
		}
	}

	if (m_currentColorDial == 5)
	{
		m_backColor = col;
		bfcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		m_diffuseColor = col;
		dcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");
		bothcolorButton->setStyleSheet("QPushButton { background-color:" + col.name() + "}");

		View* view = m_schnapps->get_selected_view();
		MapHandlerGen* map = m_schnapps->get_selected_map();
		if (view && map)
		{
			m_plugin->parameter_set_[view][map].set_back_color(m_backColor);
			m_plugin->parameter_set_[view][map].set_diffuse_color(m_diffuseColor);
			view->update();
		}
	}
}





void SurfaceRender_DockTab::addPositionVBO(QString name)
{
	b_updatingUI = true;
	combo_positionVBO->addItem(name);
	b_updatingUI = false;
}

void SurfaceRender_DockTab::removePositionVBO(QString name)
{
	b_updatingUI = true;
	int curIndex = combo_positionVBO->currentIndex();
	int index = combo_positionVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_positionVBO->setCurrentIndex(0);
	combo_positionVBO->removeItem(index);
	b_updatingUI = false;
}

void SurfaceRender_DockTab::addNormalVBO(QString name)
{
	b_updatingUI = true;
	combo_normalVBO->addItem(name);
	b_updatingUI = false;
}

void SurfaceRender_DockTab::removeNormalVBO(QString name)
{
	b_updatingUI = true;
	int curIndex = combo_normalVBO->currentIndex();
	int index = combo_normalVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_normalVBO->setCurrentIndex(0);
	combo_normalVBO->removeItem(index);
	b_updatingUI = false;
}

void SurfaceRender_DockTab::addColorVBO(QString name)
{
	b_updatingUI = true;
	combo_colorVBO->addItem(name);
	b_updatingUI = false;
}

void SurfaceRender_DockTab::removeColorVBO(QString name)
{
	b_updatingUI = true;
	int curIndex = combo_colorVBO->currentIndex();
	int index = combo_colorVBO->findText(name, Qt::MatchExactly);
	if (curIndex == index)
		combo_colorVBO->setCurrentIndex(0);
	combo_colorVBO->removeItem(index);
	b_updatingUI = false;
}

void SurfaceRender_DockTab::updateMapParameters()
{
	b_updatingUI = true;

	combo_positionVBO->clear();
	combo_positionVBO->addItem("- select VBO -");

	combo_normalVBO->clear();
	combo_normalVBO->addItem("- select VBO -");

	combo_colorVBO->clear();
	combo_colorVBO->addItem("- select VBO -");

	View* view = m_schnapps->get_selected_view();
	MapHandlerGen* map = m_schnapps->get_selected_map();

	if (view && map)
	{
		const MapParameters& p = m_plugin->parameter_set_[view][map];

		unsigned int i = 1;
		foreach(cgogn::rendering::VBO* vbo, map->get_vbo_set().values())
		{
			if (vbo->vector_dimension() == 3)
			{
				combo_positionVBO->addItem(QString::fromStdString(vbo->get_name()));
				if (vbo == p.get_position_vbo())
					combo_positionVBO->setCurrentIndex(i);

				combo_normalVBO->addItem(QString::fromStdString(vbo->get_name()));
				if (vbo == p.get_normal_vbo())
					combo_normalVBO->setCurrentIndex(i);

				combo_colorVBO->addItem(QString::fromStdString(vbo->get_name()));
				if (vbo == p.get_color_vbo())
					combo_colorVBO->setCurrentIndex(i);

				++i;
			}
		}

		check_renderVertices->setChecked(p.renderVertices);
		slider_verticesScaleFactor->setSliderPosition(p.verticesScaleFactor * 50.0);
		check_renderEdges->setChecked(p.renderEdges);
		check_renderFaces->setChecked(p.renderFaces);
		radio_flatShading->setChecked(p.faceStyle == MapParameters::FLAT);
		radio_phongShading->setChecked(p.faceStyle == MapParameters::PHONG);

		m_diffuseColor = p.get_diffuse_color();
		dcolorButton->setStyleSheet("QPushButton { background-color:" + m_diffuseColor.name() + " }");
		bothcolorButton->setStyleSheet("QPushButton { background-color:" + m_diffuseColor.name() + "}");

		m_simpleColor = p.get_simple_color();
		scolorButton->setStyleSheet("QPushButton { background-color:" + m_simpleColor.name() + " }");

		m_vertexColor = p.get_vertex_color();
		vcolorButton->setStyleSheet("QPushButton { background-color:" + m_vertexColor.name() + " }");

		m_backColor = p.get_back_color();
		bfcolorButton->setStyleSheet("QPushButton { background-color:" + m_backColor.name() + " }");
	}

	b_updatingUI = false;
}

} // namespace schnapps
