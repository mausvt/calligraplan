/* This file is part of the KDE project
  Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
  Copyright (C) 2002 - 2009, 2011, 2012 Dag Andersen <danders@get2net.dk>

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
#include "view.h"
#include "mainwindow.h"
#include "taskworkpackageview.h"
#include "workpackage.h"
#include "packagesettings.h"
#include "taskcompletiondialog.h"
#include "calligraplanworksettings.h"
#include "kpttaskeditor.h"
#include "kpttaskdescriptiondialog.h"
#include "kptcommonstrings.h"

#include "KoDocumentInfo.h"
#include <KoMainWindow.h>

#include <QApplication>
#include <QLabel>
#include <QString>
#include <QSize>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPrinter>
#include <QPrintDialog>
#include <QDomDocument>
#include <QPointer>
#include <QMenu>
#include <QAction>

#include <KLocalizedString>
#include <ktoolbar.h>

#include <kxmlguifactory.h>
#include <ktoolinvocation.h>
#include <kactioncollection.h>
#include <QTemporaryFile>

#include <kmessagebox.h>

#include <KoIcon.h>

#include "part.h"
#include "factory.h"

#include "kptviewbase.h"
#include "kptdocumentseditor.h"

#include "kptnode.h"
#include "kptproject.h"
#include "kpttask.h"
#include "kptcommand.h"
#include "kptdocuments.h"
#include "kpttaskprogressdialog.h"
#include "kptcalendar.h"

#include <assert.h>

#include "debugarea.h"

namespace KPlatoWork
{

View::View( Part *part,  QWidget *parent, KActionCollection *collection )
    : QStackedWidget( parent ),
    m_part( part ),
    m_scheduleActionGroup( new QActionGroup( this ) ),
    m_manager( 0 )
{
    m_readWrite = part->isReadWrite();
    debugPlanWork<<m_readWrite;

    // Add sub views
    createViews();

    // The menu items
    // ------ Edit
    actionRemoveSelectedPackages  = new QAction(koIcon("edit-delete"), i18n("Remove Packages"), this);
    collection->addAction("package_remove_selected", actionRemoveSelectedPackages );
    connect( actionRemoveSelectedPackages, &QAction::triggered, this, &View::slotRemoveSelectedPackages );

    actionRemoveCurrentPackage  = new QAction(koIcon("edit-delete"), i18n("Remove Package"), this);
    collection->addAction("package_remove_current", actionRemoveCurrentPackage );
    connect( actionRemoveCurrentPackage, &QAction::triggered, this, &View::slotRemoveCurrentPackage );

    actionViewList  = new QAction(koIcon("view-list-tree"), i18n("List"), this);
    actionViewList->setToolTip( i18nc( "@info:tooltip", "Select task list" ) );
    collection->addAction("view_list", actionViewList );
    connect( actionViewList, &QAction::triggered, this, &View::slotViewList );

    actionViewGantt  = new QAction(koIcon("view-time-schedule"), i18n("Gantt"), this);
    actionViewGantt->setToolTip( i18nc( "@info:tooltip", "Select timeline" ) );
    collection->addAction("view_gantt", actionViewGantt );
    connect( actionViewGantt, &QAction::triggered, this, &View::slotViewGantt );

//     actionTaskProgress  = new QAction(koIcon("document-edit"), i18n("Progress..."), this);
//     collection->addAction("task_progress", actionTaskProgress );
//     connect( actionTaskProgress, SIGNAL(triggered(bool)), SLOT(slotTaskProgress()) );

    //------ Settings
    actionConfigure  = new QAction(koIcon("configure"), i18n("Configure PlanWork..."), this);
    collection->addAction("configure", actionConfigure );
    connect( actionConfigure, &QAction::triggered, this, &View::slotConfigure );

    //------ Popups
    actionEditDocument  = new QAction(koIcon("document-edit"), i18n("Edit..."), this);
    collection->addAction("edit_document", actionEditDocument );
    connect( actionEditDocument, SIGNAL(triggered(bool)), SLOT(slotEditDocument()) );

    actionViewDocument  = new QAction(koIcon("document-preview"), i18nc( "@verb", "View..."), this);
    collection->addAction("view_document", actionViewDocument );
    connect( actionViewDocument, &QAction::triggered, this, &View::slotViewDocument );

    // FIXME remove UndoText::removeDocument() when string freeze is lifted
    actionRemoveDocument = new QAction(koIcon("list-remove"), UndoText::removeDocument().toString(), this);
    collection->addAction("remove_document", actionRemoveDocument );
    connect( actionRemoveDocument, &QAction::triggered, this, &View::slotRemoveDocument );

    actionSendPackage  = new QAction(koIcon("mail-send"), i18n("Send Package..."), this);
    collection->addAction("edit_sendpackage", actionSendPackage );
    connect( actionSendPackage, &QAction::triggered, this, &View::slotSendPackage );

    actionPackageSettings  = new QAction(koIcon("document-properties"), i18n("Package Settings..."), this);
    collection->addAction("edit_packagesettings", actionPackageSettings );
    connect( actionPackageSettings, &QAction::triggered, this, &View::slotPackageSettings );

    actionTaskCompletion  = new QAction(koIcon("document-edit"), i18n("Edit Progress..."), this);
    collection->addAction("task_progress", actionTaskCompletion );
    connect( actionTaskCompletion, &QAction::triggered, this, &View::slotTaskCompletion );

    actionViewDescription  = new QAction(/*koIcon("document_view"),*/ i18n("View Description..."), this);
    collection->addAction("task_description", actionViewDescription );
    connect( actionViewDescription, &QAction::triggered, this, &View::slotTaskDescription );


    updateReadWrite( m_readWrite );
    //debugPlanWork<<" end";

    loadContext();
    slotCurrentChanged( currentIndex() );
    connect( this, &QStackedWidget::currentChanged, this, &View::slotCurrentChanged );

    slotSelectionChanged();
}

View::~View()
{
    saveContext();
}

void View::slotCurrentChanged( int index )
{
    actionViewList->setEnabled( index != 0 );
    actionViewGantt->setEnabled( index != 1 );
    saveContext();
}

void View::slotViewList()
{
    debugPlanWork;
    setCurrentIndex( 0 );
}

void View::slotViewGantt()
{
    debugPlanWork;
    setCurrentIndex( 1 );
}

void View::createViews()
{
    QWidget *v = createTaskWorkPackageView();
    addWidget( v );
    v = createGanttView();
    addWidget( v );
}

TaskWorkPackageView *View::createTaskWorkPackageView()
{
    TaskWorkPackageView *v = new TaskWorkPackageView( part(), this );

    connect( v, &AbstractView::requestPopupMenu, this, &View::slotPopupMenu );

    connect( v, &AbstractView::selectionChanged, this, &View::slotSelectionChanged );
    v->updateReadWrite( m_readWrite );
    v->loadContext();
    return v;
}

TaskWPGanttView *View::createGanttView()
{
    TaskWPGanttView *v = new TaskWPGanttView( part(), this );

    connect( v, &AbstractView::requestPopupMenu, this, &View::slotPopupMenu );

    connect( v, &AbstractView::selectionChanged, this, &View::slotSelectionChanged );
    v->updateReadWrite( m_readWrite );
    v->loadContext();
    return v;
}

void View::setupPrinter( QPrinter &/*printer*/, QPrintDialog &/*printDialog */)
{
    //debugPlanWork;
}

void View::print( QPrinter &/*printer*/, QPrintDialog &/*printDialog*/ )
{
}

void View::slotSelectionChanged()
{
    bool enable = ! currentView()->selectedNodes().isEmpty();
    actionRemoveSelectedPackages->setEnabled( enable );
    actionRemoveCurrentPackage->setEnabled( enable );
}

void View::slotEditCut()
{
    //debugPlanWork;
}

void View::slotEditCopy()
{
    //debugPlanWork;
}

void View::slotEditPaste()
{
    //debugPlanWork;
}

void View::slotProgressChanged( int )
{
}

void View::slotConfigure()
{
}

ScheduleManager *View::currentScheduleManager() const
{
    return 0; // atm we always work with default manager
}

void View::updateReadWrite( bool readwrite )
{
    debugPlanWork<<m_readWrite<<"->"<<readwrite;
    m_readWrite = readwrite;

//    actionTaskProgress->setEnabled( readwrite );

    emit sigUpdateReadWrite( readwrite );
}

Part *View::part() const
{
    return m_part;
}

void View::slotPopupMenu( const QString& name, const QPoint & pos )
{
    Q_ASSERT( m_part->factory() );
    if ( m_part->factory() == 0 ) {
        return;
    }
    QMenu *menu = ( ( QMenu* ) m_part->factory() ->container( name, m_part ) );
    if ( menu == 0 ) {
        return;
    }
    QList<QAction*> lst;
    AbstractView *v = currentView();
    if ( v ) {
        lst = v->contextActionList();
        debugPlanWork<<lst;
        if ( ! lst.isEmpty() ) {
            menu->addSeparator();
            foreach ( QAction *a, lst ) {
                menu->addAction( a );
            }
        }
    }
    menu->exec( pos );
    foreach ( QAction *a, lst ) {
        menu->removeAction( a );
    }
}

bool View::loadContext()
{
    debugPlanWork;
    setCurrentIndex( PlanWorkSettings::self()->currentView() );
    return true;
}

void View::saveContext() const
{
    debugPlanWork;
    PlanWorkSettings::self()->setCurrentView( currentIndex() );
    PlanWorkSettings::self()->save();
}

void View::slotEditDocument()
{
    slotEditDocument( currentDocument() );
}

void View::slotEditDocument( Document *doc )
{
    debugPlanWork<<doc;
    if ( doc == 0 ) {
        debugPlanWork<<"No document";
        return;
    }
    if ( doc->type() != Document::Type_Product ) {
        KMessageBox::error( 0, i18n( "This file is not editable" ) );
        return;
    }
    part()->editWorkpackageDocument( doc );
}

void View::slotViewDocument()
{
    emit viewDocument( currentDocument() );
}

void View::slotRemoveDocument()
{
    part()->removeDocument( currentDocument() );
}

void View::slotPackageSettings()
{
    WorkPackage *wp = part()->findWorkPackage( currentNode() );
    if ( wp == 0 ) {
        return;
    }
    QPointer<PackageSettingsDialog> dia = new PackageSettingsDialog( *wp, this );
    if ( dia->exec() == QDialog::Accepted && dia ) {
        KUndo2Command *cmd = dia->buildCommand();
        if ( cmd ) {
            debugPlanWork;
            part()->addCommand( cmd );
        }
    }
    delete dia;
}

void View::slotSendPackage()
{
    Node *node = currentNode();
    if ( node == 0 ) {
        KMessageBox::error(0, i18n("No work package is selected" ) );
        return;
    }
    debugPlanWork<<node->name();
    WorkPackage *wp = part()->findWorkPackage( node );
    if ( wp == 0 ) {
        KMessageBox::error(0, i18n("Cannot find work package" ) );
        return;
    }
/*    if ( wp->isModified() ) {
        int r = KMessageBox::questionYesNoCancel( 0, i18n("This work package has been modified.\nDo you want to save it before sending?" ), node->name() );
        switch ( r ) {
            case KMessageBox::Cancel: return;
            case KMessageBox::Yes: wp->saveToProjects( part() ); break;
            default: break;
        }
    }*/

    QTemporaryFile temp(QDir::tempPath() + QLatin1String("/calligraplanwork_XXXXXX") + QLatin1String( ".planwork" ));
    temp.setAutoRemove( false );
    if ( ! temp.open() ) {
        KMessageBox::error( 0, i18n("Could not open temporary file. Sending is aborted." ) );
        return;
    }
    bool wasmodified = wp->isModified();
    wp->saveNativeFormat( part(), temp.fileName() );
    wp->setModified( wasmodified );

    QStringList attachURLs;
    attachURLs << temp.fileName();

    QString to = node->projectNode()->leader();
    QString cc;
    QString bcc;
    QString subject = i18n( "Work Package: %1", node->name() );
    QString body = node->projectNode()->name();
    QString messageFile;

    KToolInvocation::invokeMailer( to, cc, bcc, subject, body, messageFile, attachURLs );
}

void View::slotTaskDescription()
{
    Task *node = qobject_cast<Task*>( currentNode() );
    if ( node == 0 ) {
        return;
    }
    QPointer<TaskDescriptionDialog> dlg = new TaskDescriptionDialog( *node, this, true );
    dlg->exec();
    delete dlg;
}

AbstractView *View::currentView() const
{
    return qobject_cast<AbstractView*>( currentWidget() );
}

Node *View::currentNode() const
{
    AbstractView *v = currentView();
    return v ? v->currentNode() : 0;
}

Document *View::currentDocument() const
{
    AbstractView *v = currentView();
    return v ? v->currentDocument() : 0;
}

void View::slotTaskProgress()
{
    debugPlanWork;
    Task *n = qobject_cast<Task*>( currentNode() );
    if ( n == 0 ) {
        return;
    }
    StandardWorktime *w = qobject_cast<Project*>( n->projectNode() )->standardWorktime();
    QPointer<TaskProgressDialog> dlg = new TaskProgressDialog( *n, currentScheduleManager(), w, this );
    if ( dlg->exec() == QDialog::Accepted && dlg ) {
        KUndo2Command *cmd = dlg->buildCommand();
        if ( cmd ) {
            cmd->redo(); //FIXME m_part->addCommand( cmd );
        }
    }
}

void View::slotTaskCompletion()
{
    debugPlanWork;
    WorkPackage *wp = m_part->findWorkPackage( currentNode() );
    if ( wp == 0 ) {
        return;
    }
    QPointer<TaskCompletionDialog> dlg = new TaskCompletionDialog( *wp, currentScheduleManager(), this );
    if ( dlg->exec() == QDialog::Accepted && dlg ) {
        KUndo2Command *cmd = dlg->buildCommand();
        if ( cmd ) {
            m_part->addCommand( cmd );
        }
    }
    delete dlg;
}

void View::slotRemoveSelectedPackages()
{
    debugPlanWork;
    QList<Node*> lst = currentView()->selectedNodes();
    if ( lst.isEmpty() ) {
        return;
    }
    m_part->removeWorkPackages( lst );
}

void View::slotRemoveCurrentPackage()
{
    debugPlanWork;
    Node *n = currentNode();
    if ( n == 0 ) {
        return;
    }
    m_part->removeWorkPackage( n );
}


}  //KPlatoWork namespace
