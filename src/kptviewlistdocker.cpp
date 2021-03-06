/* This file is part of the KDE project
 * Copyright (C) 2007 Fredy Yanardi <fyanardi@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// clazy:excludeall=qstring-arg
#include "kptviewlistdocker.h"

#include "kptviewlist.h"

#include "kptview.h"
#include "Help.h"
#include "kptdebug.h"

#include <KLocalizedString>


namespace KPlato
{

ViewListDocker::ViewListDocker(View *view)
{
    updateWindowTitle( false );
    setView(view);
}

ViewListDocker::~ViewListDocker()
{
}

View *ViewListDocker::view()
{
    return m_view;
}

void ViewListDocker::setView(View *view)
{
    m_view = view;
    QWidget *wdg = widget();
    if (wdg)
        delete wdg;
    m_viewlist = new ViewListWidget(view->getPart(), this);
    setWhatsThis(m_viewlist->whatsThis());
    setWidget(m_viewlist);
    m_viewlist->setProject( &( view->getProject() ) );
    connect(m_viewlist, &ViewListWidget::selectionChanged, view, &View::slotSelectionChanged);
    connect(view, &View::currentScheduleManagerChanged, m_viewlist, &ViewListWidget::setSelectedSchedule);
    connect(m_viewlist, &ViewListWidget::updateViewInfo, view, &View::slotUpdateViewInfo);

}

void ViewListDocker::slotModified()
{
    setWindowTitle( xi18nc( "@title:window", "View Selector [modified]" ) );
}

void ViewListDocker::updateWindowTitle( bool modified )
{
    if ( modified ) {
        setWindowTitle( xi18nc( "@title:window", "View Selector [modified]" ) );
    } else {
        setWindowTitle(xi18nc( "@title:window", "View Selector"));
    }
}

//----------
ViewListDockerFactory::ViewListDockerFactory(View *view)
{
    m_view = view;
}

QString ViewListDockerFactory::id() const
{
    return QString("KPlatoViewList");
}

QDockWidget* ViewListDockerFactory::createDockWidget()
{
    ViewListDocker *widget = new ViewListDocker(m_view);
    widget->setObjectName(id());

    return widget;
}

} //namespace KPlato
