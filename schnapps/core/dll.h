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

#ifndef CORE_DLL_H_
#define CORE_DLL_H_

/**
* \brief Linkage declaration for SCHNApps symbols.
*/
#ifdef WIN32
#ifndef SCHNAPPS_CORE_API
#if defined SCHNAPPS_CORE_DLL_EXPORT
#define SCHNAPPS_CORE_API __declspec(dllexport)
#else
#define SCHNAPPS_CORE_API __declspec(dllimport)
#endif
#endif
#else
#define SCHNAPPS_CORE_API
#endif

#endif // CORE_DLL_H_
