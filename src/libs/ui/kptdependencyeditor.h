/* This file is part of the KDE project
  Copyright (C) 2006 -2007 Dag Andersen <danders@get2net.dk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation;
  version 2 of the License, or (at your option) any later version..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
*/

#ifndef KPTDEPENDENCYEDITOR_H
#define KPTDEPENDENCYEDITOR_H

#include "planui_export.h"

#include "kptglobal.h"
#include "kptitemmodelbase.h"
#include "kpttaskeditor.h"
#include "kptviewbase.h"
#include "kptnode.h"
#include "kptganttitemdelegate.h"

#include <KGanttGlobal>

#include <QGraphicsView>
#include <QGraphicsItem>
#include <QTimer>

#include <kpagedialog.h>

class KoPageLayoutWidget;

class QModelIndex;


namespace KPlato
{

class Project;
//class Node;
class Relation;
class ScheduleManager;

class DependencyNodeSymbolItem;
class DependencyConnectorItem;
class DependencyNodeItem;
class DependencyScene;
class DependencyView;

class PLANUI_EXPORT DependecyViewPrintingDialog : public PrintingDialog
{
    Q_OBJECT
public:
    DependecyViewPrintingDialog( ViewBase *parent, DependencyView *view );
    int documentLastPage() const;
    void printPage( int page, QPainter &painter );
    
    QList<QWidget*> createOptionWidgets() const;

private:
    DependencyView *m_depview;
};

class PLANUI_EXPORT DependencyLinkItemBase : public QGraphicsPathItem
{
public:
    explicit DependencyLinkItemBase ( QGraphicsItem * parent = 0 );
    DependencyLinkItemBase ( DependencyNodeItem *predecessor, DependencyNodeItem *successor, Relation *rel, QGraphicsItem * parent = 0 );
    ~DependencyLinkItemBase();
    
    enum { Type = QGraphicsItem::UserType + 10 };
    int type() const { return Type; }
    
    DependencyScene *itemScene() const;
    virtual void createPath() {}
    virtual void createPath( const QPointF &sp, int stype, const QPointF &dp, int dtype );
    virtual QPointF startPoint() const { return QPointF(); }
    virtual QPointF endPoint() const { return QPointF(); }

    bool isEditable() const { return m_editable; }
    void setEditable( bool on ) { m_editable = on; }

public:
    bool m_editable;
    DependencyNodeItem *predItem;
    DependencyNodeItem *succItem;
    Relation *relation;
    QGraphicsPathItem *m_arrow;
};

class PLANUI_EXPORT DependencyLinkItem : public DependencyLinkItemBase
{
public:
    explicit DependencyLinkItem ( DependencyNodeItem *predecessor, DependencyNodeItem *successor, Relation *rel, QGraphicsItem * parent = 0 );
    ~DependencyLinkItem();
    
    enum { Type = QGraphicsItem::UserType + 11 };
    int type() const { return Type; }
    
    
    int newChildColumn() const;
    
    using DependencyLinkItemBase::createPath;
    virtual void createPath();
    virtual QPointF startPoint() const;
    virtual QPointF endPoint() const;
    
    void setItemVisible( bool show );

    void resetHooverIndication();
    
protected:
    virtual void hoverEnterEvent( QGraphicsSceneHoverEvent *event );
    virtual void hoverLeaveEvent( QGraphicsSceneHoverEvent *event );
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    
private:
    QPen m_pen;
};

class PLANUI_EXPORT DependencyCreatorItem : public DependencyLinkItemBase
{
public:
    explicit DependencyCreatorItem ( QGraphicsItem * parent = 0 );
    ~DependencyCreatorItem() {}
    
    enum { Type = QGraphicsItem::UserType + 12 };
    int type() const { return Type; }
    
    void clear();
    
    using DependencyLinkItemBase::createPath;
    virtual void createPath();
    void createPath( const QPointF &ep );

    virtual QPointF startPoint() const;
    virtual QPointF endPoint() const;
    
    void setItemVisible( bool show );

    void setPredConnector( DependencyConnectorItem *item );
    void setSuccConnector( DependencyConnectorItem *item );

    bool isEditable() const { return m_editable; }
    void setEditable( bool on ) { m_editable = on; }

public:
    DependencyConnectorItem *predConnector;
    DependencyConnectorItem *succConnector;
    bool m_editable;
};

//-----------------------
class PLANUI_EXPORT DependencyNodeItem : public QGraphicsRectItem
{
public:
    explicit DependencyNodeItem( Node *node, DependencyNodeItem *parent = 0 );
    ~DependencyNodeItem();
    
    enum  { Type = QGraphicsItem::UserType + 1 };
    int type() const { return Type; }
    
    DependencyNodeItem *parentItem() const { return m_parent; }
    void setParentItem( DependencyNodeItem *parent );
    
    void addChild( DependencyNodeItem *ch ) { m_children.append( ch ); }
    DependencyNodeItem *takeChild( DependencyNodeItem *ch );
    QList<DependencyNodeItem*> children() const { return m_children; }
    
    DependencyScene *itemScene() const;
    
    void setRectangle( const QRectF &rect );
    
    void setRow( int row );
    int row() const;
    void setColumn();
    void setColumn( int column );
    int column() const;
    
    Node *node() const { return m_node; }
    QString text() const { return m_text->toPlainText(); }
    void setText( const QString &text ) const { m_text->setPlainText( text ); }
    void setText();
    
    enum ConnectorType { Start, Finish };
    QPointF connectorPoint( ConnectorType type ) const;
    void setConnectorHoverMode( bool mode );
    
    void addParentRelation( DependencyLinkItem *r ) { m_parentrelations.append( r ); }
    DependencyLinkItem *takeParentRelation( DependencyLinkItem *r );
    QList<DependencyLinkItem*> parentRelations() const { return m_parentrelations; }

    void addChildRelation( DependencyLinkItem *r ) { m_childrelations.append( r ); }
    DependencyLinkItem *takeChildRelation( DependencyLinkItem *r );
    QList<DependencyLinkItem*> childRelations() const { return m_childrelations; }

    void setExpanded( bool mode );
    void setItemVisible( bool show );
    
    void setSymbol();
    
    int nodeLevel() const { return m_node == 0 ? 0 : m_node->level() - 1; }
    
    DependencyConnectorItem *startConnector() const { return m_start; }
    DependencyConnectorItem *finishConnector() const { return m_finish; }
    
    DependencyConnectorItem *connectorItem( ConnectorType ctype ) const;
    QList<DependencyLinkItem*> predecessorItems( ConnectorType ctype ) const;
    QList<DependencyLinkItem*> successorItems( ConnectorType ctype ) const;

    bool isEditable() const { return m_editable; }
    void setEditable( bool on ) { m_editable = on; }

    qreal treeIndicatorX() const;
    void setTreeIndicator( bool on );

protected:
    void moveToY( qreal y );
    void moveToX( qreal x );
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget );
    void paintTreeIndicator( bool on );

private:
    Node *m_node;
    DependencyConnectorItem *m_start;
    DependencyConnectorItem *m_finish;
    QGraphicsTextItem *m_text;
    DependencyNodeSymbolItem *m_symbol;
    QFont m_textFont;

    DependencyNodeItem *m_parent;
    QList<DependencyNodeItem*> m_children;
    
    QList<DependencyNodeItem*> m_dependParents;
    QList<DependencyNodeItem*> m_dependChildren;
    
    QList<DependencyLinkItem*> m_parentrelations;
    QList<DependencyLinkItem*> m_childrelations;

    bool m_editable;

    QGraphicsPathItem *m_treeIndicator;
};

//-----------------------
class PLANUI_EXPORT DependencyNodeSymbolItem : public QGraphicsPathItem
{
public:
    explicit DependencyNodeSymbolItem( DependencyNodeItem *parent = 0 )
        : QGraphicsPathItem( parent ),
        m_editable( false )
    {}
    enum  { Type = QGraphicsItem::UserType + 3 };
    int type() const { return Type; }
    
    void setSymbol( int type, const QRectF &rect );
    bool isEditable() const { return m_editable; }
    void setEditable( bool on ) { m_editable = on; }

    using QGraphicsPathItem::paint;
    /// Special paint method as the default cannot be used
    void paint( Project *p, QPainter *painter, const QStyleOptionGraphicsItem *option );

private:
    GanttItemDelegate m_delegate;
    bool m_editable;
    int m_nodetype;
    KGantt::ItemType m_itemtype;
};

//-----------------------
class PLANUI_EXPORT DependencyConnectorItem : public QGraphicsRectItem
{
public:
    DependencyConnectorItem( DependencyNodeItem::ConnectorType type, DependencyNodeItem *parent );

    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const { return Type; }
    
    DependencyNodeItem::ConnectorType ctype() const { return m_ctype; }
    
    DependencyScene *itemScene() const;
    Node* node() const;
    QPointF connectorPoint() const;
    DependencyNodeItem *nodeItem() const;
    
    QList<DependencyLinkItem*> predecessorItems() const;
    QList<DependencyLinkItem*> successorItems() const;

    bool isEditable() const { return m_editable; }
    void setEditable( bool on ) { m_editable = on; }

protected:
    void hoverEnterEvent ( QGraphicsSceneHoverEvent *event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent *event );
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    
private:
    DependencyNodeItem::ConnectorType m_ctype;

    QPointF m_mousePressPos;

    bool m_editable;
};

//-----------------------
class PLANUI_EXPORT DependencyScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit DependencyScene(QWidget *parent = 0);
    ~DependencyScene();

    void setProject( Project *p ) { m_project = p; }
    Project *project() const { return m_project; }

    QList<QGraphicsItem*> itemList( int type ) const;
    void clearScene();
    
    qreal horizontalGap() const { return 40.0; }
    qreal verticalGap() const { return 6.0; }
    qreal itemWidth() const { return 50.0; }
    qreal itemHeight() const { return 26.0; }
    QRectF symbolRect() const { return QRectF( 1.0, 1.0, 15.0, 15.0 ); }
    qreal connectorWidth() const { return 8.0; }
    
    qreal totalItemWidth() const { return itemWidth(); }
    qreal columnWidth() const { return horizontalGap() + totalItemWidth(); }
    
    qreal gridHeight() const { return verticalGap() + itemHeight(); }
    qreal gridWidth() const { return horizontalGap() + totalItemWidth(); }
    
    
    qreal gridX( int col ) const { return col * gridWidth(); } 
    qreal itemX( int col=0 ) const { return ( horizontalGap() / 2 ) + ( gridWidth() * col ); }
    int column( qreal x ) const { return (int)( x / columnWidth() ); }
    
    qreal gridY( int row ) const { return row * gridHeight(); } 
    qreal itemY( int row=0 ) const { return ( verticalGap() / 2 ) + ( gridHeight() * row ); }
    int row( qreal y ) const { return (int)( y / gridHeight() ); }
    
    DependencyLinkItem *findItem( const Relation *rel ) const;
    DependencyLinkItem *findItem( const DependencyConnectorItem *c1, const DependencyConnectorItem *c2, bool exact = true ) const;
    DependencyNodeItem *findItem( const Node *node ) const;
    
    DependencyNodeItem *findPrevItem( Node *node )  const;
    DependencyNodeItem *itemBefore( DependencyNodeItem *parent, Node *node ) const;
    DependencyNodeItem *createItem( Node *node );
    
    void setItemVisible( DependencyNodeItem *item, bool show );
    
    void createLinks();
    void createLinks( DependencyNodeItem *item );
    void createLink( DependencyNodeItem *parent, Relation *rel );

    void connectorEntered( DependencyConnectorItem *item, bool entered );
    void setFromItem( DependencyConnectorItem *item );
    DependencyConnectorItem *fromItem() const { return m_connectionitem->predConnector; }
    bool connectionMode() const { return m_connectionitem->isVisible(); }
    
    void singleConnectorClicked( DependencyConnectorItem *item );
    void multiConnectorClicked( DependencyConnectorItem *item );
    bool connectionIsValid( DependencyConnectorItem *pred, DependencyConnectorItem *succ );
    void clearConnection();

    /// Used when a node has been moved
    void moveItem( DependencyNodeItem *item, const QList<Node*> &lst );
    QList<DependencyNodeItem*> removeChildItems( DependencyNodeItem *item );
    
    DependencyNodeItem *nodeItem( int row ) const;
    const QList<DependencyNodeItem*> &nodeItems() const { return m_allItems; }

    void setReadWrite( bool on );

Q_SIGNALS:
    void connectorClicked(KPlato::DependencyConnectorItem *item);
    void connectItems(KPlato::DependencyConnectorItem *pred, KPlato::DependencyConnectorItem *succ);
    void itemDoubleClicked( QGraphicsItem * );
    void contextMenuRequested( QGraphicsItem*, const QPoint& );
    void contextMenuRequested( QGraphicsItem* );
    void dependencyContextMenuRequested(KPlato::DependencyLinkItem*, KPlato::DependencyConnectorItem*);
    void focusItemChanged( QGraphicsItem* );

protected:
    virtual void drawBackground ( QPainter * painter, const QRectF & rect );
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent *mouseEvent );
    virtual void mousePressEvent( QGraphicsSceneMouseEvent *mouseEvent );
    virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent *mouseEvent );
    virtual void keyPressEvent ( QKeyEvent *keyEvent );
    virtual void contextMenuEvent ( QGraphicsSceneContextMenuEvent *contextMenuEvent );
    
private:
    Project *m_project;
    NodeItemModel *m_model;
    bool m_readwrite;
    QList<DependencyNodeItem*> m_allItems;
    QMap<int, DependencyNodeItem*> m_visibleItems;
    QMap<int, DependencyNodeItem*> m_hiddenItems;
    DependencyCreatorItem *m_connectionitem;
    
    QList<DependencyConnectorItem*> m_clickedItems;
};


//-----------------------
class PLANUI_EXPORT DependencyView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit DependencyView(QWidget *parent);

    void setProject( Project *project );
    Project *project() const { return m_project; }

    void setItemExpanded( int x, bool mode );
    DependencyNodeItem *createItem( Node *node );
    void createItems( Node *node );
    void createItems();
    void createLinks();

    DependencyLinkItem *findItem( const Relation *rel ) const;
    DependencyNodeItem *findItem( const Node *node ) const;

    DependencyScene *itemScene() const { return static_cast<DependencyScene*>( scene() ); }
    void setItemScene(DependencyScene *scene );

    void setActive( bool activate );

Q_SIGNALS:
    void selectionChanged();
    void selectionChanged( QGraphicsItem * );
    void selectionChanged( const QList<QGraphicsItem*>& );
    void makeConnection(KPlato::DependencyConnectorItem *pred, KPlato::DependencyConnectorItem *succ);
    void contextMenuRequested( QGraphicsItem*, const QPoint& );

public Q_SLOTS:
    void slotNodeAdded(KPlato::Node *node);
    void slotNodeRemoved(KPlato::Node *node);
    void slotNodeChanged(KPlato::Node *node);
    void slotNodeMoved(KPlato::Node *node);
    
protected:
    void keyPressEvent(QKeyEvent *event);    
    void mouseMoveEvent( QMouseEvent *mouseEvent );

protected Q_SLOTS:
    void slotSelectionChanged();
    void slotRelationAdded(KPlato::Relation* rel);
    void slotRelationRemoved(KPlato::Relation* rel);
    void slotRelationModified(KPlato::Relation* rel);
    void slotWbsCodeChanged();
    
    void slotSelectedItems(); // HACK due to tt bug 160653
    void slotConnectorClicked(KPlato::DependencyConnectorItem *item);
    void slotContextMenuRequested( QGraphicsItem* );
    void slotDependencyContextMenuRequested(KPlato::DependencyLinkItem *item, KPlato::DependencyConnectorItem *connector);
    void slotFocusItemChanged( QGraphicsItem* );

private Q_SLOTS:
    void slotAutoScroll();

private:
    Project *m_project;
    bool m_dirty;
    bool m_active;

    QPoint m_cursorPos;
    QTimer m_autoScrollTimer;
};

//--------------------
class DependencyeditorConfigDialog : public KPageDialog {
    Q_OBJECT
public:
    DependencyeditorConfigDialog( ViewBase *view, QWidget *parent, bool selectPrint = false );

public Q_SLOTS:
    void slotOk();

private:
    ViewBase *m_view;
    KoPageLayoutWidget *m_pagelayout;
    PrintingHeaderFooter *m_headerfooter;
};

//------------------------------
class PLANUI_EXPORT DependencyEditor : public ViewBase
{
    Q_OBJECT
public:
    DependencyEditor(KoPart *part, KoDocument *doc, QWidget *parent);
    
    void setupGui();
    Project *project() const { return m_view->project(); }
    virtual void draw( Project &project );
    virtual void draw();

    virtual Node *currentNode() const;
    QList<Node*> selectedNodes() const ;
    Node *selectedNode() const;
    
    virtual Relation *currentRelation() const;

    virtual void updateReadWrite( bool readwrite );

    virtual KoPrintJob *createPrintJob();
    
    DependencyView *view() const { return m_view; }

Q_SIGNALS:
    void openNode();
    void editNode(KPlato::Node *node);
    void addTask();
    void addMilestone();
    void addSubtask();
    void addSubMilestone();
    void deleteTaskList(const QList<KPlato::Node*>&);

    void addRelation(KPlato::Node*, KPlato::Node*, int);
    void modifyRelation(KPlato::Relation*, int);
    void editRelation(KPlato::Relation*);
    
public Q_SLOTS:
    /// Activate/deactivate the gui
    virtual void setGuiActive( bool activate );
    void slotCreateRelation(KPlato::DependencyConnectorItem *pred, KPlato::DependencyConnectorItem *succ );
    void setScheduleManager(KPlato::ScheduleManager *sm);

protected Q_SLOTS:
    virtual void slotOptions();

protected:
    void updateActionsEnabled( bool on );
    int selectedNodeCount() const;

private Q_SLOTS:
    void slotItemDoubleClicked( QGraphicsItem *item );
    void slotCurrentChanged( const QModelIndex&, const QModelIndex& );
    void slotSelectionChanged( const QList<QGraphicsItem*> &lst );
    void slotContextMenuRequested( QGraphicsItem *item, const QPoint& pos );
    
    void slotEnableActions();

    void slotAddTask();
    void slotAddSubtask();
    void slotAddMilestone();
    void slotAddSubMilestone();
    void slotDeleteTask();

private:
    void edit( const QModelIndex &index );

private:
    DependencyView *m_view;
    Node *m_currentnode;
    Relation *m_currentrelation;
    ScheduleManager *m_manager;

    KActionMenu *menuAddTask;
    KActionMenu *menuAddSubTask;
    QAction *actionAddTask;
    QAction *actionAddMilestone;
    QAction *actionAddSubtask;
    QAction *actionAddSubMilestone;
    QAction *actionDeleteTask;
};


} //namespace KPlato


#endif
