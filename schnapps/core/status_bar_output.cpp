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

#include <schnapps/core/status_bar_output.h>
#include <QStatusBar>

namespace schnapps
{

StatusBarOutput::StatusBarOutput(QStatusBar* status_bar) :
	QObject(nullptr),
	cgogn::logger::LoggerOutput(),
	status_bar_(status_bar)
{
	if (status_bar_)
	{
		connect(status_bar, SIGNAL(messageChanged(QString)), this, SLOT(status_bar_display_changed(QString)));
	}
}

void StatusBarOutput::process_entry(const StatusBarOutput::LogEntry& e)
{
	if (status_bar_)
	{
		std::stringstream sstream;
		sstream << "[" << cgogn::logger::internal::loglevel_to_string(e.get_level()) << "]";
		if (!e.get_sender().empty())
		{
			sstream << '(' << e.get_sender() << ')';
		}
		sstream << ": " << e.get_message_str();
//		if (!e.get_fileinfo().empty())
//			sstream << " (file " << e.get_fileinfo() << ')';
		if (status_bar_->currentMessage().isEmpty())
			status_bar_->showMessage(QString::fromStdString(sstream.str()), 2500);
		else
			message_buffer_.push_back(QString::fromStdString(sstream.str()));
	}
}

void StatusBarOutput::status_bar_display_changed(const QString& msg)
{
	if (msg.isEmpty() && !message_buffer_.empty())
	{
		status_bar_->showMessage(message_buffer_.back(), 2500);
		message_buffer_.pop_back();
	}
}

} // namespace schnapps
