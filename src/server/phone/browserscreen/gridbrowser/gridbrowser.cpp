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

#include "gridbrowser.h"

#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QtopiaServiceHistoryModel>
#include <QVBoxLayout>
#include <QExpressionEvaluator>
#include <QSoftMenuBar>

#include "launcherview.h"


// define PhoneBrowserStack
PhoneBrowserStack::PhoneBrowserStack(QWidget *parent)
: LazyContentStack(NoFlags, parent),
    phoneLauncher(0),
    menuIdx(-1),
    appCategories("Applications"),
    gridNeedsRotation(0),
    delayedRotatedPhoneLauncher(0)
{

#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    rotation = new QValueSpaceItem("/UI/Rotation");
    connect(rotation, SIGNAL(contentsChanged()), this, SLOT(rotationChanged()));
    defaultRotation = rotation->value("Default", 0).toInt();
#endif

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);
    stack = new QStackedWidget(this);
    layout->addWidget(stack);
}

PhoneMainMenu* PhoneBrowserStack::rotatedPhoneLauncher(bool create)
{
#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    if (!delayedRotatedPhoneLauncher && create) {
        // swap rows and columns for rotated grid
        QSettings cfg(Qtopia::defaultButtonsFile(),
                        QSettings::IniFormat); // No tr
        int newRows = cfg.value("Menu/Columns", 3).toInt();
        int newColumns = cfg.value("Menu/Rows", 3).toInt();
        cfg.setValue("Menu/Rows",newRows);
        cfg.setValue("Menu/Columns", newColumns );

        delayedRotatedPhoneLauncher = new PhoneMainMenu(cfg, stack);
        if ( !QObject::connect( delayedRotatedPhoneLauncher, SIGNAL(clicked(QContent)),
                                this, SLOT(execContent(QContent))) )
            Q_ASSERT(0);

        stack->addWidget(delayedRotatedPhoneLauncher);
        cfg.setValue("Menu/Rows",newColumns );
        cfg.setValue("Menu/Columns", newRows);
    }
#else
    Q_UNUSED(create);
#endif

    return delayedRotatedPhoneLauncher;
}

QObject* PhoneBrowserStack::createView(const QString &view)
{
    if("Main" == view) {
        Q_ASSERT(!phoneLauncher);
        QSettings cfg(Qtopia::defaultButtonsFile(),
                      QSettings::IniFormat); // No tr
        phoneLauncher = new PhoneMainMenu(cfg, stack);
        stack->addWidget(phoneLauncher);
        stack->setCurrentWidget(phoneLauncher);

        return phoneLauncher;
    } else if("Folder/Documents" == view) {
        return addType("Documents", tr("Documents"),
                       QIcon(QPixmap(":image/qpe/DocsIcon")));
    } else if("Folder/ContentSet" == view ) {
        return createContentSetView();
    } else if(view.startsWith("Folder/")) {
        return createAppView(view.mid(7));
    }

    return 0;
}

void PhoneBrowserStack::raiseView(const QString &view, bool reset)
{
    if(reset) {
        QMap<QString, TypeView>::Iterator it;
        for (it = map.begin(); it != map.end(); ++it)
            (*it).view->resetSelection();
    }

    if("Main" == view) {
        QContent *cur = 0;

        if (!gridNeedsRotation) {
            Q_ASSERT(phoneLauncher);

            stack->setCurrentWidget(phoneLauncher);

            if(reset)
                phoneLauncher->showDefaultSelection();

            cur = phoneLauncher->currentItem();
        } else {
#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
            PhoneMainMenu* launcher = rotatedPhoneLauncher();
            stack->setCurrentWidget(launcher);

            if(reset)
                launcher->showDefaultSelection();

            cur = launcher->currentItem();
#endif
        }

        if ( cur )
            setWindowTitle( cur->name() );
        setObjectName(QLatin1String("main-menu"));
    } else {
        Q_ASSERT(map.contains(view));
        TypeView tv = map[view];

        QString name = tv.name;
        if (name.isNull())
            name = tv.view->windowTitle();
        setWindowTitle(name);

        // ensure top-level window title is correct if opening a non-app window
        // from the launcher in touchscreen mode
        topLevelWidget()->setWindowTitle(name);

        stack->setCurrentWidget(tv.view);
    }
}

void PhoneBrowserStack::rotationChanged()
{

#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    currentRotation = rotation->value("Current",0).toInt();
    if ( qAbs(currentRotation - defaultRotation) == 0
         || qAbs(currentRotation - defaultRotation) == 180) {
        gridNeedsRotation = false;
    } else {
        gridNeedsRotation = true;
    }

    if (stack->count() &&
            (stack->currentWidget() == phoneLauncher
        || stack->currentWidget() == rotatedPhoneLauncher(false) ))
        raiseView("Main", false);
#endif
}

void PhoneBrowserStack::show()
{
    emit visibilityChanged();
}

void PhoneBrowserStack::hide()
{
    emit visibilityChanged();
}

void PhoneBrowserStack::back()
{
    LazyContentStack::back();
}


void PhoneBrowserStack::closeEvent(QCloseEvent *e)
{
    emit visibilityChanged();
    LazyContentStack::closeEvent(e);
}

void PhoneBrowserStack::keyPressEvent(QKeyEvent *ke)
{
    if ( ke->key() == Qt::Key_Hangup || ke->key() == Qt::Key_Flip )
        reset();
    ke->ignore();   //always ignore (Key_Hangup and Key_Flip still need to be processed by the system)
}

void PhoneBrowserStack::busy(const QContent &content)
{
    LazyContentStack::busy(content);
    if(currentViewObject() && currentViewObject()->inherits("LauncherView"))
        qobject_cast<LauncherView *>(currentViewObject())->setBusy(true);
    else if(stack->currentWidget() == phoneLauncher)
        phoneLauncher->setBusy(true);
#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
    else if(stack->count() && stack->currentWidget() == rotatedPhoneLauncher(false))
        rotatedPhoneLauncher(false)->setBusy(true);

#endif
    emit applicationLaunched(content.fileName());
}

void PhoneBrowserStack::notBusy()
{
    LazyContentStack::notBusy();
    if(currentViewObject()->inherits("LauncherView")) {
        LauncherView *launcherView = qobject_cast<LauncherView *>(currentViewObject());
        if ( launcherView ) {
            launcherView->setBusy(false);
        }
    }

    // If the PhoneLauncher is still 'busy', it, too, must have its busy flag switched off.
    if (stack->count()) {
        QWidget* widget = stack->currentWidget();
        if (widget == phoneLauncher)
            phoneLauncher->setBusy(false);

#ifdef QT_QWS_DYNAMIC_TRANSFORMATION
        if (widget == rotatedPhoneLauncher(false))
           rotatedPhoneLauncher(false)->setBusy(false);
#endif
    }
}

void PhoneBrowserStack::noView(const QString &view)
{
    QString text;
    if (view.toLower().startsWith("folder/"))
        text = tr("<qt>No applications of this type are installed.</qt>");
    else
        text = tr("<qt>No application is defined for this document.</qt>");
    showMessageBox(tr("No application"), text);
}

QObject *PhoneBrowserStack::currentViewObject()
{
    return stack->currentWidget();
}

LauncherView *PhoneBrowserStack::createAppView(const QString &category)
{
    qLog(Hardware) << __PRETTY_FUNCTION__;
    TypeView tv;
    LauncherView *view = LauncherView::createLauncherView("ApplicationLauncherView", this);
    if ( !view ) {
        qLog(Component) << "PhoneBrowserStack: ApplicationLauncherView not available";
        return 0;
    }

    view->showCategory( (QContentFilter( QContent::Application ) | QContentFilter( QContent::Folder ))
                & QContentFilter( QContentFilter::Category, category ));
    tv.view = view;
    tv.view->setObjectName(category);

    QFont f(font());
    f.setWeight(QFont::Bold);
    tv.view->setFont(f);
    tv.view->setViewMode(QListView::ListMode);

    tv.name = appCategories.label(category);
    tv.icon = appCategories.icon(category);

    map["Folder/"+category] = tv;
    stack->addWidget(tv.view);

    return tv.view;
}

LauncherView * PhoneBrowserStack::addType(const QString& type, const QString& name, const QIcon &icon)
{
    TypeView tv;
    LauncherView *lv = 0;
    if("Documents" == type) {
        //select DocumentLauncherView
        QSettings cfg( "Trolltech","UIFactory"); // No tr
        QByteArray launcherName = cfg.value("Mappings/DocumentLauncherView", "DocumentLauncherView" ).toByteArray();
        lv = LauncherView::createLauncherView( launcherName, this, 0 );
        if ( !lv ) {
            qLog(Component) << "PhoneBrowserStack:"<< launcherName << "not available";
            return 0;
        }
        tv.view = lv;
        tv.view->setObjectName(type);
    } else {
        Q_ASSERT(!"Unknown view");
        return 0;
    }

    QFont f(font());
    f.setWeight(QFont::Bold);
    tv.view->setFont(f);
    tv.view->setViewMode(QListView::ListMode);

    tv.name = name;
    tv.icon = icon;

    map["Folder/"+type] = tv;
    stack->addWidget( tv.view );
    return tv.view;
}

LauncherView * PhoneBrowserStack::createContentSetView()
{
    TypeView tv;

    LauncherView *view = LauncherView::createLauncherView( "ContentSetLauncherView", this );
    if (!view) {
        qLog(Component) << "PhoneBrowserStack: ContentSetLauncherView not available";
        return 0;
    }

    tv.view = view;
    tv.name = tr( "Content" );

    QFont f(font());
    f.setWeight(QFont::Bold);
    tv.view->setFont(f);
    tv.view->setViewMode(QListView::ListMode);
    tv.view->setObjectName(tv.name);

    map["Folder/ContentSet"] = tv;
    stack->addWidget( tv.view );
    return tv.view;
}

void PhoneBrowserStack::showMessageBox(const QString& title, const QString& text, QAbstractMessageBox::Icon icon)
{
    if (!warningBox) {
        warningBox = QAbstractMessageBox::messageBox(this, title, text, icon);
        warningBox->setAttribute(Qt::WA_DeleteOnClose); // It's a QPointer<> so safe.
    }
    warningBox->setText(text);
    QtopiaApplication::showDialog(warningBox);
}

void PhoneBrowserStack::showType(const QString &type)
{
    resetToView(type);
}

// define PhoneMainMenu
PhoneMainMenu::PhoneMainMenu(QSettings &config, QWidget * parent)
: PhoneLauncherView(config.value("Menu/Rows", 3).toInt(),
                    config.value("Menu/Columns", 3).toInt(),
                    config.value("Menu/Map", "123456789").toString(),
                    config.value("Menu/Animator","").toString(),
                    config.value("Menu/AnimatorBackground","").toString(),
                    parent)
{
    makeLauncherMenu(config);
}

void PhoneMainMenu::showDefaultSelection()
{
    setCurrentItem(defaultSelection);
}

// Make it simple to map between expressions and keys
class PhoneMainMenu::ItemExpression : public QExpressionEvaluator
{
public:
    ItemExpression(const QChar &c, const QByteArray &exp, QObject *parent = 0)
        : QExpressionEvaluator(exp, parent), m_c(c)
    {
    }

    QChar character() const
    {
        return m_c;
    }

private:
    QChar m_c;
};

void PhoneMainMenu::makeLauncherMenu(QSettings &cfg)
{
    cfg.beginGroup("Menu"); // No tr
    const int menur = cfg.value("Rows",3).toInt();
    const int menuc = cfg.value("Columns",3).toInt();
    menuKeyMap = cfg.value("Map","123456789").toString();

    qLog(UI) << "begin PhoneMainMenu create";

    qLog(UI) << "PhoneMainMenu:";
    qLog(UI) << "    " << menur << "x" << menuc;
    qLog(UI) << "    Mapping keys:" << menuKeyMap;

    // For multitasking, this is always needed.
    for (int i = 0; i < menur*menuc; i++) {
        QChar key = menuKeyMap[i];
        QStringList entries = cfg.value(QString(key)).toStringList();
        if(entries.isEmpty())
            qLog(UI) << "    Key" << QString(key) << ": No mapping";
        else
            qLog(UI) << "    Key" << QString(key) << ":" << entries.join(",");

        for(int jj = 0; jj < entries.count(); ++jj) {
            const QString &entry = entries.at(jj);

            QString lnk;
            QString expr;

            // Check for expression
            int exprIdx = entry.indexOf('{');
            if(-1 != exprIdx) {
                lnk = entry.left(exprIdx);
                int exprEndIdx = entry.indexOf('}', exprIdx);
                expr = entry.mid(exprIdx + 1, (exprEndIdx == -1)?-1:exprEndIdx - exprIdx - 1);
            } else {
                lnk = entry;
            }

            QContent *appLnk = readLauncherMenuItem(lnk);
            if(appLnk) {
                if(expr.isEmpty())
                    qLog(UI) << "        Name:" << appLnk->name()
                             <<         "Icon:" << appLnk->icon()
                             <<         "Exec:" << appLnk->executableName();
                else
                    qLog(UI) << "        Name:" << appLnk->name()
                             <<         "Icon:" << appLnk->icon()
                             <<         "Exec:" << appLnk->executableName()
                             <<         "Expr:" << expr;

                Item item;
                item.lnk = *appLnk;
                item.expr = 0;
                if(!expr.isEmpty()) {
                    item.expr = new ItemExpression(key, expr.toLatin1());
                    QObject::connect(item.expr, SIGNAL(termsChanged()),
                                     this, SLOT(expressionChanged()));
                }

                mainMenuEntries[key].append(item);

                delete appLnk;
            } else {
                qLog(UI) << "        No target: " << lnk;
            }
        }

        QMap<QChar,Items>::Iterator iter = mainMenuEntries.find(key);
        if(iter != mainMenuEntries.end()) {
            // Setup default
            bool setDefault = false;
            for(int jj = 0; !setDefault && jj < iter->count(); ++jj) {
                if((*iter)[jj].exprTrue()) {
                    iter->activeItem = jj;
                    setDefault = true;
                    addItem(new QContent(iter->at(jj).lnk), i);
                }
            }
            if(!setDefault) {
                iter->activeItem = iter->count() - 1;
                addItem(new QContent(iter->at(iter->count() - 1).lnk), i);
            }
        }
    }

    QString d = cfg.value("Default",menuKeyMap.mid(menuKeyMap.length()/2,1)).toString();
    defaultSelection = menuKeyMap.indexOf(d);
    setCurrentItem(defaultSelection);

    // just to get help for the main menu
    (void)QSoftMenuBar::menuFor(this);

    qLog(UI) << "end PhoneMainMenu create";

    cfg.endGroup();
}

QContent *PhoneMainMenu::readLauncherMenuItem(const QString &entry)
{
    QContent *applnk = 0;

    if (entry.right(8)==".desktop") {
        // There used to be a quick way to locate a .desktop file
        // Now we have to create a QContentSet and iterate over the items

        // The path to the apps folder (which only exists in the database)
        QString apps = Qtopia::qtopiaDir()+"apps/";
        // We need the full path to the entry to compare against the items we get from QContentSet
        QString entryPath = apps+entry;
        applnk = new QContent( entryPath, false );
        if ( applnk->id() == QContent::InvalidId ) {
            delete applnk;
            applnk = 0;
        }
    } else if (entry == "Documents") { // No tr
        applnk = new QContent();
        applnk->setName(tr("Documents"));
        applnk->setType("Folder/Documents");
        applnk->setIcon("qpe/DocsIcon");
    } else {
        QCategoryManager catman("Applications");
        if(catman.contains(entry))
        {
            applnk = new QContent();
            applnk->setName(catman.label(entry));
            applnk->setIcon(catman.iconFile(entry));
            applnk->setType("Folder/"+entry);
        }
        else
            applnk = NULL;
    }

    return applnk;
}

bool PhoneMainMenu::Item::exprTrue()
{
    return !expr || (expr->evaluate() && expr->result().toBool());
}

void PhoneMainMenu::activateItem(const QChar &c, int idx)
{
    Items &items = mainMenuEntries[c];
    Q_ASSERT(items.count() > idx && idx >= 0);
    if(items.activeItem != idx) {
        items.activeItem = idx;
        qLog(UI) << "PhoneMainMenu: Activating"
                 << items.at(idx).lnk.name()
                 << "for key" << c;
        addItem(new QContent(items.at(idx).lnk),
                menuKeyMap.indexOf(c));
    }
}

void PhoneMainMenu::expressionChanged()
{
    Q_ASSERT(sender());
    ItemExpression *e = static_cast<ItemExpression *>(sender());

    Items &items = mainMenuEntries[e->character()];
    for(int ii = 0; ii < items.count(); ++ii) {
        Q_ASSERT(ii == items.activeItem || items.at(ii).expr);

        if(ii == items.activeItem && e == items.at(ii).expr) {
            // Hmmm... we're the one that changed.  If we're still true, there's
            // nothing to do, otherwise we need to find the next valid guy or
            // the last if none are true
            bool valid = items[ii].exprTrue();
            if(!valid) {
                for(int jj = ii + 1; jj < items.count(); ++jj) {
                    if(items[jj].exprTrue()) {
                        activateItem(e->character(), jj);
                        return;
                    }
                }
                activateItem(e->character(), items.count() - 1);
            }
            return;

        } else if(ii == items.activeItem) {
            // The changed item must be behind the active item, so there's
            // nothing todo
            return;
        } else if(e == items.at(ii).expr) {
            // We've found the expression that changed and it is infront of the
            // active item.  If it is true, then we should set it as active,
            // otherwise do nothing
            if(e->evaluate() && e->result().toBool()) {
                // Activate this guy
                activateItem(e->character(), ii);
            }
            return;
        }
    }
    Q_ASSERT(!"Invalid code path");
}

void PhoneMainMenu::setMainMenuItemEnabled(const QString &file, const QString &name, const QString &icon, bool enabled)
{
    QMap<QChar,Items>::Iterator it;
    for (it = mainMenuEntries.begin(); it != mainMenuEntries.end(); ++it) {
        Items &itemList = *it;
        bool found = false;
        bool change = false;
        Item *enabledItem = 0;
        for (int i=0; i < (int)itemList.count(); i++) {
            Item &item = itemList[i];
            if (!found && item.lnk.executableName() == file) {
                item.enabled = enabled;
                if (!enabledItem && enabled)
                    change = true;
                found = true;
            }
            if (!enabledItem && item.enabled)
                enabledItem = &item;
        }
        if (found) {
            if (!enabledItem && itemList.count())
                enabledItem = &itemList.last();
            if (enabledItem) {
                QContent *appLnk = new QContent(enabledItem->lnk);
                if (change) {
                    if (!name.isEmpty())
                        appLnk->setName(name);
                    if (!icon.isEmpty())
                        appLnk->setIcon(icon);
                }
                addItem(appLnk, menuKeyMap.indexOf(it.key()));
            }
            break;
        }
    }
}

QString PhoneBrowserStack::currentName() const
{
    LauncherView *v = qobject_cast<LauncherView *>(stack->currentWidget());
    for (QMap<QString,TypeView>::ConstIterator it = map.begin();
            it != map.end(); it++) {
        if ( (*it).view == v )
            return (*it).name;
    }
    return QString();
}


/*!
    \class GridBrowserScreen
    \inpublicgroup QtBaseModule
    \brief The GridBrowserScreen class provides the main launcher grid for Qt Extended Phone.
    \ingroup QtopiaServer::PhoneUI

    An image of this browser screen can be found in the \l {Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractServerInterface, QAbstractBrowserScreen

*/

/*!
    Constructs a new GridBrowserScreen instance with the specified \a parent
    and widget \a flags
*/
GridBrowserScreen::GridBrowserScreen(QWidget *parent, Qt::WFlags flags)
: QAbstractBrowserScreen(parent, flags), m_stack(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);
    m_stack = new PhoneBrowserStack(this);
    QObject::connect(m_stack, SIGNAL(applicationLaunched(QString)), this,
                     SIGNAL(applicationLaunched(QString)));
    layout->addWidget(m_stack);
    setFocusPolicy(Qt::NoFocus);
//    setFocusProxy(m_stack);
}

/*!
  \reimp
  */
QString GridBrowserScreen::currentView() const
{
    return m_stack->currentName();
}

/*!
  \reimp
  */
bool GridBrowserScreen::viewAvailable(const QString &) const
{
    return true;
}

/*!
  \reimp
  */
void GridBrowserScreen::resetToView(const QString &view)
{
    m_stack->resetToView(view);
}

/*!
  \reimp
  */
void GridBrowserScreen::moveToView(const QString &view)
{
    m_stack->showView(view);
}
/*!
  \internal
  */
void GridBrowserScreen::closeEvent(QCloseEvent *e)
{
    m_stack->back();

    if(m_stack->isDone()) {
        e->accept();
    } else {
        e->ignore();
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractBrowserScreen, GridBrowserScreen);

