/* This file is part of the KDE project
  Copyright (C) 2007 - 2009 Dag Andersen <danders@get2net.dk>

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

#ifndef TASKWORKPACKAGEVIEW_H
#define TASKWORKPACKAGEVIEW_H

#include "planwork_export.h"

#include "kptitemmodelbase.h"

#include "kptviewbase.h"
#include "kptganttview.h"

#include <KGanttView>

#include <QSplitter>

class QItemSelection;


namespace KPlato
{

class Project;
class Node;
class Document;

}
using namespace KPlato;

namespace KPlatoWork
{
class Part;
class WorkPackage;

class TaskWorkPackageModel;

class PLANWORK_EXPORT TaskWorkPackageTreeView : public DoubleTreeViewBase
{
    Q_OBJECT
public:
    TaskWorkPackageTreeView( Part *part, QWidget *parent );
    
    
    //void setSelectionModel( QItemSelectionModel *selectionModel );

    TaskWorkPackageModel *itemModel() const;
    
    Project *project() const;
    void setProject( Project *project );

    Document *currentDocument() const;
    Node *currentNode() const;
    QList<Node*> selectedNodes() const;

Q_SIGNALS:
    void sectionsMoved();

protected Q_SLOTS:
    void slotActivated( const QModelIndex &index );
    void setSortOrder( int col, Qt::SortOrder order );

protected:
    void dragMoveEvent(QDragMoveEvent *event);
};


class PLANWORK_EXPORT AbstractView : public QWidget, public ViewActionLists
{
    Q_OBJECT
public:
    AbstractView( Part *part, QWidget *parent );
    
    /// reimplement
    virtual void updateReadWrite( bool readwrite );
    /// reimplement
    virtual Node *currentNode() const;
    /// reimplement
    virtual Document *currentDocument() const;
    /// reimplement
    virtual QList<Node*> selectedNodes() const;
    
    /// Loads context info into this view. Reimplement.
    virtual bool loadContext();
    /// Save context info from this view. Reimplement.
    virtual void saveContext();

    /// reimplement
    virtual KoPrintJob *createPrintJob();
    
Q_SIGNALS:
    void requestPopupMenu( const QString& name, const QPoint &pos );
    void selectionChanged();

protected Q_SLOTS:
    /// Builds menu from action list
    virtual void slotHeaderContextMenuRequested( const QPoint &pos );

    /// Reimplement if you have index specific context menu, standard calls slotHeaderContextMenuRequested()
    void slotContextMenuRequested( const QModelIndex &index, const QPoint& pos );

    /// Should not need to be reimplemented
    virtual void slotContextMenuRequested(KPlato::Node *node, const QPoint& pos);
    /// Should not need to be reimplemented
    virtual void slotContextMenuRequested(KPlato::Document *doc, const QPoint& pos);

    /// Calls  saveContext(), connect to this to have configuration saved
    virtual void sectionsMoved();

protected:
    Part *m_part;

};

class PLANWORK_EXPORT TaskWorkPackageView : public AbstractView
{
    Q_OBJECT
public:
    TaskWorkPackageView( Part *part, QWidget *parent );

    void setupGui();

    TaskWorkPackageModel *itemModel() const { return m_view->itemModel(); }

    void updateReadWrite( bool readwrite );
    Node *currentNode() const;
    Document *currentDocument() const;
    QList<Node*> selectedNodes() const;

    /// Loads context info into this view. Reimplement.
    virtual bool loadContext();
    /// Save context info from this view. Reimplement.
    virtual void saveContext();

    using AbstractView::slotContextMenuRequested;

protected Q_SLOTS:
    void slotOptions();
    void slotSplitView();

    void slotContextMenuRequested( const QModelIndex &index, const QPoint& pos );
    void slotSelectionChanged( const QModelIndexList &lst );

protected:
    void updateActionsEnabled( bool on );

private:
    TaskWorkPackageTreeView *m_view;

};

//-------------
class GanttItemDelegate : public KPlato::GanttItemDelegate
{
    Q_OBJECT
public:
    enum Brushes { Brush_Normal, Brush_Late, Brush_NotScheduled, Brush_Finished, Brush_NotReadyToStart, Brush_ReadyToStart };

    explicit GanttItemDelegate(QObject *parent = 0);

    void paintGanttItem( QPainter* painter, const KGantt::StyleOptionGanttItem& opt, const QModelIndex& idx );
    QString toolTip( const QModelIndex &idx ) const;

protected:
    bool showStatus;
    QMap<int, QBrush> m_brushes;
};

class GanttView : public KPlato::GanttViewBase
{
    Q_OBJECT
public:
    GanttView( Part *part, QWidget *parent );
    ~GanttView();

    TaskWorkPackageModel *itemModel() const;
    void setProject( Project *project );
    Project *project() const { return m_project; }

    GanttItemDelegate *delegate() const { return m_ganttdelegate; }

    QList<Node*> selectedNodes() const;
    Node *currentNode() const;

    /// Loads context info into this view. Reimplement.
    virtual bool loadContext( const KoXmlElement &context );
    /// Save context info from this view. Reimplement.
    virtual void saveContext( QDomElement &context ) const;

Q_SIGNALS:
    void headerContextMenuRequested( const QPoint& );
    void contextMenuRequested( const QModelIndex&, const QPoint& );
    void selectionChanged( const QModelIndexList& );
    void sectionsMoved();

protected Q_SLOTS:
    void slotSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    void slotRowsInserted( const QModelIndex &parent, int start, int end );
    void slotRowsRemoved( const QModelIndex &parent, int start, int end );

    void updateDateTimeGrid(KPlatoWork::WorkPackage *wp);

protected:
    Part *m_part;
    Project *m_project;
    GanttItemDelegate *m_ganttdelegate;
    TaskWorkPackageModel *m_itemmodel;
    KGantt::TreeViewRowController *m_rowController;
};

class PLANWORK_EXPORT TaskWPGanttView : public AbstractView
{
    Q_OBJECT
public:
    TaskWPGanttView( Part *part, QWidget *parent );

    void setupGui();

    TaskWorkPackageModel *itemModel() const { return m_view->itemModel(); }

    Node *currentNode() const;
    QList<Node*> selectedNodes() const;

    /// Loads context info into this view. Reimplement.
    virtual bool loadContext();
    /// Save context info from this view. Reimplement.
    virtual void saveContext();

    using AbstractView::slotContextMenuRequested;

protected Q_SLOTS:
    void slotContextMenuRequested( const QModelIndex &index, const QPoint& pos );
    void slotSelectionChanged( const QModelIndexList &lst );
    void slotOptions();

private:
    GanttView *m_view;

};

} //namespace KPlatoWork


#endif
