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

#ifndef SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP_HANDLER_H_
#define SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP_HANDLER_H_

#include <schnapps/plugins/cmap_provider/plugin_cmap_provider_export.h>
#include <schnapps/core/types.h>
#include <schnapps/core/object.h>
#include <cgogn/rendering/map_render.h>
#include <functional>

namespace schnapps
{

namespace plugin_cmap_provider
{

class CMapCellsSetGen;


class PLUGIN_CMAP_PROVIDER_EXPORT CMapHandlerGen : public Object
{
	Q_OBJECT

public:

	using Self = CMapHandlerGen;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

public:

	CMapHandlerGen(const QString& name, PluginProvider* p);
	~CMapHandlerGen() override;

	virtual MapBaseData* map();
	virtual const MapBaseData* map() const;

	void view_linked(View*);
	void view_unlinked(View*);

	/**********************************************************
	 * MANAGE DRAWING                                         *
	 *********************************************************/

	virtual void draw(cgogn::rendering::DrawingType primitive) = 0;
	virtual void init_primitives(cgogn::rendering::DrawingType primitive) = 0;

	/**********************************************************
	 * MANAGE VBOs                                            *
	 *********************************************************/

	cgogn::rendering::VBO* vbo(const QString& name) const;
	void delete_vbo(const QString& name);
	void foreach_vbo(const std::function<void(cgogn::rendering::VBO*)>& f) const;
	virtual cgogn::rendering::VBO* create_vbo(const QString& name) = 0;
	virtual void update_vbo(const QString& name) = 0;

	/**********************************************************
	 * MANAGE CELLS SETS                                      *
	 *********************************************************/

	CMapCellsSetGen* add_cells_set(cgogn::Orbit orb, const QString& name);
	void remove_cells_set(cgogn::Orbit orbit, const QString& name);
	CMapCellsSetGen* cells_set(cgogn::Orbit orbit, const QString& name);
	void foreach_cells_set(cgogn::Orbit orbit, const std::function<void(CMapCellsSetGen*)>& f) const;
	void notify_cells_set_mutually_exclusive_change(cgogn::Orbit orb, const QString& name) const;

protected:
	virtual CMapCellsSetGen* new_cell_set(cgogn::Orbit orbit, const QString& name) = 0;

public:

	/*********************************************************
	 * MANAGE BOUNDING BOX
	 *********************************************************/

	QString bb_vertex_attribute_name() const;
	void set_bb_vertex_attribute(const QString& attribute_name);
	void check_bb_vertex_attribute(cgogn::Orbit orbit, const QString& attribute_name);
	void compute_bb();

protected:
	virtual std::unique_ptr<cgogn::Attribute_T<VEC3>> get_bb_vertex_attribute(const QString& attribute_name) const = 0;

private:


	/**********************************************************
	 * MANAGE ATTRIBUTES & CONNECTIVITY                       *
	 *********************************************************/

public:

	bool remove_attribute(cgogn::Orbit orbit, const QString& att_name);
	template <typename T, cgogn::Orbit ORBIT>
	bool remove_attribute(const cgogn::Attribute<T, ORBIT>& ah)
	{
		if (ah.is_valid())
			remove_attribute(ORBIT, QString::fromStdString(ah.name()));
		else
			return false;
	}


	void notify_attribute_added(cgogn::Orbit, const QString&);
	void notify_attribute_removed(cgogn::Orbit, const QString&);
	void notify_attribute_change(cgogn::Orbit, const QString&);

	void notify_connectivity_change();

	void lock_topo_access();
	void unlock_topo_access();


signals:

	void bb_vertex_attribute_changed(const QString&);

	void vbo_added(cgogn::rendering::VBO*);
	void vbo_removed(cgogn::rendering::VBO*);

	void attribute_added(cgogn::Orbit, const QString&);
	void attribute_removed(cgogn::Orbit, const QString&);
	void attribute_changed(cgogn::Orbit, const QString&);

	void cells_set_added(cgogn::Orbit, const QString&);
	void cells_set_removed(cgogn::Orbit, const QString&);
	void cells_set_selected_cells_changed(cgogn::Orbit, const QString&);
	void cells_set_mutually_exclusive_changed(cgogn::Orbit, const QString&) const;

	void connectivity_changed();

protected:
	cgogn::MapBaseData* map_;

	std::mutex topo_access_;

	// MapRender object of the map
	cgogn::rendering::MapRender render_;

	// VBO managed for the map attributes
	std::map<QString, std::unique_ptr<cgogn::rendering::VBO>> vbos_;

	// CellsSets of the map
	std::array<std::map<QString, CMapCellsSetGen*>, cgogn::NB_ORBITS> cells_sets_;

	std::unique_ptr<cgogn::Attribute_T<VEC3>> bb_vertex_attribute_;
};

} // namespace plugin_cmap_provider
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_CMAP_PROVIDER_CMAP_HANDLER_H_
