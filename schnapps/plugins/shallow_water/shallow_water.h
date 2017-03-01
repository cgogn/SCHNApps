/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
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

#ifndef SCHNAPPS_PLUGIN_SHALLOW_WATER_H_
#define SCHNAPPS_PLUGIN_SHALLOW_WATER_H_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <schnapps/core/map_handler.h>

#include <shallow_water_dock_tab.h>

namespace schnapps
{

namespace plugin_shallow_water
{

/**
* @brief Shallow water simulation
*/
class SCHNAPPS_PLUGIN_SHALLOW_WATER_API Plugin_ShallowWater : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	Plugin_ShallowWater() {}
	~Plugin_ShallowWater() override {}

private:

	bool enable() override;
	void disable() override;

	void draw(View*, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override {}

	void keyPress(View*, QKeyEvent*) override {}
	void keyRelease(View*, QKeyEvent*) override {}
	void mousePress(View*, QMouseEvent*) override {}
	void mouseRelease(View*, QMouseEvent*) override {}
	void mouseMove(View*, QMouseEvent*) override {}
	void wheelEvent(View*, QWheelEvent*) override {}
	void resizeGL(View* /*view*/, int /*width*/, int /*height*/) override {}

	void view_linked(View*) override;
	void view_unlinked(View*) override;

public slots:

	void init();
	void start();

private slots:

	void update_dock_tab();
	void execute_time_step();

private:

	struct Flux
	{
		SCALAR F1;
		SCALAR F2;
		SCALAR S0L;
		SCALAR S0R;
	};

	struct Flux Solv_HLL(
		SCALAR zbL, SCALAR zbR,
		SCALAR PhiL, SCALAR PhiR,
		SCALAR hL, SCALAR hR,
		SCALAR qL, SCALAR qR,
		SCALAR hmin, SCALAR g
	);

	std::pair<CMap2::Edge, CMap2::Edge> get_LR_edges(CMap2::Face f);
	std::pair<CMap2::Face, CMap2::Face> get_LR_faces(CMap2::Edge e);

	ShallowWater_DockTab* dock_tab_;

	SCALAR t_;
	SCALAR dt_;
	QTimer* timer_;

	MapHandler<CMap2>* map_;
	CMap2* map2_;
	CMap2::Edge boundaryL_, boundaryR_;

	CMap2::VertexAttribute<VEC3> position_; // vertices position

	CMap2::VertexAttribute<SCALAR> water_height_;
	CMap2::VertexAttribute<VEC3> water_position_;

	CMap2::FaceAttribute<SCALAR> h_;        // water height
	CMap2::FaceAttribute<SCALAR> h_tmp_;
	CMap2::FaceAttribute<SCALAR> q_;        // water flow
	CMap2::FaceAttribute<SCALAR> q_tmp_;
	CMap2::FaceAttribute<VEC3> centroid_;   // cell centroid
	CMap2::FaceAttribute<SCALAR> length_;   // cell length
	CMap2::FaceAttribute<SCALAR> phi_;      // cell width

	CMap2::EdgeAttribute<SCALAR> f1_;
	CMap2::EdgeAttribute<SCALAR> f2_;
	CMap2::EdgeAttribute<SCALAR> s0L_;
	CMap2::EdgeAttribute<SCALAR> s0R_;
};

} // namespace plugin_shallow_water

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SHALLOW_WATER_H_
