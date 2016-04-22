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

#ifndef SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_
#define SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_

#include "ui_surface_render.h"

#include <QColorDialog>

namespace schnapps
{

class SCHNApps;
class Plugin_SurfaceRender;

//struct MapParameters;

class SurfaceRender_DockTab : public QWidget, public Ui::Surface_Render_TabWidget
{
	Q_OBJECT

	friend class Plugin_SurfaceRender;

public:
	SurfaceRender_DockTab(SCHNApps* s, Plugin_SurfaceRender* p);

private:

	SCHNApps* m_schnapps;
	Plugin_SurfaceRender* m_plugin;

	QColorDialog* m_colorDial;
	QColor m_diffuseColor;
	QColor m_simpleColor;
	QColor m_vertexColor;
	QColor m_backColor;
	int m_currentColorDial;

	bool b_updatingUI;

private slots:

	void positionVBOChanged(int index);
	void normalVBOChanged(int index);
	void colorVBOChanged(int index);
	void renderVerticesChanged(bool b);
	void verticesScaleFactorChanged(int i);
	void verticesScaleFactorPressed();
	void renderEdgesChanged(bool b);
	void renderFacesChanged(bool b);
	void faceStyleChanged(QAbstractButton* b);
	void renderBoundaryChanged(bool b);
	void renderBackfaceChanged(bool b);

	void diffuseColorClicked();
	void simpleColorClicked();
	void vertexColorClicked();
	void backColorClicked();
	void bothColorClicked();
	void colorSelected();

private:

	void addPositionVBO(QString name);
	void removePositionVBO(QString name);
	void addNormalVBO(QString name);
	void removeNormalVBO(QString name);
	void addColorVBO(QString name);
	void removeColorVBO(QString name);

	void updateMapParameters();
};

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_SURFACE_RENDER_DOCK_TAB_H_
