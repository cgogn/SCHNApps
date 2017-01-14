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

#ifndef SCHNAPPS_CORE_PLUGIN_H_
#define SCHNAPPS_CORE_PLUGIN_H_

#include <schnapps/core/dll.h>
#include <schnapps/core/settings.h>

#include <QtPlugin>

namespace schnapps
{

class SCHNApps;

class SCHNAPPS_CORE_API Plugin : public QObject
{
	Q_OBJECT

	friend class SCHNApps;

public:

	inline Plugin() {}
	virtual ~Plugin();

	inline const QString& get_name() const { return name_; }

public slots:

	/**
	 * @brief get the name of Plugin object
	 * @return name
	 */
	inline QString get_name() { return name_; }

	/**
	 * @brief get the file path to the plugin library file
	 * @return file path
	 */
	inline QString get_file_path() { return file_path_; }

	/**
	 * @brief get the schnapps objet ptr
	 * @return the ptr
	 */
	inline SCHNApps* get_schnapps() const { return schnapps_; }

	const QVariant get_setting(const QString& name) const;
	void add_setting(const QString& name, const QVariant& val);

private:

	inline void set_name(const QString& name) { name_ = name; }

	inline void set_file_path(const QString& f) { file_path_ = f; }

	inline void set_schnapps(SCHNApps* s) { schnapps_ = s; }

	virtual bool enable() = 0;
	virtual void disable() = 0;

protected:

	// plugin name
	QString name_;

	// file path to the plugin library file
	QString file_path_;

	// pointer to schnapps object
	SCHNApps* schnapps_;
};

} // namespace schnapps

Q_DECLARE_INTERFACE(schnapps::Plugin, "SCHNApps.Plugin")

#endif // SCHNAPPS_CORE_PLUGIN_H_
