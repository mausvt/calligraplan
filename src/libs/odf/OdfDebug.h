/*
 *  Copyright (c) 2015 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef ODF_DEBUG_H_
#define ODF_DEBUG_H_

#include <QDebug>
#include <QLoggingCategory>
#include <koodf_export.h>

extern const KOODF_EXPORT QLoggingCategory &ODF_LOG();

#define debugOdf qCDebug(ODF_LOG)<<Q_FUNC_INFO
#define warnOdf qCWarning(ODF_LOG)
#define errorOdf qCCritical(ODF_LOG)

#endif
