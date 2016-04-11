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

#ifndef SCHNAPPS_CORE_PLUGIN_INTERACTION_H_
#define SCHNAPPS_CORE_PLUGIN_INTERACTION_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/plugin.h>

#include <QKeyEvent>

namespace cgogn { namespace rendering { class ShaderProgram; } }

namespace schnapps
{

class View;

class SCHNAPPS_CORE_API PluginInteraction : public Plugin
{
	Q_OBJECT

	friend class View;

public:

	PluginInteraction()
	{}

	~PluginInteraction() override
	{}

public slots:

	/**
	 * @brief get the list of views linked to the plugin
	 * @return the list
	 */
	const QList<View*>& get_linked_views() const;

	/**
	 * @brief is the plugin linked to the given view
	 * @param view
	 * @return
	 */
	bool is_linked_to_view(View* view) const;

private:

	virtual void draw(View* view) = 0;
//	virtual void drawMap(View* view, MapHandlerGen* map) = 0;

	virtual void keyPress(View* view, QKeyEvent* event) = 0;
	virtual void keyRelease(View* view, QKeyEvent* event) = 0;
	virtual void mousePress(View* view, QMouseEvent* event) = 0;
	virtual void mouseRelease(View* view, QMouseEvent* event) = 0;
	virtual void mouseMove(View* view, QMouseEvent* event) = 0;
	virtual void wheelEvent(View* view, QWheelEvent* event) = 0;

	virtual void view_linked(View* view) = 0;
	virtual void view_unlinked(View* view) = 0;

	void link_view(View* view);
	void unlink_view(View* view);

	void register_shader(cgogn::rendering::ShaderProgram* sp);
	void register_shaders(const std::vector<cgogn::rendering::ShaderProgram*>& sp);

	void unregister_shader(cgogn::rendering::ShaderProgram* sp);
	void unregister_shaders(const std::vector<cgogn::rendering::ShaderProgram*>& sp);

protected:

	// list of views that are linked to this plugin
	QList<View*> views_;

	// list of shaders that are used by this plugin
	QList<cgogn::rendering::ShaderProgram*> shaders_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_PLUGIN_INTERACTION_H_
