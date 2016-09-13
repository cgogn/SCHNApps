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

#ifndef SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_
#define SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_

#include "dll.h"
#include <schnapps/core/plugin_interaction.h>
#include <volume_modelisation_docktab.h>
#include <volume_operation.h>

#include <cgogn/core/basic/cell.h>
#include <memory>

namespace schnapps
{

namespace plugin_volume_modelisation
{

class SCHNAPPS_PLUGIN_VOLUME_MODELISATION_API VolumeModelisationPlugin : public PluginInteraction
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

	friend class VolumeModelisation_DockTab;

public:
	VolumeModelisationPlugin();
	~VolumeModelisationPlugin() override;

private:
	bool enable() override;
	void disable() override;

	public slots:
	void process_operation();
	private slots:
	void current_map_changed(MapHandlerGen* prev, MapHandlerGen* next);
	void current_cells_set_added(CellType ct, const QString& name);
	void current_cells_set_removed(CellType ct, const QString& name);
	void current_map_attribute_added(cgogn::Orbit orbit, const QString& name);
	void current_map_attribute_removed(cgogn::Orbit orbit, const QString& name);
	void update_dock_tab();

private:
	virtual void draw(View* view, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	virtual void draw_map(View* view, MapHandlerGen* map, const QMatrix4x4& proj, const QMatrix4x4& mv) override;
	virtual void keyPress(View* view, QKeyEvent* event) override;
	virtual void keyRelease(View* view, QKeyEvent* event) override;
	virtual void mousePress(View* view, QMouseEvent* event) override;
	virtual void mouseRelease(View* view, QMouseEvent* event) override;
	virtual void mouseMove(View* view, QMouseEvent* event) override;
	virtual void wheelEvent(View* view, QWheelEvent* event) override;
	virtual void view_linked(View* view) override;
	virtual void view_unlinked(View* view) override;

	std::unique_ptr<VolumeModelisation_DockTab> docktab_;
	std::unique_ptr<VolumeOperation> operations_;
};

} // namespace plugin_volume_modelisation
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_VOLUME_MODELISATION_VOLUME_MODELISATION_
