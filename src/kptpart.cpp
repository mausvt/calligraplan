/* This file is part of the KDE project
   Copyright (C) 2012 C. Boemann <cbo@kogmbh.com>

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
#include "kptpart.h"

#include "kptview.h"
#include "kptmaindocument.h"
#include "kptfactory.h"
#include "welcome/WelcomeView.h"
#include "kpthtmlview.h"
#include "Help.h"
#include "kptdebug.h"

#include <KoComponentData.h>

#include <KRecentFilesAction>
#include <KXMLGUIFactory>
#include <KConfigGroup>
#include <KHelpClient>
#include <KRun>
#include <KDesktopFile>
#include <KAboutData>

#include <QStackedWidget>
#include <QDesktopServices>

using namespace KPlato;

Part::Part(QObject *parent)
    : KoPart(Factory::global(), parent)
    , startUpWidget(0)
{
    setTemplatesResourcePath(QLatin1String("calligraplan/templates/"));

    KDesktopFile df(componentData().aboutData().desktopFileName() + ".desktop");
    new Help(df.readDocPath());
}

Part::~Part()
{
}

void Part::setDocument(KPlato::MainDocument *document)
{
    KoPart::setDocument(document);
    m_document = document;
}

KoView *Part::createViewInstance(KoDocument *document, QWidget *parent)
{
    // synchronize view selector
    View *view = dynamic_cast<View*>(views().value(0));
    /*FIXME
    if (view && m_context) {
        QDomDocument doc = m_context->save(view);
        m_context->setContent(doc.toString());
    }*/
    view = new View(this, qobject_cast<MainDocument*>(document), parent);
//    connect(view, SIGNAL(destroyed()), this, SLOT(slotViewDestroyed()));
//    connect(document, SIGNAL(viewListItemAdded(const ViewListItem*,const ViewListItem*,int)), view, SLOT(addViewListItem(const ViewListItem*,const ViewListItem*,int)));
//    connect(document, SIGNAL(viewListItemRemoved(const ViewListItem*)), view, SLOT(removeViewListItem(const ViewListItem*)));
    return view;
}

KoMainWindow *Part::createMainWindow()
{
    KoMainWindow *w = new KoMainWindow(PLAN_MIME_TYPE, componentData());
    QAction *handbookAction = w->action("help_contents");
    if (handbookAction) {
        // we do not want to use khelpcenter as we do not install docs
        disconnect(handbookAction, 0, 0, 0);
        connect(handbookAction, &QAction::triggered, this, &Part::slotHelpContents);
    }
    return w;
}

void Part::slotHelpContents()
{
    QDesktopServices::openUrl(QUrl(Help::page("Manual")));
}

void Part::showStartUpWidget(KoMainWindow *parent)
{
    m_toolbarVisible = parent->factory()->container("mainToolBar", parent)->isVisible();
    if (m_toolbarVisible) {
        parent->factory()->container("mainToolBar", parent)->hide();
    }

    if (startUpWidget) {
        startUpWidget->show();
    } else {
        createStarUpWidget(parent);
        parent->setCentralWidget(startUpWidget);
    }

    parent->setPartToOpen(this);

}

void Part::slotOpenTemplate(const QUrl &url)
{
    openTemplate(url);
}

void Part::openTemplate(const QUrl &url)
{
    debugPlan<<"Open shared resources template:"<<url;
    m_document->setLoadingTemplate(true);
    m_document->setLoadingSharedResourcesTemplate(url.fileName() == "SharedResources.plant");
    KoPart::openTemplate(url);
    m_document->setLoadingTemplate(false);
}

void Part::openTaskModule(const QUrl &url)
{
    Part *part = new Part(0);
    MainDocument *doc = new MainDocument(part);
    part->setDocument(doc);
    doc->setIsTaskModule(true);
    mainWindows().first()->openDocument(part, url);
}

void Part::createStarUpWidget(KoMainWindow *parent)
{
    startUpWidget = new QStackedWidget(parent);

    startUpWidget->addWidget(createWelcomeView(parent));
    startUpWidget->addWidget(createIntroductionView());
}

void Part::finish()
{
    mainWindows().first()->setRootDocument(document(), this);
    if (m_toolbarVisible) {
        KoPart::mainWindows().first()->factory()->container("mainToolBar", mainWindows().first())->show();
    }
}

QWidget *Part::createWelcomeView(KoMainWindow *mw)
{
    MainDocument *doc = static_cast<MainDocument*>(document());

    WelcomeView *v = new WelcomeView(this, doc, startUpWidget);
    v->setProject(&(doc->getProject()));

    KSharedConfigPtr configPtr = Factory::global().config();
    KRecentFilesAction recent("x", 0);
    recent.loadEntries(configPtr->group("RecentFiles"));
    v->setRecentFiles(recent.items());

    connect(v, &WelcomeView::loadSharedResources, doc, &MainDocument::insertResourcesFile);
    connect(v, &WelcomeView::recentProject, mw, &KoMainWindow::slotFileOpenRecent);
    connect(v, &WelcomeView::showIntroduction, this, &Part::slotShowIntroduction);
    connect(v, &WelcomeView::projectCreated, doc, &MainDocument::slotProjectCreated);
    connect(v, &WelcomeView::finished, this, &Part::finish);

    connect(v, &WelcomeView::openTemplate, this, &Part::slotOpenTemplate);

    return v;
}

void Part::slotShowIntroduction()
{
    startUpWidget->setCurrentIndex(1);
    slotOpenUrlRequest(static_cast<HtmlView*>(startUpWidget->currentWidget()), QUrl("about:plan/main"));
}

void Part::slotOpenUrlRequest( HtmlView *v, const QUrl &url )
{
    debugPlan<<url;
    if ( url.scheme() == QLatin1String("about") ) {
        if ( url.url() == QLatin1String("about:close") ) {
            startUpWidget->setCurrentIndex(0);
            return;
        }
        if ( url.url().startsWith( QLatin1String( "about:plan" ) ) ) {
            MainDocument *doc = static_cast<MainDocument*>(document());
            doc->aboutPage().generatePage( v->htmlPart(), url );
            return;
        }
    }
    if ( url.scheme() == QLatin1String("help") ) {
        KHelpClient::invokeHelp( "", url.fileName() );
        return;
    }
    // try to open the url
    debugPlan<<url<<"is external, discard";
    new KRun( url,  currentMainwindow() );
}

QWidget *Part::createIntroductionView()
{
    HtmlView *v = new HtmlView(this, document(), startUpWidget );
    v->htmlPart().setJScriptEnabled(false);
    v->htmlPart().setJavaEnabled(false);
    v->htmlPart().setMetaRefreshEnabled(false);
    v->htmlPart().setPluginsEnabled(false);

    slotOpenUrlRequest( v, QUrl( "about:plan/main" ) );

    connect( v, &KPlato::HtmlView::openUrlRequest, this, &KPlato::Part::slotOpenUrlRequest );

    return v;
}
