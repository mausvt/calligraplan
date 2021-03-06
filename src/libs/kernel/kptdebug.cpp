/* This file is part of the KDE project
  Copyright (C) 2012 Dag Andersen <danders@get2net.dk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

// clazy:excludeall=qstring-arg
#include "kptdebug.h"

const QLoggingCategory &PLANSHARED_LOG()
{
    static const QLoggingCategory category("calligra.plan.shared");
    return category;
}

const QLoggingCategory &PLANDEPEDITOR_LOG()
{
    static const QLoggingCategory category("calligra.plan.dependencyeditor");
    return category;
}

const QLoggingCategory &PLANXML_LOG()
{
    static const QLoggingCategory category("calligra.plan.xml");
    return category;
}

const QLoggingCategory &PLAN_LOG()
{
    static const QLoggingCategory category("calligra.plan");
    return category;
}
