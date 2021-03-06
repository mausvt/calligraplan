/* This file is part of the KDE project
  Copyright (C) 2006 - 2010, 2012 Dag Andersen <danders@get2net.dk>

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
#include "kpttaskeditor.h"

#include "kptglobal.h"
#include "kptcommonstrings.h"
#include "kptnodeitemmodel.h"
#include "kptcommand.h"
#include "kptproject.h"
#include "kptitemviewsettup.h"
#include "kptworkpackagesenddialog.h"
#include "kptworkpackagesendpanel.h"
#include "kptdatetime.h"
#include "kptdebug.h"
#include "kptresourcemodel.h"
#include "kptresourceallocationmodel.h"
#include "ResourceAllocationView.h"
#include "kpttaskdialog.h"
#include "TasksEditController.h"
#include "Help.h"

#include <KoXmlReader.h>
#include <KoDocument.h>
#include <KoIcon.h>

#include <QItemSelectionModel>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QDragMoveEvent>
#include <QAction>
#include <QHeaderView>
#include <QMenu>

#include <kactionmenu.h>
#include <KLocalizedString>
#include <ktoggleaction.h>
#include <kactioncollection.h>


namespace KPlato
{

//--------------------
TaskEditorItemModel::TaskEditorItemModel( QObject *parent )
: NodeItemModel( parent )
{
}

Qt::ItemFlags TaskEditorItemModel::flags( const QModelIndex &index ) const
{
    if ( index.column() == NodeModel::NodeType ) {
        if ( ! m_readWrite || isColumnReadOnly( index.column() ) ) {
            return QAbstractItemModel::flags( index );
        }
        Node *n = node( index );
        bool baselined = n ? n->isBaselined() : false;
        if ( n && ! baselined && ( n->type() == Node::Type_Task || n->type() == Node::Type_Milestone ) ) {
            return QAbstractItemModel::flags( index ) | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
        }
        return QAbstractItemModel::flags( index ) | Qt::ItemIsDropEnabled;
    }
    return NodeItemModel::flags( index );
}

QVariant TaskEditorItemModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if ( orientation == Qt::Horizontal && section == NodeModel::NodeType ) {
        if ( role == Qt::ToolTipRole ) {
            return xi18nc( "@info:tooltip", "The type of task or the estimate type of the task" );
        } else if ( role == Qt::WhatsThisRole ) {
            return xi18nc( "@info:whatsthis",
                          "<p>Indicates the type of task or the estimate type of the task.</p>"
                          "The type can be set to <emphasis>Milestone</emphasis>, <emphasis>Effort</emphasis> or <emphasis>Duration</emphasis>.<nl/>"
                          "<note>If the type is <emphasis>Summary</emphasis> or <emphasis>Project</emphasis> the type is not editable.</note>");
        }
    }
    return NodeItemModel::headerData(section, orientation, role);
}

QVariant TaskEditorItemModel::data( const QModelIndex &index, int role ) const
{
    if ( role == Qt::TextAlignmentRole ) {
        return NodeItemModel::data( index, role );
    }
    Node *n = node( index );
    if ( n != 0 && index.column() == NodeModel::NodeType ) {
        return type( n, role );
    }
    return NodeItemModel::data( index, role );
}

bool TaskEditorItemModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
    Node *n = node( index );
    if ( n != 0 && role == Qt::EditRole && index.column() == NodeModel::NodeType ) {
        return setType( n, value, role );
    }
    return NodeItemModel::setData( index, value, role );
}

QVariant TaskEditorItemModel::type( const Node *node, int role ) const
{
    switch ( role ) {
        case Qt::DisplayRole: {
            if ( node->type() == Node::Type_Task ) {
                return node->estimate()->typeToString( true );
            }
            return node->typeToString( true );
        }
        case Qt::EditRole:
            return node->type();
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        case Qt::ToolTipRole: {
            if ( node->type() == Node::Type_Task ) {
                return xi18nc( "@info:tooltip", "Task with estimate type: %1", node->estimate()->typeToString( true ) );
            }
            return xi18nc( "@info:tooltip", "Task type: %1", node->typeToString( true ) );
        }
        case Qt::StatusTipRole:
        case Qt::WhatsThisRole:
            return QVariant();
        case Role::EnumListValue: {
            if ( node->type() == Node::Type_Milestone ) {
                return 0;
            }
            if ( node->type() == Node::Type_Task ) {
                return node->estimate()->type() + 1;
            }
            return -1;
        }
        case Role::EnumList: {
            QStringList lst;
            lst << Node::typeToString( Node::Type_Milestone, true );
            lst += Estimate::typeToStringList( true );
            return lst;
        }
    }
    return QVariant();
}

bool TaskEditorItemModel::setType( Node *node, const QVariant &value, int role )
{
    switch ( role ) {
        case Qt::EditRole: {
            if ( node->type() == Node::Type_Summarytask ) {
                return false;
            }
            int v = value.toInt();
            switch ( v ) {
                case 0: { // Milestone
                    NamedCommand *cmd = 0;
                    if ( node->constraint() == Node::FixedInterval ) {
                        cmd = new NodeModifyConstraintEndTimeCmd( *node, node->constraintStartTime(), kundo2_i18n( "Set type to Milestone" ) );
                    } else {
                        cmd =  new ModifyEstimateCmd( *node, node->estimate()->expectedEstimate(), 0.0, kundo2_i18n( "Set type to Milestone" ) );
                    }
                    emit executeCommand( cmd );
                    return true;
                }
                default: { // Estimate
                    --v;
                    MacroCommand *m = new MacroCommand( kundo2_i18n( "Set type to %1", Estimate::typeToString( (Estimate::Type)v, true ) ) );
                    m->addCommand( new ModifyEstimateTypeCmd( *node, node->estimate()->type(), v ) );
                    if ( node->type() == Node::Type_Milestone ) {
                        if ( node->constraint() == Node::FixedInterval ) {
                            m->addCommand( new NodeModifyConstraintEndTimeCmd( *node, node->constraintStartTime().addDays( 1 ) ) );
                        } else {
                            m->addCommand( new ModifyEstimateUnitCmd( *node, node->estimate()->unit(), Duration::Unit_d ) );
                            m->addCommand( new ModifyEstimateCmd( *node, node->estimate()->expectedEstimate(), 1.0 ) );
                        }
                    }
                    emit executeCommand( m );
                    return true;
                }
            }
            break;
        }
        default: break;
    }
    return false;
}

//--------------------
TaskEditorTreeView::TaskEditorTreeView( QWidget *parent )
    : DoubleTreeViewBase( parent )
{
    TaskEditorItemModel *m = new TaskEditorItemModel( this );
    setModel( m );
    //setSelectionBehavior( QAbstractItemView::SelectItems );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );

    createItemDelegates( m );
    setItemDelegateForColumn( NodeModel::NodeType, new EnumDelegate( this ) );

    connect( this, &DoubleTreeViewBase::dropAllowed, this, &TaskEditorTreeView::slotDropAllowed );
}

NodeItemModel *TaskEditorTreeView::baseModel() const
{
    NodeSortFilterProxyModel *pr = proxyModel();
    if ( pr ) {
        return static_cast<NodeItemModel*>( pr->sourceModel() );
    }
    return static_cast<NodeItemModel*>( model() );
}

void TaskEditorTreeView::slotDropAllowed( const QModelIndex &index, int dropIndicatorPosition, QDragMoveEvent *event )
{
    QModelIndex idx = index;
    NodeSortFilterProxyModel *pr = proxyModel();
    if ( pr ) {
        idx = pr->mapToSource( index );
    }
    event->ignore();
    if ( baseModel()->dropAllowed( idx, dropIndicatorPosition, event->mimeData() ) ) {
        event->accept();
    }
}

//--------------------
NodeTreeView::NodeTreeView( QWidget *parent )
    : DoubleTreeViewBase( parent )
{
    NodeItemModel *m = new NodeItemModel( this );
    setModel( m );
    //setSelectionBehavior( QAbstractItemView::SelectItems );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );

    createItemDelegates( m );

    connect( this, &DoubleTreeViewBase::dropAllowed, this, &NodeTreeView::slotDropAllowed );
}

NodeItemModel *NodeTreeView::baseModel() const
{
    NodeSortFilterProxyModel *pr = proxyModel();
    if ( pr ) {
        return static_cast<NodeItemModel*>( pr->sourceModel() );
    }
    return static_cast<NodeItemModel*>( model() );
}

void NodeTreeView::slotDropAllowed( const QModelIndex &index, int dropIndicatorPosition, QDragMoveEvent *event )
{
    QModelIndex idx = index;
    NodeSortFilterProxyModel *pr = proxyModel();
    if ( pr ) {
        idx = pr->mapToSource( index );
    }
    event->ignore();
    if ( baseModel()->dropAllowed( idx, dropIndicatorPosition, event->mimeData() ) ) {
        event->accept();
    }
}


//-----------------------------------
TaskEditor::TaskEditor(KoPart *part, KoDocument *doc, QWidget *parent)
    : ViewBase(part, doc, parent )
{
    debugPlan<<"----------------- Create TaskEditor ----------------------";
    QVBoxLayout * l = new QVBoxLayout( this );
    l->setMargin( 0 );
    m_view = new TaskEditorTreeView( this );
    connect(this, &ViewBase::expandAll, m_view, &DoubleTreeViewBase::slotExpand);
    connect(this, &ViewBase::collapseAll, m_view, &DoubleTreeViewBase::slotCollapse);

    l->addWidget( m_view );
    debugPlan<<m_view->actionSplitView();
    setupGui();

    m_view->setEditTriggers( m_view->editTriggers() | QAbstractItemView::EditKeyPressed );

    m_view->setDragDropMode( QAbstractItemView::DragDrop );
    m_view->setDropIndicatorShown( true );
    m_view->setDragEnabled ( true );
    m_view->setAcceptDrops( true );
    m_view->setAcceptDropsOnView( true );

    QList<int> lst1; lst1 << 1 << -1; // only display column 0 (NodeName) in left view
    QList<int> show;
    show << NodeModel::NodeResponsible
            << NodeModel::NodeAllocation
            << NodeModel::NodeType
            << NodeModel::NodeEstimateCalendar
            << NodeModel::NodeEstimate
            << NodeModel::NodeOptimisticRatio
            << NodeModel::NodePessimisticRatio
            << NodeModel::NodeRisk
            << NodeModel::NodeConstraint
            << NodeModel::NodeConstraintStart
            << NodeModel::NodeConstraintEnd
            << NodeModel::NodeRunningAccount
            << NodeModel::NodeStartupAccount
            << NodeModel::NodeStartupCost
            << NodeModel::NodeShutdownAccount
            << NodeModel::NodeShutdownCost
            << NodeModel::NodeDescription;

    QList<int> lst2;
    for ( int i = 0; i < model()->columnCount(); ++i ) {
        if ( ! show.contains( i ) ) {
            lst2 << i;
        }
    }
    for ( int i = 0; i < show.count(); ++i ) {
        int sec = m_view->slaveView()->header()->visualIndex( show[ i ] );
        //debugPlan<<"move section:"<<i<<show[i]<<sec;
        if ( i != sec ) {
            m_view->slaveView()->header()->moveSection( sec, i );
        }
    }
    m_view->hideColumns( lst1, lst2 );
    m_view->masterView()->setDefaultColumns( QList<int>() << NodeModel::NodeName );
    m_view->slaveView()->setDefaultColumns( show );

    connect( model(), SIGNAL(executeCommand(KUndo2Command*)), doc, SLOT(addCommand(KUndo2Command*)) );

    connect( m_view, &DoubleTreeViewBase::currentChanged, this, &TaskEditor::slotCurrentChanged );

    connect( m_view, &DoubleTreeViewBase::selectionChanged, this, &TaskEditor::slotSelectionChanged );

    connect( m_view, &DoubleTreeViewBase::contextMenuRequested, this, &TaskEditor::slotContextMenuRequested );

    connect( m_view, &DoubleTreeViewBase::headerContextMenuRequested, this, &ViewBase::slotHeaderContextMenuRequested );

    connect(baseModel(), &NodeItemModel::projectShownChanged, this, &TaskEditor::slotProjectShown);
    connect(model(), &QAbstractItemModel::rowsMoved, this, &TaskEditor::slotEnableActions);

    Help::add(this,
              xi18nc("@info:whatsthis", 
                     "<title>Task Editor</title>"
                     "<para>"
                     "The Task Editor is used to create, edit, and delete tasks. "
                     "Tasks are organized into a Work Breakdown Structure (WBS) to any depth."
                     "</para><para>"
                     "This view supports configuration and printing using the context menu."
                     "<nl/><link url='%1'>More...</link>"
                     "</para>", Help::page("Manual/Task_Editor")));
}

void TaskEditor::slotProjectShown( bool on )
{
    debugPlan<<proxyModel();
    QModelIndex idx;
    if ( proxyModel() ) {
        if ( proxyModel()->rowCount() > 0 ) {
            idx = proxyModel()->index( 0, 0 );
            m_view->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
        }
    } else if ( baseModel() && baseModel()->rowCount() > 0 ) {
        idx = baseModel()->index( 0, 0 );
        m_view->selectionModel()->setCurrentIndex( idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
    }
    if ( on && idx.isValid() ) {
        m_view->masterView()->expand( idx );
    }
    slotEnableActions();
}

void TaskEditor::updateReadWrite( bool rw )
{
    m_view->setReadWrite( rw );
    ViewBase::updateReadWrite( rw );
}

void TaskEditor::setProject( Project *project )
{
    debugPlan<<project;
    m_view->setProject( project );
    ViewBase::setProject( project );
}

void TaskEditor::createDockers()
{
    // Add dockers
    DockWidget *ds = 0;
    {
        ds = new DockWidget( this, "Allocations", xi18nc( "@title resource allocations", "Allocations" ) );
        QTreeView *x = new QTreeView( ds );
        AllocatedResourceItemModel *m1 = new AllocatedResourceItemModel( x );
        x->setModel( m1 );
        m1->setProject( project() );
    //     x->setHeaderHidden( true );
        x->setSelectionBehavior( QAbstractItemView::SelectRows );
        x->setSelectionMode( QAbstractItemView::ExtendedSelection );
        x->expandAll();
        x->resizeColumnToContents( 0 );
        x->setDragDropMode( QAbstractItemView::DragOnly );
        x->setDragEnabled ( true );
        ds->setWidget( x );
        connect(this, &ViewBase::projectChanged, m1, &AllocatedResourceItemModel::setProject);
        connect(this, &TaskEditor::taskSelected, m1, &AllocatedResourceItemModel::setTask);
        connect(m1, &AllocatedResourceItemModel::expandAll, x, &QTreeView::expandAll);
        connect(m1, &AllocatedResourceItemModel::resizeColumnToContents, x, &QTreeView::resizeColumnToContents);
        addDocker( ds );
    }

    {
        ds = new DockWidget( this, "Resources", xi18nc( "@title", "Resources" ) );
        ds->setToolTip( xi18nc( "@info:tooltip",
                            "Drag resources into the Task Editor"
                            " and drop into the allocations- or responsible column" ) );
        ResourceAllocationView *e = new ResourceAllocationView(part(), ds );
        ResourceItemModel *m = new ResourceItemModel( e );
        e->setModel( m );
        m->setProject( project() );
        m->setReadWrite( isReadWrite() );
        QList<int> show; show << ResourceModel::ResourceName;
        for ( int i = m->columnCount() - 1; i >= 0; --i ) {
            e->setColumnHidden( i, ! show.contains( i ) );
        }
        e->setHeaderHidden( true );
        e->setSelectionBehavior( QAbstractItemView::SelectRows );
        e->setSelectionMode( QAbstractItemView::ExtendedSelection );
        e->expandAll();
        e->resizeColumnToContents( ResourceModel::ResourceName );
        e->setDragDropMode( QAbstractItemView::DragOnly );
        e->setDragEnabled ( true );
        ds->setWidget( e );
        connect(m_view->selectionModel(), &QItemSelectionModel::selectionChanged, e, &ResourceAllocationView::setSelectedTasks);
        connect(this, SIGNAL(projectChanged(KPlato::Project*)), m, SLOT(setProject(KPlato::Project*)));
        connect(this, &ViewBase::readWriteChanged, m, &ItemModelBase::setReadWrite);
        connect(m, &ItemModelBase::executeCommand, part(), &KoDocument::addCommand);
        addDocker( ds );
    }

    {
        ds = new DockWidget( this, "Taskmodules", xi18nc( "@title", "Task Modules" ) );
        ds->setToolTip( xi18nc( "@info:tooltip", "Drag a task module into the <emphasis>Task Editor</emphasis> to add it to the project" ) );
        ds->setLocation( Qt::LeftDockWidgetArea );
        ds->setShown(false); // hide by default
        QTreeView *e = new QTreeView( ds );
        QSortFilterProxyModel *sf = new QSortFilterProxyModel(e);
        TaskModuleModel *m = new TaskModuleModel(sf);
        sf->setSourceModel(m);
        e->setModel(sf);
        e->sortByColumn(0, Qt::AscendingOrder);
        e->setSortingEnabled(true);
        e->setHeaderHidden( true );
        e->setRootIsDecorated( false );
        e->setSelectionBehavior( QAbstractItemView::SelectRows );
        e->setSelectionMode( QAbstractItemView::SingleSelection );
//         e->resizeColumnToContents( 0 );
        e->setDragDropMode( QAbstractItemView::DragDrop );
        e->setAcceptDrops( true );
        e->setDragEnabled ( true );
        ds->setWidget( e );
        connect(e, &QAbstractItemView::doubleClicked, this, &TaskEditor::taskModuleDoubleClicked);
        connect(this, &TaskEditor::loadTaskModules, m, &TaskModuleModel::loadTaskModules);
        connect(m, &TaskModuleModel::saveTaskModule, this, &TaskEditor::saveTaskModule);
        connect(m, &TaskModuleModel::removeTaskModule, this, &TaskEditor::removeTaskModule);
        addDocker( ds );
    }
}

void TaskEditor::taskModuleDoubleClicked(QModelIndex idx)
{
    QUrl url = idx.data(Qt::UserRole).toUrl();
    if (url.isValid()) {
        emit openDocument(url);
    }
}

void TaskEditor::setTaskModules(const QStringList& files)
{
    emit loadTaskModules( files );
}

void TaskEditor::setGuiActive( bool activate )
{
    debugPlan<<activate;
    updateActionsEnabled( true );
    ViewBase::setGuiActive( activate );
    if ( activate && !m_view->selectionModel()->currentIndex().isValid() && m_view->model()->rowCount() > 0 ) {
        m_view->selectionModel()->setCurrentIndex(m_view->model()->index( 0, 0 ), QItemSelectionModel::NoUpdate);
    }
}

void TaskEditor::slotCurrentChanged( const QModelIndex &curr, const QModelIndex & )
{
    debugPlan<<curr.row()<<","<<curr.column();
    slotEnableActions();
}

void TaskEditor::slotSelectionChanged( const QModelIndexList &list)
{
    debugPlan<<list.count();
    slotEnableActions();
    emit taskSelected( dynamic_cast<Task*>( selectedNode() ) );
}

QModelIndexList TaskEditor::selectedRows() const
{
#if 0
// Qt bug?
    return m_view->selectionModel()->selectedRows();
#else
    QModelIndexList lst;
    foreach ( QModelIndex i, m_view->selectionModel()->selectedIndexes() ) {
        if ( i.column() == 0 ) {
            lst << i;
        }
    }
    return lst;
#endif
}

int TaskEditor::selectedRowCount() const
{
    return selectedRows().count();
}

QList<Node*> TaskEditor::selectedNodes() const {
    QList<Node*> lst;
    foreach ( const QModelIndex &i, selectedRows() ) {
        Node * n = m_view->baseModel()->node( i );
        if ( n != 0 && n->type() != Node::Type_Project ) {
            lst.append( n );
        }
    }
    return lst;
}

Node *TaskEditor::selectedNode() const
{
    QList<Node*> lst = selectedNodes();
    if ( lst.count() != 1 ) {
        return 0;
    }
    return lst.first();
}

Node *TaskEditor::currentNode() const {
    Node * n = m_view->baseModel()->node( m_view->selectionModel()->currentIndex() );
    if ( n == 0 || n->type() == Node::Type_Project ) {
        return 0;
    }
    return n;
}

void TaskEditor::slotContextMenuRequested( const QModelIndex& index, const QPoint& pos, const QModelIndexList &rows )
{
    QString name;
    if (rows.count() > 1) {
        debugPlan<<rows;
        QList<Task*> summarytasks;
        QList<Task*> tasks;
        QList<Task*> milestones;
        for (const QModelIndex &idx : rows) {
            Node *node = m_view->baseModel()->node( idx );
            if (node) {
                switch ( node->type() ) {
                    case Node::Type_Task:
                        tasks << static_cast<Task*>(node); break;
                    case Node::Type_Milestone:
                        milestones << static_cast<Task*>(node); break;
                    case Node::Type_Summarytask:
                        summarytasks << static_cast<Task*>(node); break;
                    default: break;
                }
            }
        }
        if (!tasks.isEmpty()) {
            editTasks(tasks, pos);
            return;
        }
        return;
    }
    Node *node = m_view->baseModel()->node( index );
    if ( node == 0 ) {
        return;
    }
    debugPlan<<node->name()<<" :"<<pos;
    switch ( node->type() ) {
    case Node::Type_Project:
        name = "task_edit_popup";
        break;
    case Node::Type_Task:
        name = node->isScheduled( baseModel()->id() ) ? "task_popup" : "task_edit_popup";
        break;
    case Node::Type_Milestone:
        name = node->isScheduled( baseModel()->id() ) ? "taskeditor_milestone_popup" : "task_edit_popup";
        break;
    case Node::Type_Summarytask:
        name = "summarytask_popup";
        break;
    default:
        name = "node_popup";
        break;
    }
    m_view->setContextMenuIndex(index);
    if ( name.isEmpty() ) {
        slotHeaderContextMenuRequested( pos );
        m_view->setContextMenuIndex(QModelIndex());
        return;
    }
    debugPlan<<name;
    emit requestPopupMenu( name, pos );
    m_view->setContextMenuIndex(QModelIndex());
}

void TaskEditor::editTasks(const QList<Task*> &tasks, const QPoint &pos)
{
    QList<QAction*> lst;
    QAction tasksEdit(i18n( "Edit..."), nullptr);
    if (!tasks.isEmpty()) {
        TasksEditController *ted = new TasksEditController(*project(), tasks, this);
        connect(&tasksEdit, &QAction::triggered, ted, &TasksEditController::activate);
        connect(ted, &TasksEditController::addCommand, koDocument(), &KoDocument::addCommand);
        lst << &tasksEdit;
    }
    lst += contextActionList();
    if (!lst.isEmpty()) {
        QMenu::exec( lst, pos, lst.first() );
    }

}

void TaskEditor::setScheduleManager( ScheduleManager *sm )
{
    if (!sm && scheduleManager()) {
        // we should only get here if the only schedule manager is scheduled,
        // or when last schedule manager is deleted
        m_domdoc.clear();
        QDomElement element = m_domdoc.createElement("expanded");
        m_domdoc.appendChild(element);
        m_view->masterView()->saveExpanded(element);
    }
    bool tryexpand = sm && !scheduleManager();
    QDomDocument doc;
    bool expand = sm && scheduleManager();
    if (expand) {
        m_view->masterView()->setObjectName("TaskEditor");
        QDomElement element = doc.createElement("expanded");
        doc.appendChild(element);
        m_view->masterView()->saveExpanded(element);
    }
    ViewBase::setScheduleManager(sm);
    m_view->baseModel()->setScheduleManager( sm );

    if (expand) {
        m_view->masterView()->doExpand(doc);
    } else if (tryexpand) {
        m_view->masterView()->doExpand(m_domdoc);
    }
}

void TaskEditor::slotEnableActions()
{
    updateActionsEnabled( isReadWrite() );
}

void TaskEditor::updateActionsEnabled( bool on )
{
//     debugPlan<<selectedRowCount()<<selectedNode()<<currentNode();
    if ( ! on ) {
        menuAddTask->setEnabled( false );
        actionAddTask->setEnabled( false );
        actionAddMilestone->setEnabled( false );
        menuAddSubTask->setEnabled( false );
        actionAddSubtask->setEnabled( false );
        actionAddSubMilestone->setEnabled( false );
        actionDeleteTask->setEnabled( false );
        actionMoveTaskUp->setEnabled( false );
        actionMoveTaskDown->setEnabled( false );
        actionIndentTask->setEnabled( false );
        actionUnindentTask->setEnabled( false );
        return;
    }
        
    int selCount = selectedRowCount();
    if ( selCount == 0 ) {
        if ( currentNode() ) {
            // there are tasks but none is selected
            menuAddTask->setEnabled( false );
            actionAddTask->setEnabled( false );
            actionAddMilestone->setEnabled( false );
            menuAddSubTask->setEnabled( false );
            actionAddSubtask->setEnabled( false );
            actionAddSubMilestone->setEnabled( false );
            actionDeleteTask->setEnabled( false );
            actionMoveTaskUp->setEnabled( false );
            actionMoveTaskDown->setEnabled( false );
            actionIndentTask->setEnabled( false );
            actionUnindentTask->setEnabled( false );
        } else {
            // we need to be able to add the first task
            menuAddTask->setEnabled( true );
            actionAddTask->setEnabled( true );
            actionAddMilestone->setEnabled( true );
            menuAddSubTask->setEnabled( false );
            actionAddSubtask->setEnabled( false );
            actionAddSubMilestone->setEnabled( false );
            actionDeleteTask->setEnabled( false );
            actionMoveTaskUp->setEnabled( false );
            actionMoveTaskDown->setEnabled( false );
            actionIndentTask->setEnabled( false );
            actionUnindentTask->setEnabled( false );
        }
        return;
    }
    Node *n = selectedNode(); // 0 if not a single task, summarytask or milestone
    if ( selCount == 1 && n == 0 ) {
        // only project selected
        menuAddTask->setEnabled( true );
        actionAddTask->setEnabled( true );
        actionAddMilestone->setEnabled( true );
        menuAddSubTask->setEnabled( true );
        actionAddSubtask->setEnabled( true );
        actionAddSubMilestone->setEnabled( true );
        actionDeleteTask->setEnabled( false );
        actionMoveTaskUp->setEnabled( false );
        actionMoveTaskDown->setEnabled( false );
        actionIndentTask->setEnabled( false );
        actionUnindentTask->setEnabled( false );
        return;
    }
    if ( selCount == 1 && n != currentNode() ) {
        // multi selection in progress
        menuAddTask->setEnabled( false );
        actionAddTask->setEnabled( false );
        actionAddMilestone->setEnabled( false );
        menuAddSubTask->setEnabled( false );
        actionAddSubtask->setEnabled( false );
        actionAddSubMilestone->setEnabled( false );
        actionDeleteTask->setEnabled( false );
        actionMoveTaskUp->setEnabled( false );
        actionMoveTaskDown->setEnabled( false );
        actionIndentTask->setEnabled( false );
        actionUnindentTask->setEnabled( false );
        return;
    }

    bool baselined = false;
    Project *p = m_view->project();
    if ( p && p->isBaselined() ) {
        foreach ( Node *n, selectedNodes() ) {
            if ( n->isBaselined() ) {
                baselined = true;
                break;
            }
        }
    }
    if ( selCount == 1 ) {
        menuAddTask->setEnabled( true );
        actionAddTask->setEnabled( true );
        actionAddMilestone->setEnabled( true );
        menuAddSubTask->setEnabled( ! baselined || n->type() == Node::Type_Summarytask );
        actionAddSubtask->setEnabled( ! baselined || n->type() == Node::Type_Summarytask );
        actionAddSubMilestone->setEnabled( ! baselined || n->type() == Node::Type_Summarytask );
        actionDeleteTask->setEnabled( ! baselined );
        Node *s = n->siblingBefore();
        actionMoveTaskUp->setEnabled( s );
        actionMoveTaskDown->setEnabled( n->siblingAfter() );
        s = n->siblingBefore();
        actionIndentTask->setEnabled( ! baselined && s && ! s->isBaselined() );
        actionUnindentTask->setEnabled( ! baselined && n->level() > 1 );
        return;
    }
    // selCount > 1
    menuAddTask->setEnabled( false );
    actionAddTask->setEnabled( false );
    actionAddMilestone->setEnabled( false );
    menuAddSubTask->setEnabled( false );
    actionAddSubtask->setEnabled( false );
    actionAddSubMilestone->setEnabled( false );
    actionDeleteTask->setEnabled( ! baselined );
    actionMoveTaskUp->setEnabled( false );
    actionMoveTaskDown->setEnabled( false );
    actionIndentTask->setEnabled( false );
    actionUnindentTask->setEnabled( false );
}

void TaskEditor::setupGui()
{
    QString name = "taskeditor_add_list";

    menuAddTask = new KActionMenu(koIcon("view-task-add"), i18n("Add Task"), this);
    actionCollection()->addAction("add_task", menuAddTask );
    connect( menuAddTask, &QAction::triggered, this, &TaskEditor::slotAddTask );
    addAction( name, menuAddTask );

    actionAddTask  = new QAction( i18n( "Add Task" ), this);
    actionAddTask->setShortcut( Qt::CTRL + Qt::Key_I );
    connect( actionAddTask, &QAction::triggered, this, &TaskEditor::slotAddTask );
    menuAddTask->addAction( actionAddTask );

    actionAddMilestone  = new QAction( i18n( "Add Milestone" ), this );
    actionAddMilestone->setShortcut( Qt::CTRL + Qt::ALT + Qt::Key_I );
    connect( actionAddMilestone, &QAction::triggered, this, &TaskEditor::slotAddMilestone );
    menuAddTask->addAction( actionAddMilestone );


    menuAddSubTask = new KActionMenu(koIcon("view-task-child-add"), i18n("Add Sub-Task"), this);
    actionCollection()->addAction("add_subtask", menuAddTask );
    connect( menuAddSubTask, &QAction::triggered, this, &TaskEditor::slotAddSubtask );
    addAction( name, menuAddSubTask );

    actionAddSubtask  = new QAction( i18n( "Add Sub-Task" ), this );
    actionAddSubtask->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::Key_I );
    connect( actionAddSubtask, &QAction::triggered, this, &TaskEditor::slotAddSubtask );
    menuAddSubTask->addAction( actionAddSubtask );

    actionAddSubMilestone = new QAction( i18n( "Add Sub-Milestone" ), this );
    actionAddSubMilestone->setShortcut( Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_I );
    connect( actionAddSubMilestone, &QAction::triggered, this, &TaskEditor::slotAddSubMilestone );
    menuAddSubTask->addAction( actionAddSubMilestone );

    actionDeleteTask  = new QAction(koIcon("edit-delete"), xi18nc("@action", "Delete"), this);
    actionCollection()->setDefaultShortcut( actionDeleteTask, Qt::Key_Delete );
    actionCollection()->addAction("delete_task", actionDeleteTask );
    connect( actionDeleteTask, &QAction::triggered, this, &TaskEditor::slotDeleteTask );
    addAction( name, actionDeleteTask );


    name = "taskeditor_move_list";
    actionIndentTask  = new QAction(koIcon("format-indent-more"), i18n("Indent Task"), this);
    actionCollection()->addAction("indent_task", actionIndentTask );
    connect(actionIndentTask, &QAction::triggered, this, &TaskEditor::slotIndentTask);
    addAction( name, actionIndentTask );

    actionUnindentTask  = new QAction(koIcon("format-indent-less"), i18n("Unindent Task"), this);
    actionCollection()->addAction("unindent_task", actionUnindentTask );
    connect(actionUnindentTask, &QAction::triggered, this, &TaskEditor::slotUnindentTask);
    addAction( name, actionUnindentTask );

    actionMoveTaskUp  = new QAction(koIcon("arrow-up"), i18n("Move Up"), this);
    actionCollection()->addAction("move_task_up", actionMoveTaskUp );
    connect(actionMoveTaskUp, &QAction::triggered, this, &TaskEditor::slotMoveTaskUp);
    addAction( name, actionMoveTaskUp );

    actionMoveTaskDown  = new QAction(koIcon("arrow-down"), i18n("Move Down"), this);
    actionCollection()->addAction("move_task_down", actionMoveTaskDown );
    connect(actionMoveTaskDown, &QAction::triggered, this, &TaskEditor::slotMoveTaskDown);
    addAction( name, actionMoveTaskDown );

    // Add the context menu actions for the view options
    actionShowProject = new KToggleAction( i18n( "Show Project" ), this );
    connect(actionShowProject, &QAction::triggered, baseModel(), &NodeItemModel::setShowProject);
    addContextAction( actionShowProject );

    connect(m_view->actionSplitView(), &QAction::triggered, this, &TaskEditor::slotSplitView);
    addContextAction( m_view->actionSplitView() );

    createOptionActions(ViewBase::OptionAll);

    createDockers();
}

void TaskEditor::slotSplitView()
{
    debugPlan;
    m_view->setViewSplitMode( ! m_view->isViewSplit() );
    emit optionsModified();
}


void TaskEditor::slotOptions()
{
    debugPlan;
    SplitItemViewSettupDialog *dlg = new SplitItemViewSettupDialog( this, m_view, this );
    dlg->addPrintingOptions(sender()->objectName() == "print options");
    connect(dlg, SIGNAL(finished(int)), SLOT(slotOptionsFinished(int)));
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

void TaskEditor::slotAddTask()
{
    debugPlan;
    if ( selectedRowCount() == 0 || ( selectedRowCount() == 1 && selectedNode() == 0 ) ) {
        m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
        Task *t = m_view->project()->createTask( m_view->project()->taskDefaults() );
        QModelIndex idx = m_view->baseModel()->insertSubtask( t, m_view->project() );
        Q_ASSERT( idx.isValid() );
        edit( idx );
        return;
    }
    Node *sib = selectedNode();
    if ( sib == 0 ) {
        return;
    }
    m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
    Task *t = m_view->project()->createTask( m_view->project()->taskDefaults() );
    QModelIndex idx = m_view->baseModel()->insertTask( t, sib );
    Q_ASSERT( idx.isValid() );
    edit( idx );
}

void TaskEditor::slotAddMilestone()
{
    debugPlan;
    if ( selectedRowCount() == 0  || ( selectedRowCount() == 1 && selectedNode() == 0 ) ) {
        // None selected or only project selected: insert under main project
        m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
        Task *t = m_view->project()->createTask();
        t->estimate()->clear();
        QModelIndex idx = m_view->baseModel()->insertSubtask( t, m_view->project() );
        Q_ASSERT( idx.isValid() );
        edit( idx );
        return;
    }
    Node *sib = selectedNode(); // sibling
    if ( sib == 0 ) {
        return;
    }
    m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
    Task *t = m_view->project()->createTask();
    t->estimate()->clear();
    QModelIndex idx = m_view->baseModel()->insertTask( t, sib );
    Q_ASSERT( idx.isValid() );
    edit( idx );
}

void TaskEditor::slotAddSubMilestone()
{
    debugPlan;
    Node *parent = selectedNode();
    if ( parent == 0 && selectedRowCount() == 1 ) {
        // project selected
        parent = m_view->project();
    }
    if ( parent == 0 ) {
        return;
    }
    m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
    Task *t = m_view->project()->createTask( m_view->project()->taskDefaults() );
    t->estimate()->clear();
    QModelIndex idx = m_view->baseModel()->insertSubtask( t, parent );
    Q_ASSERT( idx.isValid() );
    edit( idx );
}

void TaskEditor::slotAddSubtask()
{
    debugPlan;
    Node *parent = selectedNode();
    if ( parent == 0 && selectedRowCount() == 1 ) {
        // project selected
        parent = m_view->project();
    }
    if ( parent == 0 ) {
        return;
    }
    m_view->closePersistentEditor( m_view->selectionModel()->currentIndex() );
    Task *t = m_view->project()->createTask( m_view->project()->taskDefaults() );
    QModelIndex idx = m_view->baseModel()->insertSubtask( t, parent );
    Q_ASSERT( idx.isValid() );
    edit( idx );
}

void TaskEditor::edit( const QModelIndex &i )
{
    if ( i.isValid() ) {
        m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
        m_view->setParentsExpanded( i, true ); // in case treeview does not have focus
        m_view->edit( i );
    }
}

void TaskEditor::slotDeleteTask()
{
    //debugPlan;
    QList<Node*> lst = selectedNodes();
    while ( true ) {
        // remove children of selected tasks, as parents delete their children
        Node *ch = 0;
        foreach ( Node *n1, lst ) {
            foreach ( Node *n2, lst ) {
                if ( n2->isChildOf( n1 ) ) {
                    ch = n2;
                    break;
                }
            }
            if ( ch != 0 ) {
                break;
            }
        }
        if ( ch == 0 ) {
            break;
        }
        lst.removeAt( lst.indexOf( ch ) );
    }
    //foreach ( Node* n, lst ) { debugPlan<<n->name(); }
    emit deleteTaskList( lst );
    QModelIndex i = m_view->selectionModel()->currentIndex();
    if ( i.isValid() ) {
        m_view->selectionModel()->select( i, QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect );
        m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
    }
}

void TaskEditor::slotIndentTask()
{
    debugPlan;
    Node *n = selectedNode();
    if ( n ) {
        emit indentTask();
        QModelIndex i = baseModel()->index( n );
        m_view->selectionModel()->select( i, QItemSelectionModel::Rows | QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect );
        m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
        m_view->setParentsExpanded( i, true );
    }
}

void TaskEditor::slotUnindentTask()
{
    debugPlan;
    Node *n = selectedNode();
    if ( n ) {
        emit unindentTask();
        QModelIndex i = baseModel()->index( n );
        m_view->selectionModel()->select( i, QItemSelectionModel::Rows | QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect );
        m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
    }
}

void TaskEditor::slotMoveTaskUp()
{
    debugPlan;
    Node *n = selectedNode();
    if ( n ) {
        emit moveTaskUp();
        QModelIndex i = baseModel()->index( n );
        m_view->selectionModel()->select( i, QItemSelectionModel::Rows | QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect );
        m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
    }
}

void TaskEditor::slotMoveTaskDown()
{
    debugPlan;
    Node *n = selectedNode();
    if ( n ) {
        emit moveTaskDown();
        QModelIndex i = baseModel()->index( n );
        m_view->selectionModel()->select( i, QItemSelectionModel::Rows | QItemSelectionModel::Current | QItemSelectionModel::ClearAndSelect );
         m_view->selectionModel()->setCurrentIndex( i, QItemSelectionModel::NoUpdate );
   }
}

bool TaskEditor::loadContext( const KoXmlElement &context )
{
    ViewBase::loadContext( context );
    bool show = (bool)(context.attribute( "show-project", "0" ).toInt() );
    actionShowProject->setChecked( show );
    baseModel()->setShowProject( show ); // why is this not called by the action?
    bool res = m_view->loadContext( baseModel()->columnMap(), context );
    return res;
}

void TaskEditor::saveContext( QDomElement &context ) const
{
    ViewBase::saveContext( context );
    context.setAttribute( "show-project", QString::number(baseModel()->projectShown()) );
    m_view->saveContext( baseModel()->columnMap(), context );
}

KoPrintJob *TaskEditor::createPrintJob()
{
    return m_view->createPrintJob( this );
}


//-----------------------------------
TaskView::TaskView(KoPart *part, KoDocument *doc, QWidget *parent)
    : ViewBase(part, doc, parent)
{
    QVBoxLayout * l = new QVBoxLayout( this );
    l->setMargin( 0 );
    m_view = new NodeTreeView( this );
    connect(this, &ViewBase::expandAll, m_view, &DoubleTreeViewBase::slotExpand);
    connect(this, &ViewBase::collapseAll, m_view, &DoubleTreeViewBase::slotCollapse);

    NodeSortFilterProxyModel *p = new NodeSortFilterProxyModel( m_view->baseModel(), m_view );
    m_view->setModel( p );
    l->addWidget( m_view );
    setupGui();

    //m_view->setEditTriggers( m_view->editTriggers() | QAbstractItemView::EditKeyPressed );
    m_view->setDragDropMode( QAbstractItemView::InternalMove );
    m_view->setDropIndicatorShown( false );
    m_view->setDragEnabled ( true );
    m_view->setAcceptDrops( false );
    m_view->setAcceptDropsOnView( false );

    QList<int> readonly;
    readonly << NodeModel::NodeName
            << NodeModel::NodeResponsible
            << NodeModel::NodeAllocation
            << NodeModel::NodeEstimateType
            << NodeModel::NodeEstimateCalendar
            << NodeModel::NodeEstimate
            << NodeModel::NodeOptimisticRatio
            << NodeModel::NodePessimisticRatio
            << NodeModel::NodeRisk
            << NodeModel::NodeConstraint
            << NodeModel::NodeConstraintStart
            << NodeModel::NodeConstraintEnd
            << NodeModel::NodeRunningAccount
            << NodeModel::NodeStartupAccount
            << NodeModel::NodeStartupCost
            << NodeModel::NodeShutdownAccount
            << NodeModel::NodeShutdownCost
            << NodeModel::NodeDescription;
    foreach ( int c, readonly ) {
        m_view->baseModel()->setReadOnly( c, true );
    }

    QList<int> lst1; lst1 << 1 << -1;
    QList<int> show;
    show << NodeModel::NodeStatus
            << NodeModel::NodeCompleted
            << NodeModel::NodeResponsible
            << NodeModel::NodeAssignments
            << NodeModel::NodePerformanceIndex
            << NodeModel::NodeBCWS
            << NodeModel::NodeBCWP
            << NodeModel::NodeACWP
            << NodeModel::NodeDescription;

    for ( int s = 0; s < show.count(); ++s ) {
        m_view->slaveView()->mapToSection( show[s], s );
    }
    QList<int> lst2;
    for ( int i = 0; i < m_view->model()->columnCount(); ++i ) {
        if ( ! show.contains( i ) ) {
            lst2 << i;
        }
    }
    m_view->hideColumns( lst1, lst2 );
    m_view->masterView()->setDefaultColumns( QList<int>() << 0 );
    m_view->slaveView()->setDefaultColumns( show );

    connect( m_view->baseModel(), &ItemModelBase::executeCommand, doc, &KoDocument::addCommand );

    connect( m_view, &DoubleTreeViewBase::currentChanged, this, &TaskView::slotCurrentChanged );

    connect( m_view, &DoubleTreeViewBase::selectionChanged, this, &TaskView::slotSelectionChanged );

    connect( m_view, &DoubleTreeViewBase::contextMenuRequested, this, &TaskView::slotContextMenuRequested );

    connect( m_view, &DoubleTreeViewBase::headerContextMenuRequested, this, &ViewBase::slotHeaderContextMenuRequested );

    Help::add(this,
              xi18nc("@info:whatsthis", 
                     "<title>Task Execution View</title>"
                     "<para>"
                     "The view is used to edit and inspect task progress during project execution."
                     "</para><para>"
                     "This view supports configuration and printing using the context menu."
                     "<nl/><link url='%1'>More...</link>"
                     "</para>", Help::page("Manual/Task_Execution_View")));
}

void TaskView::updateReadWrite( bool rw )
{
    m_view->setReadWrite( rw );
    ViewBase::updateReadWrite( rw );
}

void TaskView::draw( Project &project )
{
    m_view->setProject( &project );
}

void TaskView::draw()
{
}

void TaskView::setGuiActive( bool activate )
{
    debugPlan<<activate;
    updateActionsEnabled( true );
    ViewBase::setGuiActive( activate );
    if ( activate && !m_view->selectionModel()->currentIndex().isValid() && m_view->model()->rowCount() > 0 ) {
        m_view->selectionModel()->setCurrentIndex(m_view->model()->index( 0, 0 ), QItemSelectionModel::NoUpdate);
    }
}

void TaskView::slotCurrentChanged(  const QModelIndex &curr, const QModelIndex & )
{
    debugPlan<<curr.row()<<","<<curr.column();
    slotEnableActions();
}

void TaskView::slotSelectionChanged( const QModelIndexList &list)
{
    debugPlan<<list.count();
    slotEnableActions();
}

int TaskView::selectedNodeCount() const
{
    QItemSelectionModel* sm = m_view->selectionModel();
    return sm->selectedRows().count();
}

QList<Node*> TaskView::selectedNodes() const {
    QList<Node*> lst;
    QItemSelectionModel* sm = m_view->selectionModel();
    if ( sm == 0 ) {
        return lst;
    }
    foreach ( const QModelIndex &i, sm->selectedRows() ) {
        Node * n = m_view->baseModel()->node( proxyModel()->mapToSource( i ) );
        if ( n != 0 && n->type() != Node::Type_Project ) {
            lst.append( n );
        }
    }
    return lst;
}

Node *TaskView::selectedNode() const
{
    QList<Node*> lst = selectedNodes();
    if ( lst.count() != 1 ) {
        return 0;
    }
    return lst.first();
}

Node *TaskView::currentNode() const {
    Node * n = m_view->baseModel()->node( proxyModel()->mapToSource( m_view->selectionModel()->currentIndex() ) );
    if ( n == 0 || n->type() == Node::Type_Project ) {
        return 0;
    }
    return n;
}

void TaskView::slotContextMenuRequested( const QModelIndex& index, const QPoint& pos )
{
    QString name;
    Node *node = m_view->baseModel()->node( proxyModel()->mapToSource( index ) );
    if ( node ) {
        switch ( node->type() ) {
            case Node::Type_Task:
                name = "taskview_popup";
                break;
            case Node::Type_Milestone:
                name = "taskview_milestone_popup";
                break;
            case Node::Type_Summarytask:
                name = "taskview_summary_popup";
                break;
            default:
                break;
        }
    } else debugPlan<<"No node: "<<index;
    if ( name.isEmpty() ) {
        debugPlan<<"No menu";
        slotHeaderContextMenuRequested( pos );
        return;
    }
    m_view->setContextMenuIndex(index);
    emit requestPopupMenu( name, pos );
    m_view->setContextMenuIndex(QModelIndex());
}

void TaskView::setScheduleManager( ScheduleManager *sm )
{
    //debugPlan<<endl;
    if (!sm && scheduleManager()) {
        // we should only get here if the only schedule manager is scheduled,
        // or when last schedule manager is deleted
        m_domdoc.clear();
        QDomElement element = m_domdoc.createElement("expanded");
        m_domdoc.appendChild(element);
        m_view->masterView()->saveExpanded(element);
    }
    bool tryexpand = sm && !scheduleManager();
    QDomDocument doc;
    bool expand = sm && scheduleManager() && sm != scheduleManager();
    if (expand) {
        m_view->masterView()->setObjectName("TaskEditor");
        QDomElement element = doc.createElement("expanded");
        doc.appendChild(element);
        m_view->masterView()->saveExpanded(element);
    }
    ViewBase::setScheduleManager(sm);
    m_view->baseModel()->setScheduleManager( sm );

    if (expand) {
        m_view->masterView()->doExpand(doc);
    } else if (tryexpand) {
        m_view->masterView()->doExpand(m_domdoc);
    }
}

void TaskView::slotEnableActions()
{
    updateActionsEnabled( true );
}

void TaskView::updateActionsEnabled( bool /*on*/ )
{

}

void TaskView::setupGui()
{
//    KActionCollection *coll = actionCollection();

    // Add the context menu actions for the view options
    actionShowProject = new KToggleAction( i18n( "Show Project" ), this );
    connect(actionShowProject, &QAction::triggered, baseModel(), &NodeItemModel::setShowProject);
    addContextAction( actionShowProject );

    connect(m_view->actionSplitView(), &QAction::triggered, this, &TaskView::slotSplitView);
    addContextAction( m_view->actionSplitView() );

    createOptionActions(ViewBase::OptionAll);
}

void TaskView::slotSplitView()
{
    debugPlan;
    m_view->setViewSplitMode( ! m_view->isViewSplit() );
    emit optionsModified();
}

void TaskView::slotOptions()
{
    debugPlan;
    SplitItemViewSettupDialog *dlg = new SplitItemViewSettupDialog( this, m_view, this );
    dlg->addPrintingOptions(sender()->objectName() == "print options");
    connect(dlg, SIGNAL(finished(int)), SLOT(slotOptionsFinished(int)));
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

bool TaskView::loadContext( const KoXmlElement &context )
{
    ViewBase::loadContext( context );
    bool show = (bool)(context.attribute( "show-project", "0" ).toInt() );
    actionShowProject->setChecked( show );
    baseModel()->setShowProject( show ); // why is this not called by the action?
    return m_view->loadContext( m_view->baseModel()->columnMap(), context );
}

void TaskView::saveContext( QDomElement &context ) const
{
    ViewBase::saveContext( context );
    context.setAttribute( "show-project", QString::number(baseModel()->projectShown()) );
    m_view->saveContext( m_view->baseModel()->columnMap(), context );
}

KoPrintJob *TaskView::createPrintJob()
{
    return m_view->createPrintJob( this );
}

//---------------------------------
WorkPackageTreeView::WorkPackageTreeView( QWidget *parent )
    : DoubleTreeViewBase( parent )
{
    debugPlan<<"----------"<<this<<"----------";
    m = new WorkPackageProxyModel( this );
    setModel( m );
    //setSelectionBehavior( QAbstractItemView::SelectItems );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setSelectionBehavior( QAbstractItemView::SelectRows );

    createItemDelegates( baseModel() );

    setSortingEnabled( true );
    sortByColumn( NodeModel::NodeWBSCode, Qt::AscendingOrder );

    connect( this, &DoubleTreeViewBase::dropAllowed, this, &WorkPackageTreeView::slotDropAllowed );
}

NodeItemModel *WorkPackageTreeView::baseModel() const
{
    return m->baseModel();
}

void WorkPackageTreeView::slotDropAllowed( const QModelIndex &index, int dropIndicatorPosition, QDragMoveEvent *event )
{
    Q_UNUSED(index);
    Q_UNUSED(dropIndicatorPosition);
    Q_UNUSED(event);
/*    QModelIndex idx = index;
    NodeSortFilterProxyModel *pr = proxyModel();
    if ( pr ) {
        idx = pr->mapToSource( index );
    }
    event->ignore();
    if ( baseModel()->dropAllowed( idx, dropIndicatorPosition, event->mimeData() ) ) {
        event->accept();
    }*/
}

//--------------------------------
TaskWorkPackageView::TaskWorkPackageView(KoPart *part, KoDocument *doc, QWidget *parent)
    : ViewBase(part, doc, parent ),
    m_cmd( 0 )
{
    QVBoxLayout * l = new QVBoxLayout( this );
    l->setMargin( 0 );
    m_view = new WorkPackageTreeView( this );
    connect(this, &ViewBase::expandAll, m_view, &DoubleTreeViewBase::slotExpand);
    connect(this, &ViewBase::collapseAll, m_view, &DoubleTreeViewBase::slotCollapse);

    l->addWidget( m_view );
    setupGui();

    //m_view->setEditTriggers( m_view->editTriggers() | QAbstractItemView::EditKeyPressed );
    m_view->setDragDropMode( QAbstractItemView::InternalMove );
    m_view->setDropIndicatorShown( false );
    m_view->setDragEnabled ( true );
    m_view->setAcceptDrops( false );
    m_view->setAcceptDropsOnView( false );

    QList<int> readonly;
    readonly << NodeModel::NodeName
            << NodeModel::NodeResponsible
            << NodeModel::NodeAllocation
            << NodeModel::NodeEstimateType
            << NodeModel::NodeEstimateCalendar
            << NodeModel::NodeEstimate
            << NodeModel::NodeOptimisticRatio
            << NodeModel::NodePessimisticRatio
            << NodeModel::NodeRisk
            << NodeModel::NodeConstraint
            << NodeModel::NodeConstraintStart
            << NodeModel::NodeConstraintEnd
            << NodeModel::NodeRunningAccount
            << NodeModel::NodeStartupAccount
            << NodeModel::NodeStartupCost
            << NodeModel::NodeShutdownAccount
            << NodeModel::NodeShutdownCost
            << NodeModel::NodeDescription;
    foreach ( int c, readonly ) {
        m_view->baseModel()->setReadOnly( c, true );
    }

    QList<int> lst1; lst1 << 1 << -1;
    QList<int> show;
    show << NodeModel::NodeStatus
            << NodeModel::NodeCompleted
            << NodeModel::NodeResponsible
            << NodeModel::NodeAssignments
            << NodeModel::NodeDescription;

    for ( int s = 0; s < show.count(); ++s ) {
        m_view->slaveView()->mapToSection( show[s], s );
    }
    QList<int> lst2;
    for ( int i = 0; i < m_view->model()->columnCount(); ++i ) {
        if ( ! show.contains( i ) ) {
            lst2 << i;
        }
    }
    m_view->hideColumns( lst1, lst2 );
    m_view->masterView()->setDefaultColumns( QList<int>() << 0 );
    m_view->slaveView()->setDefaultColumns( show );

    connect( m_view->baseModel(), &ItemModelBase::executeCommand, doc, &KoDocument::addCommand );

    connect( m_view, &DoubleTreeViewBase::currentChanged, this, &TaskWorkPackageView::slotCurrentChanged );

    connect( m_view, &DoubleTreeViewBase::selectionChanged, this, &TaskWorkPackageView::slotSelectionChanged );

    connect( m_view, &DoubleTreeViewBase::contextMenuRequested, this, &TaskWorkPackageView::slotContextMenuRequested );

    connect( m_view, &DoubleTreeViewBase::headerContextMenuRequested, this, &ViewBase::slotHeaderContextMenuRequested );
}

Project *TaskWorkPackageView::project() const
{
    return m_view->project();
}

void TaskWorkPackageView::setProject( Project *project )
{
    m_view->setProject( project );
}

WorkPackageProxyModel *TaskWorkPackageView::proxyModel() const
{
    return m_view->proxyModel();
}

void TaskWorkPackageView::updateReadWrite( bool rw )
{
    m_view->setReadWrite( rw );
    ViewBase::updateReadWrite( rw );
}

void TaskWorkPackageView::setGuiActive( bool activate )
{
    debugPlan<<activate;
    updateActionsEnabled( true );
    ViewBase::setGuiActive( activate );
    if ( activate && !m_view->selectionModel()->currentIndex().isValid() && m_view->model()->rowCount() > 0 ) {
        m_view->selectionModel()->setCurrentIndex(m_view->model()->index( 0, 0 ), QItemSelectionModel::NoUpdate);
    }
}

void TaskWorkPackageView::slotRefreshView()
{
    emit checkForWorkPackages(false);
}

void TaskWorkPackageView::slotCurrentChanged(  const QModelIndex &curr, const QModelIndex & )
{
    debugPlan<<curr.row()<<","<<curr.column();
    slotEnableActions();
}

void TaskWorkPackageView::slotSelectionChanged( const QModelIndexList &list)
{
    debugPlan<<list.count();
    slotEnableActions();
}

int TaskWorkPackageView::selectedNodeCount() const
{
    QItemSelectionModel* sm = m_view->selectionModel();
    return sm->selectedRows().count();
}

QList<Node*> TaskWorkPackageView::selectedNodes() const {
    QList<Node*> lst;
    QItemSelectionModel* sm = m_view->selectionModel();
    if ( sm == 0 ) {
        return lst;
    }
    foreach ( const QModelIndex &i, sm->selectedRows() ) {
        Node * n = proxyModel()->taskFromIndex( i );
        if ( n != 0 && n->type() != Node::Type_Project ) {
            lst.append( n );
        }
    }
    return lst;
}

Node *TaskWorkPackageView::selectedNode() const
{
    QList<Node*> lst = selectedNodes();
    if ( lst.count() != 1 ) {
        return 0;
    }
    return lst.first();
}

Node *TaskWorkPackageView::currentNode() const {
    Node * n = proxyModel()->taskFromIndex( m_view->selectionModel()->currentIndex() );
    if ( n == 0 || n->type() == Node::Type_Project ) {
        return 0;
    }
    return n;
}

void TaskWorkPackageView::slotContextMenuRequested( const QModelIndex& index, const QPoint& pos )
{
    QString name;
    Node *node = proxyModel()->taskFromIndex( index );
    if ( node ) {
        switch ( node->type() ) {
            case Node::Type_Task:
                name = "workpackage_popup";
                break;
            case Node::Type_Milestone:
                name = "taskview_milestone_popup";
                break;
            case Node::Type_Summarytask:
                name = "taskview_summary_popup";
                break;
            default:
                break;
        }
    } else debugPlan<<"No node: "<<index;
    if ( name.isEmpty() ) {
        debugPlan<<"No menu";
        slotHeaderContextMenuRequested( pos );
        return;
    }
    m_view->setContextMenuIndex(index);
    emit requestPopupMenu( name, pos );
    m_view->setContextMenuIndex(QModelIndex());
}

void TaskWorkPackageView::setScheduleManager( ScheduleManager *sm )
{
    //debugPlan<<endl;
    m_view->baseModel()->setScheduleManager( sm );
}

void TaskWorkPackageView::slotEnableActions()
{
    updateActionsEnabled( true );
}

void TaskWorkPackageView::updateActionsEnabled( bool on )
{
    bool o = ! selectedNodes().isEmpty();
    actionMailWorkpackage->setEnabled( o && on );
}

void TaskWorkPackageView::setupGui()
{
//    KActionCollection *coll = actionCollection();

    QString name = "workpackage_list";
    actionMailWorkpackage  = new QAction(koIcon("mail-send"), i18n("Send..."), this);
    actionCollection()->setDefaultShortcut( actionMailWorkpackage, Qt::CTRL + Qt::Key_M );
    actionCollection()->addAction("send_workpackage", actionMailWorkpackage );
    connect( actionMailWorkpackage, &QAction::triggered, this, &TaskWorkPackageView::slotMailWorkpackage );
    addAction( name, actionMailWorkpackage );

    // Add the context menu actions for the view options
    connect(m_view->actionSplitView(), &QAction::triggered, this, &TaskWorkPackageView::slotSplitView);
    addContextAction( m_view->actionSplitView() );

    createOptionActions(ViewBase::OptionAll);
}

void TaskWorkPackageView::slotMailWorkpackage()
{
    QList<Node*> lst = selectedNodes();
    if ( ! lst.isEmpty() ) {
        // TODO find a better way to log to avoid undo/redo
        m_cmd = new MacroCommand( kundo2_i18n( "Log Send Workpackage" ) );
        QPointer<WorkPackageSendDialog> dlg = new WorkPackageSendDialog( lst, scheduleManager(), this );
        connect ( dlg->panel(), &WorkPackageSendPanel::sendWorkpackages, this, &TaskWorkPackageView::mailWorkpackages );

        connect ( dlg->panel(), &WorkPackageSendPanel::sendWorkpackages, this, &TaskWorkPackageView::slotWorkPackageSent );
        dlg->exec();
        delete dlg;
        if ( ! m_cmd->isEmpty() ) {
            part()->addCommand( m_cmd );
            m_cmd = 0;
        }
        delete m_cmd;
        m_cmd = 0;
    }
}

void TaskWorkPackageView::slotWorkPackageSent( const QList<Node*> &nodes, Resource *resource )
{
    foreach ( Node *n, nodes ) {
        WorkPackage *wp = new WorkPackage( static_cast<Task*>( n )->workPackage() );
        wp->setOwnerName( resource->name() );
        wp->setOwnerId( resource->id() );
        wp->setTransmitionTime( DateTime::currentDateTime() );
        wp->setTransmitionStatus( WorkPackage::TS_Send );
        m_cmd->addCommand( new WorkPackageAddCmd( static_cast<Project*>( n->projectNode() ), n, wp ) );
    }
}

void TaskWorkPackageView::slotSplitView()
{
    debugPlan;
    m_view->setViewSplitMode( ! m_view->isViewSplit() );
    emit optionsModified();
}

void TaskWorkPackageView::slotOptions()
{
    debugPlan;
    SplitItemViewSettupDialog *dlg = new SplitItemViewSettupDialog( this, m_view, this );
    dlg->addPrintingOptions(sender()->objectName() == "print options");
    connect(dlg, SIGNAL(finished(int)), SLOT(slotOptionsFinished(int)));
    dlg->show();
    dlg->raise();
    dlg->activateWindow();
}

bool TaskWorkPackageView::loadContext( const KoXmlElement &context )
{
    debugPlan;
    ViewBase::loadContext( context );
    return m_view->loadContext( m_view->baseModel()->columnMap(), context );
}

void TaskWorkPackageView::saveContext( QDomElement &context ) const
{
    ViewBase::saveContext( context );
    m_view->saveContext( m_view->baseModel()->columnMap(), context );
}

KoPrintJob *TaskWorkPackageView::createPrintJob()
{
    return m_view->createPrintJob( this );
}

} // namespace KPlato
