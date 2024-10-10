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

#ifndef SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP0_HANDLER_H_
#define SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP0_HANDLER_H_

#include <schnapps/plugins/cmap_provider/plugin_cmap_provider_export.h>

#include <schnapps/plugins/cmap_provider/cmap_handler.h>
#include <schnapps/core/types.h>
#include <schnapps/core/object.h>

#include <cgogn/geometry/algos/bounding_box.h>
#include <cgogn/core/cmap/cmap0.h>
#include <cgogn/rendering/map_render.h>

namespace schnapps
{

class SCHNApps;
class PluginProvider;

namespace plugin_cmap_provider
{

class CMapCellsSetGen;
template <typename CMapHandler, typename CellType> class CMapCellsSet;

class PLUGIN_CMAP_PROVIDER_EXPORT CMap0Handler : public CMapHandlerGen
{
	Q_OBJECT

public:

	using Self = CMap0Handler;
	using MapType = CMap0;

	template <typename CellType>
	using CMap0CellsSet = CMapCellsSet<Self, CellType>;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	CMap0Handler(const QString& name, PluginProvider* p);
	~CMap0Handler();


	/**********************************************************
	 * BASIC FUNCTIONS                                        *
	 *********************************************************/

	CMapType type() const override;
	CMap0* map() override;
	const CMap0* map() const override;

	void foreach_cell(cgogn::Orbit orb, const std::function<void(cgogn::Dart)>& func) const override;

	/**********************************************************
	 * MANAGE DRAWING                                         *
	 *********************************************************/

	void draw(cgogn::rendering::DrawingType primitive) override;
	void init_primitives(cgogn::rendering::DrawingType primitive) override;

	/**********************************************************
	 * MANAGE VBOs                                            *
	 *********************************************************/

	virtual cgogn::rendering::VBO* create_vbo(const QString& name) override;
	virtual void update_vbo(const QString& name) override;


	/**********************************************************
	 * MANAGE CELLS SETS                                      *
	 *********************************************************/

	CMapCellsSetGen* new_cell_set(cgogn::Orbit orbit, const QString& name) override;

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/
	void compute_bb() override;

protected:
	std::unique_ptr<cgogn::Attribute_T<VEC3>> get_bb_vertex_attribute(const QString& attribute_name) const override;

	/**********************************************************
	 * MANAGE ATTRIBUTES & CONNECTIVITY                       *
	 *********************************************************/

public:

	template <typename T, cgogn::Orbit ORBIT>
	cgogn::Attribute<T, ORBIT> add_attribute(const QString& attribute_name)
	{
		cgogn::Attribute<T, ORBIT> a = map()->template add_attribute<T, ORBIT>(attribute_name.toStdString());
		if (a.is_valid())
			notify_attribute_added(ORBIT, attribute_name);
		return a;
	}
};


} // namespace plugin_cmap_provider

} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CMAP2_PROVIDER_CMAP0_HANDLER_H_

