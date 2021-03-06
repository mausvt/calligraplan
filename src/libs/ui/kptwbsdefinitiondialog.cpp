/* This file is part of the KDE project
   Copyright (C) 2005 Dag Andersen <danders@get2net.dk>

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

// clazy:excludeall=qstring-arg
#include "kptwbsdefinitiondialog.h"
#include "kptwbsdefinitionpanel.h"
#include "kptwbsdefinition.h"
#include <kptcommand.h>

#include <KLocalizedString>


namespace KPlato
{

WBSDefinitionDialog::WBSDefinitionDialog(Project &project, WBSDefinition &def, QWidget *p)
    : KoDialog(p)
{
    setCaption( i18n("WBS Definition") );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );

    m_panel = new WBSDefinitionPanel(project, def, this);
    setMainWidget(m_panel);
    enableButtonOk(false);
    connect(m_panel, &WBSDefinitionPanel::changed, this, &KoDialog::enableButtonOk);
    connect(this, &KoDialog::okClicked, this, &WBSDefinitionDialog::slotOk);
}


KUndo2Command *WBSDefinitionDialog::buildCommand() {
    return m_panel->buildCommand();
}

void WBSDefinitionDialog::slotOk() {
    if (!m_panel->ok())
        return;
    accept();
}


}  //KPlato namespace
