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
		positionVBO(NULL),
		normalVBO(NULL),
		colorVBO(NULL),
		verticesScaleFactor(1.0f),
		renderVertices(false),
		renderEdges(false),
		renderFaces(true),
		faceStyle(FLAT),
		diffuseColor(0.85f,0.25f,0.19f,0.0f),
		simpleColor(0.0f,0.0f,0.0f,0.0f),
		vertexColor(0.0f,0.0f,1.0f,0.0f),
		backColor(0.85f, 0.25f, 0.19f, 0.0f)
	{}

	cgogn::rendering::VBO* positionVBO;
	cgogn::rendering::VBO* normalVBO;
	cgogn::rendering::VBO* colorVBO;

	float verticesScaleFactor;
	float basePSradius;
	bool renderVertices;
	bool renderEdges;
	bool renderFaces;
	bool renderBoundary;
	bool renderBackfaces;
	FaceShadingStyle faceStyle;

	VEC4 diffuseColor;
	VEC4 simpleColor;
	VEC4 vertexColor;
	VEC4 backColor;
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

public slots:



private:

	SurfaceRender_DockTab* dock_tab_;
	QHash<View*, QHash<MapHandlerGen*, MapParameters> > parameter_set_;

	cgogn::rendering::ShaderFlat* shader_flat_;
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_H_
