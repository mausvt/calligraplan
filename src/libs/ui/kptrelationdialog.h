/* This file is part of the KDE project
   Copyright (C) 2002 The calligra team <calligra@kde.org>
   Copyright (C) 2003 - 2010 Dag Andersen <danders@get2net.dk>

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

#ifndef KPTRELATIONDIALOG_H
#define KPTRELATIONDIALOG_H

#include "planui_export.h"

#include "ui_relationpanel.h"
#include <KoDialog.h>

#include <QWidget>

namespace KPlato
{

class RelationPanel;

class Relation;
class Project;
class Node;
class MacroCommand;

class RelationPanel : public QWidget, public Ui_RelationPanel
{
    Q_OBJECT
public:
    explicit RelationPanel(QWidget *parent = 0);
};

class PLANUI_EXPORT AddRelationDialog : public KoDialog
{
    Q_OBJECT
public:
    AddRelationDialog(Project &project, Relation *rel, QWidget *p, const QString& caption=QString(), ButtonCodes buttons=Ok|Cancel);
    ~AddRelationDialog();

    virtual MacroCommand *buildCommand();
    int selectedRelationType() const;

protected Q_SLOTS:
    void slotOk();
    void lagChanged();
    void typeClicked(int);
    void slotFinishStartToggled(bool);
    void slotFinishFinishToggled(bool);
    void slotStartStartToggled(bool);
    void slotNodeRemoved(KPlato::Node *node);

protected:
    Project &m_project;
    RelationPanel *m_panel;
    Relation *m_relation;
    bool m_deleterelation;
};


class PLANUI_EXPORT ModifyRelationDialog : public AddRelationDialog
{
    Q_OBJECT
public:
    explicit ModifyRelationDialog(Project &project, Relation *rel, QWidget *p=0);

    virtual MacroCommand *buildCommand();
    bool relationIsDeleted() { return m_deleted; }

protected Q_SLOTS:
    void slotUser1();

    void slotRelationRemoved(KPlato::Relation *relation);

private:
    bool m_deleted;
};

}  //KPlato namespace

#endif // RELATIONDIALOG_H
