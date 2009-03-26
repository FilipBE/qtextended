/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#include "e2_browser.h"
#include "e2_telephonybar.h"
#include "e2_bar.h"
#include "qtopiaserverapplication.h"
#include <QVBoxLayout>
#include <QContentSet>
#include <QCategoryManager>
#include <QMap>
#include "lazycontentstack.h"
#include <QStackedWidget>
#include <QContentSet>
#include <QContent>
#include "launcherview.h"

QTOPIA_REPLACE_WIDGET(QAbstractBrowserScreen, E2BrowserScreen);

// declare E2BrowserStack
class E2BrowserStack : public LazyContentStack
{
Q_OBJECT
public:
    E2BrowserStack(QWidget *parent = 0);
    void showView(const QString &view);
    void resetToView(const QString &view);

    void setViewMode(QListView::ViewMode);

signals:
    void launched(const QString &);
    void currentViewChanged(const QString &);

protected:
    virtual QObject* createView(const QString &);
    virtual void raiseView(const QString &, bool reset);
    virtual void busy(const QContent &);
    virtual QObject* currentViewObject();

private:
    QStackedWidget *m_stack;
    QMap<QString, LauncherView *> m_views;
    QString m_currentView;
    QListView::ViewMode m_mode;
};

// define E2BrowserStack
E2BrowserStack::E2BrowserStack(QWidget *parent)
: LazyContentStack(NoStack, parent), m_stack(0), m_mode(QListView::IconMode)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack);
}

void E2BrowserStack::setViewMode(QListView::ViewMode mode)
{
    m_mode = mode;

    for(QMap<QString, LauncherView *>::Iterator iter = m_views.begin(); iter != m_views.end(); ++iter) {
        (*iter)->setViewMode(m_mode);
    }
}

QObject* E2BrowserStack::createView(const QString &category)
{
    if(category == "Main")
        return 0;

    LauncherView *alv = LauncherView::createLauncherView( "ApplicationLauncherView", m_stack );
    if ( !alv ) {
        qLog(Component) << "E1PhoneBrowser: ApplicationLauncherView not available";
        return 0;
    }
    if("All" == category) {
        QContentSet set(QContent::Application);
        QContentList list = set.items();
        for(int ii = 0; ii < list.count(); ++ii)
            alv->addItem(new QContent(list.at(ii)));
    } else {
        QContentFilter filters = (QContentFilter( QContent::Application ) | QContentFilter( QContent::Folder ))
                    & QContentFilter( QContentFilter::Category, category );
        alv->showCategory( filters );
    }
    alv->setColumns(3);
    alv->setViewMode(m_mode);
    m_views.insert(category, alv);
    m_stack->addWidget(alv);
    return alv;
}

void E2BrowserStack::busy(const QContent &content)
{
    emit launched(content.fileName());
}

void E2BrowserStack::raiseView(const QString &category, bool)
{
    Q_ASSERT(m_views.contains(category));
    QWidget * wid = m_views[category];
    m_stack->setCurrentWidget(wid);
    m_currentView = category;
    emit currentViewChanged(category);
}

QObject* E2BrowserStack::currentViewObject()
{
    return m_stack->currentWidget();
}

void E2BrowserStack::resetToView(const QString &view)
{
    if("Main" == view) {
        if(m_currentView.isEmpty())
            LazyContentStack::resetToView("All");
    } else {
        LazyContentStack::resetToView(view);
    }
}

void E2BrowserStack::showView(const QString &view)
{
    if("Main" == view)
        LazyContentStack::showView("All");
    else
        LazyContentStack::showView(view);
}

// define E2BrowserScreen
E2BrowserScreen::E2BrowserScreen(QWidget *parent, Qt::WFlags flags)
    : QAbstractBrowserScreen(parent, flags), m_mode(IconView), m_stack(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    E2TelephonyBar *tbar = new E2TelephonyBar(this);
    tbar->setFixedWidth(240);
    layout->addWidget(tbar);

    m_stack = new E2BrowserStack(this);
    layout->addWidget(m_stack);
    QObject::connect(m_stack, SIGNAL(launched(QString)),
                     this, SIGNAL(applicationLaunched(QString)));
    QObject::connect(m_stack, SIGNAL(currentViewChanged(QString)),
                     this, SLOT(viewChanged(QString)));

    E2Bar *bbar = new E2Bar(this);
    bbar->setFixedWidth(240);
    E2Button *but = new E2Button(bbar);
    but->setPixmap(QPixmap(":image/samples/e2_menu"));
    m_menu = new E2Menu(0);
    QObject::connect(m_menu, SIGNAL(itemClicked(int)),
                     this, SLOT(menuClicked(int)));
    m_menu->setFixedWidth(150);
    but->setMenu(m_menu);
    m_menu->addItem("List View");
    m_menu->addSeparator();
    m_menu->addItem("Task Manager");
    bbar->addButton(but, 42);
    m_catMenu = new E2Button(bbar);
    m_catMenu->setPixmap(QPixmap(":image/samples/e2_up"));
    bbar->addButton(m_catMenu, 0);
    but = new E2Button(bbar);
    but->setPixmap(QPixmap(":image/samples/e2_cross"));
    QObject::connect(but, SIGNAL(clicked()), this, SLOT(close()));
    bbar->addButton(but, 42);
    layout->addWidget(bbar);

    m_categories.append("All");
    m_categoryNames.append("All");
    m_catMenu->setText("All");

    E2Menu *catMenu = new E2Menu(0);
    catMenu->addItem("All");
    catMenu->addSeparator();

    QContentSet contentSet( (QContentFilter( QContent::Folder ) & QContentFilter::category( "MainApplications" )) );
    QContentSetModel model(&contentSet);
    for(int ii = 0; ii < model.rowCount(); ++ii) {
        QContent c = model.content( ii );
        if( c.type().startsWith( QLatin1String( "Folder/" ) ) ) {
            m_categories.append(c.type().mid( 7 ));
            m_categoryNames.append(c.name());
            catMenu->addItem(m_categoryNames.last());
        }
    }

    m_catMenu->setMenu(catMenu, true);
    QObject::connect(catMenu, SIGNAL(itemClicked(int)),
                     this, SLOT(categoryChanged(int)));
}

void E2BrowserScreen::categoryChanged(int cat)
{
    m_catMenu->setText(m_categoryNames.at(cat));
    resetToView(m_categories.at(cat));
}

void E2BrowserScreen::viewChanged(const QString &cat)
{
    for(int ii = 0; ii < m_categories.count(); ++ii) {
        if(m_categories.at(ii) == cat) {
            m_catMenu->setText(m_categoryNames.at(ii));
            return;
        }
    }
}

QString E2BrowserScreen::currentView() const
{
    return "Main";
}

bool E2BrowserScreen::viewAvailable(const QString &view) const
{
    Q_UNUSED(view);
    return true;
}

void E2BrowserScreen::resetToView(const QString &v)
{
    m_stack->resetToView(v);
}

void E2BrowserScreen::moveToView(const QString &v)
{
    m_stack->showView(v);
}

void E2BrowserScreen::menuClicked(int item)
{
    if(0 == item)
        toggleViewMode();
}

void E2BrowserScreen::toggleViewMode()
{
    if(m_mode == IconView) {
        m_mode = ListView;
        m_menu->replaceItem(0, "Thumbnail View");
        m_stack->setViewMode(QListView::ListMode);
    } else {
        m_mode = IconView;
        m_menu->replaceItem(0, "List View");
        m_stack->setViewMode(QListView::IconMode);
    }
}

#include "e2_browser.moc"

