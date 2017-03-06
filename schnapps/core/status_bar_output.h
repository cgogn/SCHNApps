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

#ifndef SCHNAPPS_CORE_STATUS_BAR_OUTPUT_H_
#define SCHNAPPS_CORE_STATUS_BAR_OUTPUT_H_

#include <schnapps/core/dll.h>
#include <cgogn/core/utils/logger_output.h>
#include <QObject>
#include <QStringList>

class QStatusBar;

namespace schnapps
{

class SCHNAPPS_CORE_API StatusBarOutput final : public QObject, public  cgogn::logger::LoggerOutput
{
	Q_OBJECT

public:
	using LogEntry = cgogn::logger::LogEntry;
	StatusBarOutput(QStatusBar* status_bar);
	CGOGN_NOT_COPYABLE_NOR_MOVABLE(StatusBarOutput);

	virtual void process_entry(const LogEntry&) override;

private slots:
	void status_bar_display_changed(const QString& msg);
private:
	QStatusBar* status_bar_;
	QStringList message_buffer_;
};

} // namespace schnapps

#endif // SCHNAPPS_CORE_STATUS_BAR_OUTPUT_H_
