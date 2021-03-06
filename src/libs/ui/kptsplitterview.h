/* This file is part of the KDE project
  Copyright (C) 2007 Dag Andersen <danders@get2net.dk>

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
  Boston, MA 02110-1301, USA.
*/

#ifndef KPTSPLITTERVIEW_H
#define KPTSPLITTERVIEW_H

#include "planui_export.h"

#include "kptviewbase.h"

#include <KoXmlReaderForward.h>


class KoDocument;

class QSplitter;
class QTabWidget;

/// The main namespace
namespace KPlato
{

class Project;
class Node;
class Resource;
class ResourceGroup;
class Calendar;
class Relation;
class Context;

/**
 * SplitterView is a view with a vertical QSplitter that can contain
 * other ViewBase based views and/or QTabWidgets that can hold
 * ViewVase based views as well.
 * This splitter view is created by the main View, and subviews can then be
 * added with addView(). A QTabWidget is added with addTabWidget().
 *
 * To have context info loaded, views added to this splitter must have a
 * unique objectName().
*/
class PLANUI_EXPORT SplitterView : public ViewBase
{
    Q_OBJECT
public:
    /// Constructor
    SplitterView(KoPart *part, KoDocument *doc, QWidget *parent);
    /// Destructor
    virtual ~SplitterView() {}

    /// Set the project this view shall handle.
    virtual void setProject( Project *project );
    /// Draw data from current part / project
    virtual void draw();
    /// Draw data from project.
    virtual void draw( Project &project );
    /// Set readWrite mode
    virtual void updateReadWrite( bool );

    /// Return the view that has focus
    ViewBase *focusView() const;
    /// Returns the list of action lists that shall be plugged/unplugged
    virtual QStringList actionListNames() const;
    /// Returns the list of actions associated with the action list name
    virtual QList<QAction*> actionList( const QString &name ) const;
    /// Returns the list of context menu actions for the active view
    virtual QList<QAction*> contextActionList() const;

    /// Sets context info to this view. Reimplement.
    virtual bool setContext( const Context &/*context*/ ) { return false; }
    /// Gets context info from this view. Reimplement.
    virtual void getContext( Context &/*context*/ ) const {}
    
    /// Add a QTabWidget to the splitter
    QTabWidget *addTabWidget();
    /// Add the @p view to the splitter
    void addView( ViewBase *view );
    /// Add the @p view to the @p tab. Set the tabs label to @p label
    void addView( ViewBase *view, QTabWidget *tab, const QString &label );
    /// Return the active view at @p pos
    ViewBase *findView( const QPoint &pos ) const;

    /// Loads context info into this view. Reimplement.
    virtual bool loadContext( const KoXmlElement &/*context*/ );
    /// Save context info from this view. Reimplement.
    virtual void saveContext( QDomElement &/*context*/ ) const;
    
    virtual Node* currentNode() const;
    
    virtual Resource* currentResource() const;
    
    virtual ResourceGroup* currentResourceGroup() const;

    virtual Calendar* currentCalendar() const;

    virtual Relation *currentRelation() const;

public Q_SLOTS:
    /// Activate/deactivate the gui (also of subviews)
    virtual void setGuiActive( bool activate );
    virtual void setScheduleManager(KPlato::ScheduleManager *sm);
    virtual void slotEditCopy();

protected Q_SLOTS:
    virtual void slotGuiActivated(KPlato::ViewBase *v, bool active);
    virtual void currentTabChanged( int i );
    
protected:
    QSplitter *m_splitter;
    ViewBase *m_activeview;
};


} // namespace KPlato

#endif
