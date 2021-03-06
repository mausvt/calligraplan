/* This file is part of the KDE project
   Copyright (C) 2005-2007 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KPTMILESTONEPROGRESSPANEL_H
#define KPTMILESTONEPROGRESSPANEL_H

#include "planui_export.h"

#include "ui_kptmilestoneprogresspanelbase.h"
#include "kpttask.h"


namespace KPlato
{

class MacroCommand;

class MilestoneProgressPanelImpl : public QWidget, public Ui_MilestoneProgressPanelBase {
    Q_OBJECT
public:
    explicit MilestoneProgressPanelImpl(QWidget *parent=0, const char *name=0);
    
    void enableWidgets();

Q_SIGNALS:
    void changed();
    
public Q_SLOTS:
    void slotChanged();
    void slotFinishedChanged(bool state);
};

class MilestoneProgressPanel : public MilestoneProgressPanelImpl {
    Q_OBJECT
public:
    explicit MilestoneProgressPanel(Task &task, QWidget *parent=0, const char *name=0);

    MacroCommand *buildCommand();

private:
    Task &m_task;
    Completion &m_completion;
};

}  //KPlato namespace

#endif // MILESTONEPROGRESSPANEL_H
