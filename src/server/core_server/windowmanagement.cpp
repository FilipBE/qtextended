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

#include "windowmanagement.h"
#include <QSet>
#include "qtopiaserverapplication.h"
#include <QWidget>
#include <qwindowsystem_qws.h>
#include <QApplication>
#include <QDesktopWidget>
#include <qscreen_qws.h>
#include <QRect>
#include <QSize>
#include <QTimer>
#include <QVector>
#include <QDebug>
#include <qvaluespace.h>
#include <qtopialog.h>

#include <QWSDisplay>

// declare ServerLayoutManager
class ServerLayoutManager : public QObject
{
    Q_OBJECT
  public:
    ServerLayoutManager();

    void addDocked(QWidget* w,
		   WindowManagement::DockArea placement,
		   const QSize &s,
		   int screen);

    void showDockedWindow(QWidget *w);
    void hideDockedWindow(QWidget *w);

  protected:
    virtual void customEvent(QEvent *);

  private slots:
    void removeWidget();
    void updateGeometries(int screen=-1);
    void desktopResized(int);

  private:
    struct Item {
        QWidget* w;
        WindowManagement::DockArea p;
        QSize fixed;
        int screen;
        QRect geometry;
        bool hiding;
    };

    bool eventFilter(QObject* object, QEvent* event);
    void setWidgetGeometry(QWidget *w, const QRect &r);
    void dolayout(int screen, bool deferUpdate=false);
    void layout(int screen);
    Item* findWidget(const QWidget* w) const;

    QVector<QList<Item*> > docked;
    QVector<QRect> availableGeom;
};

// define ServerLayoutManager
ServerLayoutManager::ServerLayoutManager()
    : docked(QApplication::desktop()->numScreens())
{
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, SIGNAL(resized(int)), this, SLOT(desktopResized(int)));
    availableGeom.resize(desktop->numScreens());
    for (int screen=0; screen < desktop->numScreens(); ++screen)
        availableGeom[screen] = desktop->availableGeometry(screen);
    desktop->installEventFilter(this);
}

void ServerLayoutManager::addDocked(QWidget* w,
				    WindowManagement::DockArea placement,
				    const QSize& s,
				    int screen)
{
    if (screen == -1)
        screen = QApplication::desktop()->primaryScreen();

    Item *oldItem = findWidget(w);
    if (oldItem) {
        docked[screen].removeAll(oldItem);
        delete oldItem;
    }

    Item *i = new Item;
    i->w = w;
    i->p = placement;
    i->fixed = s;
    i->screen = screen;
    i->hiding = false;
    if (!oldItem) {
        w->installEventFilter(this);
        connect(w, SIGNAL(destroyed()), this, SLOT(removeWidget()));
    }
    docked[screen].append(i);
    dolayout(screen);
}

void ServerLayoutManager::showDockedWindow(QWidget *w)
{
    Item *item = findWidget(w);
    if (item) {
        bool wasHiding = item->hiding;
        item->hiding = false;
        w->show();
        if (wasHiding)
            layout(item->screen);
    }
}

void ServerLayoutManager::hideDockedWindow(QWidget *w)
{
    Item *item = findWidget(w);
    if (item) {
        item->hiding = true;
        layout(item->screen);
    }
}

void ServerLayoutManager::removeWidget()
{
    QWidget *w = (QWidget*)sender();
    Item *item = findWidget(w);
    if (item) {
        int screen = item->screen;
        docked[screen].removeAll(item);
        delete item;
        layout(screen);
    }
}

bool ServerLayoutManager::eventFilter(QObject *object, QEvent *event)
{
    if (object == qApp->desktop()) {
        if (event->type() == QEvent::Resize) {
            QDesktopWidget *desktop = QApplication::desktop();
            for (int screen=0; screen < desktop->numScreens(); ++screen) {
                if (availableGeom[screen] != desktop->availableGeometry(screen)) {
                    layout(screen);
                }
            }
        }
        return QObject::eventFilter(object, event);
    }

    Item *item;

    switch (event->type()) {
        case QEvent::Hide:
            item = findWidget((QWidget *)object);
            if (item) {
                if (item->hiding)
                    item->hiding = false;
                else
                    layout(item->screen);
            }
            break;
        case QEvent::Show:
            item = findWidget((QWidget *)object);
            if (item)
                layout(item->screen);
            break;

        default:
            break;
    }

    return QObject::eventFilter(object, event);
}

class LayoutEvent : public QEvent
{
  public:
    LayoutEvent(int screen)
	: QEvent(QEvent::User), _screen(screen) { }

    int screen() const { return _screen; }

  private:
    int _screen;
};

void ServerLayoutManager::customEvent(QEvent* e)
{
    if (e->type() == QEvent::User) {
        dolayout(static_cast<LayoutEvent *>(e)->screen());
    } 
}

void ServerLayoutManager::layout(int screen)
{
    QEvent* e = new LayoutEvent(screen);
    QCoreApplication::postEvent(this, e);
}

void ServerLayoutManager::setWidgetGeometry(QWidget *w, const QRect &r)
{
    if (w->size() != r.size() || w->geometry().topLeft() != r.topLeft()) {
        w->setGeometry(r.left(), r.top(), r.width(), r.height());
    }
}

void ServerLayoutManager::updateGeometries(int screen)
{
    if (screen < 0) {
        for (int i=0; i < QApplication::desktop()->numScreens(); ++i)
            updateGeometries(i);
        return;
    }
    foreach (Item *item, docked[screen]) {
        if (item->hiding) {
            if (item->w->isVisible()) {
                item->w->hide();
            }
        } else if (item->w->isVisible()) {
            setWidgetGeometry(item->w, item->geometry);
        }
    }
}

void ServerLayoutManager::dolayout(int screen, bool deferUpdate)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect oldMwr(desktop->availableGeometry(screen));
    QRect mwr(desktop->screenGeometry(screen));
    qLog(UI) << "Layout screen docking" << screen;
    foreach (Item *item, docked[screen])
        item->geometry = item->w->geometry();

    foreach (Item *item, docked[screen]) {
        QWidget *w = item->w;
        QSize sh = w->sizeHint();
        QSize fs = item->fixed.isValid() ? item->fixed : sh;
        switch (item->p) {
            case WindowManagement::Top:
                item->geometry = QRect(mwr.left(), mwr.top(),
                                        mwr.width(), fs.height());
                if (w->isVisible() && !item->hiding)
                    mwr.setTop(mwr.top() + fs.height());
                break;
            case WindowManagement::Bottom:
                item->geometry = QRect(mwr.left(), mwr.bottom()-fs.height()+1,
                                        mwr.width(), fs.height());
                if (w->isVisible() && !item->hiding)
                    mwr.setBottom(mwr.bottom()-fs.height());
                break;
            case WindowManagement::Left:
                item->geometry = QRect(mwr.left(), mwr.top(),
                                        fs.width(), mwr.height());
                if (w->isVisible() && !item->hiding)
                    mwr.setLeft(item->geometry.right() + 1);
                break;
            case WindowManagement::Right:
                item->geometry = QRect(mwr.right()-fs.width()+1, mwr.top(),
                                        fs.width(), mwr.height());
                if (w->isVisible() && !item->hiding)
                    mwr.setRight(item->geometry.left() - 1);
                break;
        }
    }

    if (mwr != oldMwr) {
        qLog(UI) << " set max window rect for screen" << screen << mwr;
        availableGeom[screen] = mwr;
        QWSServer::setMaxWindowRect(mwr);
    }

    if (oldMwr.intersected(mwr) == mwr && !deferUpdate) {
        updateGeometries(screen);
    } else {
        // Would be better to wait until we know the active window has
        // resized and then updateGeometries.
        QTimer::singleShot(100, this, SLOT(updateGeometries()));
    }
}

/*
    Called after dynamic screen rotation.
*/
void ServerLayoutManager::desktopResized(int s)
{
    dolayout(s, true);

    QWidgetList list = QApplication::topLevelWidgets();
    foreach (QWidget* widget, list) {
        if ((widget->inherits("QAbstractServerInterface") && s == 0) ||
            (widget->inherits("QAbstractSecondaryDisplay") && s == 1)) {
                widget->setGeometry( QApplication::desktop()->screenGeometry(s) );
                break;
        }
    }
}

ServerLayoutManager::Item*
ServerLayoutManager::findWidget(const QWidget* w) const
{
    foreach (QList<Item*> screenItems, docked) {
        foreach (Item *item, screenItems) {
            if (item->w == w)
                return item;
        }
    }

    return 0;
}

//===========================================================================
// declare WindowManagementPrivate
class WindowManagementPrivate : public QObject
{
    Q_OBJECT
  public:
    WindowManagementPrivate(QObject * = 0);
    void protectWindow(QWidget *);
    const QString& activeAppName() { return m_activeAppName; }

    ServerLayoutManager *serverLayoutManager() {
        static ServerLayoutManager *lm = 0;
        if (!lm)
            lm = new ServerLayoutManager;
        return lm;
    }
    
    signals:
    void windowActive(const QString &, const QRect &, WId);
    void windowCaption(const QString &);

  private slots:
    void windowEvent(QWSWindow *, QWSServer::WindowEvent);
    void destroyed(QObject *);

  private:
    void doWindowActive(const QString &, const QRect &, QWSWindow *);

  private:
    QString		m_activeAppName;
    QValueSpaceObject 	m_wmValueSpace;
    QSet<QWidget*> 	m_widgets;
};
extern QWSServer *qwsServer;
Q_GLOBAL_STATIC(WindowManagementPrivate, windowManagement);

// define WindowManagementPrivate
WindowManagementPrivate::WindowManagementPrivate(QObject *parent)
    : QObject(parent),
      m_activeAppName(""),
      m_wmValueSpace("/UI/ActiveWindow")
{
    Q_ASSERT(qwsServer);
    connect(qwsServer, SIGNAL(windowEvent(QWSWindow*,QWSServer::WindowEvent)),
            this, SLOT(windowEvent(QWSWindow*,QWSServer::WindowEvent)));
}

void WindowManagementPrivate::protectWindow(QWidget *wid)
{
    if (wid->isVisible())
        wid->raise();

    if (!m_widgets.contains(wid)) {
        m_widgets.insert(wid);
        QObject::connect(wid, SIGNAL(destroyed(QObject*)),
                         this, SLOT(destroyed(QObject*)));
    }
}

void WindowManagementPrivate::windowEvent(QWSWindow* w,
                                          QWSServer::WindowEvent e)
{
    if (!w)
        return;
    static int active = 0;

    bool known = false;

    /*
      only calculate known for these events, and only for created
      QWSWindows (otherwise QWidget::winId() creates them)
    */
    switch (e) {
        case QWSServer::Raise:
        case QWSServer::Show:
        case QWSServer::Active:
        case QWSServer::Name:
            for (QSet<QWidget *>::ConstIterator iter = m_widgets.begin();
                    !known && m_widgets.end() != iter;
                    ++iter)
                known = ((*iter)->testAttribute(Qt::WA_WState_Created)) &&
		    (((*iter)->winId() == (unsigned)w->winId()));
            break;
        default:
	    break;
    }

    switch(e) {
        case QWSServer::Raise:
            if (!w->isVisible() || w->requestedRegion().isEmpty())
                break;
            // else FALL THROUGH
        case QWSServer::Show:
            {
                QWidget *widget = QWidget::find( w->winId() );
                static bool fullscreen = false;
                if ( widget && ( widget->windowState() & Qt::WindowFullScreen ) )
                    fullscreen = true;
#ifdef QTOPIA_HOMEUI
                bool isServer = widget && widget->inherits("QAbstractServerInterface");
#endif

                /*
                 Check for "reserved" caption is added to allow
                 full screen QDirectPainter, since we can't set "_allow_on_top_" to it.
                */
                if ( !known && !fullscreen
                        && w->caption() != QLatin1String("_allow_on_top_")
                        && w->caption() != QLatin1String("reserved")
                     ) {
                    QRect req = w->requestedRegion().boundingRect();
                    QSize s(qt_screen->deviceWidth(),
                            qt_screen->deviceHeight());
                    req = qt_screen->mapFromDevice(req, s);

                    for(QSet<QWidget *>::ConstIterator iter = m_widgets.begin();
                            !known && m_widgets.end() != iter;
                            ++iter) {
#ifndef QTOPIA_HOMEUI
                        // Keep known windows on top.
                        if ((*iter)->isVisible() &&
                                req.intersects((*iter)->geometry())) {
                            QTimer::singleShot(0, *iter, SLOT(raise()));
                        }
#else
                        // Qtopia Home ensures the context bar is visible
                        // only if the top window is not fullscreen or
                        // the top window is the server (homescreen).
                        QRect avail = QApplication::desktop()->availableGeometry();
                        if (w->caption() != QLatin1String("_ignore_")
                            && (isServer || ((*iter)->isVisible() &&
                                req == avail))) {
                            QTimer::singleShot(0, *iter, SLOT(raise()));
                        }
#endif
                    }
                }
            }
            // FALL THROUGH
        case QWSServer::Name:
        case QWSServer::Active:
            if (w->caption() == QLatin1String("_ignore_"))
                break;
            if (e == QWSServer::Active)
                active = w->winId();

            if (active == w->winId() && !known) {
                QRect req = w->requestedRegion().boundingRect();
                doWindowActive(w->caption(), req, w);
            }
            break;
        default:
            break;
    }
}

void WindowManagementPrivate::doWindowActive(const QString& caption,
                                             const QRect& rect,
                                             QWSWindow* win)
{
    static QString currCaption;
    static QRect currRect;
    static int currWinId = 0;

    m_activeAppName = win->client()->identity();
    if (m_activeAppName == "qpe") {
        foreach (QWidget *widget, QApplication::topLevelWidgets()) {
            if (widget->winId() == (WId)win->winId()) {
                m_wmValueSpace.setAttribute("ProcessName", widget->objectName());
            }
        }
    } else {
        m_wmValueSpace.setAttribute("ProcessName", m_activeAppName);
    }

    if (currCaption != caption || currRect != rect || currWinId != win->winId()) {
        emit windowActive(caption, rect, win->winId());
        currWinId = win->winId();
    }

    if (currCaption != caption || currRect != rect) {
        QRect sRect(QApplication::desktop()->screenGeometry());
        if(rect.top() <= sRect.height()/3 &&
                rect.bottom() > sRect.height()/2 &&
                rect.width() >= sRect.width()-4) {
            m_wmValueSpace.setAttribute("Caption", caption);
            QString iconName;
            QContentId cid = QContent::execToContent(m_activeAppName);
            if (cid != QContent::InvalidId) {
                QContent app(cid);
                iconName = QLatin1String(":icon/")+app.iconName();
            }
            m_wmValueSpace.setAttribute("Icon", iconName);
            emit windowCaption(caption);
        }
    }

    if (currRect != rect) {
        m_wmValueSpace.setAttribute("Rect", rect);
        currRect = rect;
    }

    if (currCaption != caption) {
        m_wmValueSpace.setAttribute("Title", caption);
        currCaption = caption;
    }
}

void WindowManagementPrivate::destroyed(QObject *obj)
{
    QWidget* wid = static_cast<QWidget *>(obj);
    m_widgets.remove(wid);
}

// define WindowManagement
/*!
  \class WindowManagement
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer
  \brief The WindowManagement class allows you to monitor and control the
         application windows in the system.

  The WindowManagement class allows server windows to be protected, preventing
  other windows from obscuring them, and docked to the edge of the screen.

  The WindowManagement class also updates the value space with the following
  keys that can be used to track window status:
  \table
  \header \o Key \o Description
  \row \o \c {/UI/ActiveWindow/Title} \o The title of the currently active window
  \row \o \c {/UI/ActiveWindow/Rect} \o The rectangle of the currently active window
  \row \o \c {/UI/ActiveWindow/Caption} \o The caption of the currently active window.  The caption is the same as the title except that it isn't changed for popup windows that only consume a small part of the screen.
  \endtable

  These value space keys are updated if either the \c {WindowManagement} task is
  running, or at least one instance of the WindowManagement class has been
  instantiated.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*!
  \fn void WindowManagement::windowActive(const QString &caption, const QRect &rect, WId window)

  Emitted whenever a \a window becomes active or the active window's \a caption
  changes.  \a rect is the rectangle covered by the window.
 */

/*!
  \enum WindowManagement::DockArea

  The DockArea enum type defines the areas where widgets can be docked:
  \value Top - the top of the screen.
  \value Bottom - the Bottom of the screen.
  \value Left - the Left of the screen.
  \value Right - the Right of the screen.
*/

/*!
  Construct a new WindowManagement instance with the specified \a parent.
 */
WindowManagement::WindowManagement(QObject *parent)
    : QObject(parent), d(windowManagement())
{
    if (d) {
        QObject::connect(d, SIGNAL(windowActive(QString,QRect,WId)),
                this, SIGNAL(windowActive(QString,QRect,WId)));
        QObject::connect(d, SIGNAL(windowCaption(QString)),
                this, SIGNAL(windowCaption(QString)));
    }
}

/*!
  \fn void WindowManagement::windowCaption(const QString &caption)

  Emitted whenever the active window \a caption changes.
  */

/*!
  \internal
  */
WindowManagement::~WindowManagement()
{
    // nothing.
}

/*!
  Prevent application windows from being raised above the
  provided toplevel \a window.  If an application window
  attempts to obscure any of the protected region, the
  protected window will immediately be raised above it.
 */
void WindowManagement::protectWindow(QWidget* window)
{
    Q_ASSERT(window->isWindow());
    WindowManagementPrivate* d = windowManagement();
    if (d)
        d->protectWindow(window);
}

/*!
  Docks a top-level widget \a window on a side of the \a screen specified by
  \a placement.  The widget is placed according to the order that it was
  docked, its sizeHint() and whether previously docked widgets are visible.
  The desktop area available to QWidget::showMaximized() will exclude any
  visible docked widgets.

  For example, if a widget is docked at the bottom of the screen, its sizeHint()
  will define its height and it will use the full width of the screen.  If a
  widget is then docked to the right, its sizeHint() will define its width and
  it will be as high as possible without covering the widget docked at the
  bottom.

  This function is useful for reserving system areas such as taskbars
  and input methods that should not be covered by applications.  Even after
  calling this method, an application can manually position itself over the
  docked window.  To prevent this, also call WindowManagement::protectWindow()
  on the window.
*/
void WindowManagement::dockWindow(QWidget* window,
				  DockArea placement,
				  int screen)
{
    dockWindow(window, placement, QSize(), screen);
}

/*!
  \overload

  Normally the QWidget::sizeHint() of the docked widget is used to determine
  its docked size.  If the sizeHint() is not correct, the \a size parameter can
  be used to override it.  The \a window, \a placement and \a screen parameters
  are used as above.
 */
void WindowManagement::dockWindow(QWidget* window,
				  DockArea placement,
                                  const QSize& size,
				  int screen)
{
    Q_ASSERT(window->isWindow());
    WindowManagementPrivate* d = windowManagement();
    if (d)
        d->serverLayoutManager()->addDocked(window, placement, size, screen);
}

/*!
  Shows the docked \a window.  While you can call QWidget::show() on the
  docked window, using this function may result in smoother screen
  updates.
 */
void WindowManagement::showDockedWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());
    WindowManagementPrivate* d = windowManagement();
    if (d)
        d->serverLayoutManager()->showDockedWindow(window);
}

/*!
  Hides the docked \a window.  While you can call QWidget::hide() on the
  docked window, using this function will result in smoother screen
  updates.
 */
void WindowManagement::hideDockedWindow(QWidget *window)
{
    Q_ASSERT(window->isWindow());
    WindowManagementPrivate* d = windowManagement();
    if (d)
        d->serverLayoutManager()->hideDockedWindow(window);
}

/*!
    Sets \a window as the lowest in the screen stacking order,
    below all normal and docked windows.  This is normally used
    only for the QAbstractServerInterface widget.
*/
void WindowManagement::setLowestWindow(QWidget *window)
{
    // The QWS window placement logic should guarantee that the
    // specified window is lowest without doing anything special.
    Q_UNUSED(window);
}

/*!
  Returns a reference to the name of the active application,
  ie the one that is visible. If there is no active application,
  the string is returned empty.
 */
QString WindowManagement::activeAppName()
{
    WindowManagementPrivate* d = windowManagement();
    if (d)
        return d->activeAppName();
    else
        return QString();
}

/*!
  Returns true if \a winId supports the Qt Extended soft menu bar via the
  QSoftMenuBar class; false otherwise.

  Under QWS, this function always returns true.  Under X11, it will
  return false if the application uses a widget toolkit
  other than that of Qtopia.  The soft menu bar changes its behavior so
  that the "back" button will send a close message to the foreign
  application using closeWindow().

  \sa closeWindow()
*/
bool WindowManagement::supportsSoftMenus(WId winId)
{
    Q_UNUSED(winId);
    return true;
}

/*!
 Sends a close message to the window \a winId.  This is used for
 applications that do not support the Qt Extended soft menu bar.

 \sa supportsSoftMenus()
*/
void WindowManagement::closeWindow(WId winId)
{
    Q_UNUSED(winId);
}

QTOPIA_STATIC_TASK(WindowManagement, windowManagement());
#include "windowmanagement.moc"
