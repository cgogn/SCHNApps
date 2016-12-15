/*******************************************************************************
* SCHNApps                                                                     *
* Copyright (C) 2016, IGG Group, ICube, University of Strasbourg, France       *
* Merge plugin                                                                 *
* Author Etienne Schmitt (etienne.schmitt@inria.fr) Inria/Mimesis              *
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

#ifndef SCHNAPPS_PLUGIN_EMPTY_PLUGIN_EMPTY_PLUGIN_H_
#define SCHNAPPS_PLUGIN_EMPTY_PLUGIN_EMPTY_PLUGIN_H_

#include "dll.h"
#include <schnapps/core/plugin_processing.h>
#include <QAction>

namespace schnapps
{

namespace merge_plugin
{

class MergeDialog;
/**
* @brief Empty plugin example
*/
class SCHNAPPS_PLUGIN_MERGE_PLUGIN_API MergePlugin : public PluginProcessing
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "SCHNApps.Plugin")
	Q_INTERFACES(schnapps::Plugin)

public:

	MergePlugin();
	~MergePlugin() override;

private:

	bool enable() override;
	void disable() override;

	QAction* merge_action_;
	MergeDialog* merge_dialog_;

	public slots:
private slots:
	void merge_dialog();
};

} // namespace merge_plugin
} // namespace schnapps

#endif // SCHNAPPS_PLUGIN_EMPTY_PLUGIN_EMPTY_PLUGIN_H_
