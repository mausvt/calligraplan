/* This file is part of the KDE project
   Copyright (C) 2009 Dag Andersen <danders@get2net.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation;; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KPTLOCALECONFIGMONEYDIALOG_H
#define KPTLOCALECONFIGMONEYDIALOG_H

#include "planui_export.h"

#include <KoDialog.h>

class KUndo2Command;

namespace KPlato
{

class Locale;
class LocaleConfigMoney;
class Project;

class PLANUI_EXPORT LocaleConfigMoneyDialog : public KoDialog {
    Q_OBJECT
public:
    explicit LocaleConfigMoneyDialog( Locale *locale, QWidget *parent=0);

    KUndo2Command *buildCommand( Project &project );

protected Q_SLOTS:
    void slotChanged();

private:
    LocaleConfigMoney *m_panel;

};

} //KPlato namespace

#endif // KPLATO_LOCALECONFIGMONEYDIALOG_H
