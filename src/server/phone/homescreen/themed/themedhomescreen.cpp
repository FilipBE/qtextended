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

#include "messageboard.h"
#include "themedhomescreen.h"
#include "serverthemeview.h"
#include "themecontrol.h"
#include "themebackground_p.h"
#include "uifactory.h"
#include <themedview.h>
#include <qvaluespace.h>
#include "qtopiainputevents.h"
#include "qtopiaserverapplication.h"

#include <QtopiaServiceDescription>
#include <QtopiaServiceRequest>
#include <QtopiaIpcEnvelope>
#include <QTimer>
#include <QDialog>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <custom.h>
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#include <qexportedbackground.h>
#endif
#include <QPainter>

/*!
    \class ThemedHomeScreen
    \inpublicgroup QtUiModule
    \brief The ThemedHomeScreen class provides the home screen for Qt Extended.
    \ingroup QtopiaServer::PhoneUI

    This class extends the BasicHomeScreen by adding a \l {Theming}{themeable} user interface.
    An image of this home screen using the Qt Extended theme can be found in the
    \l{Server Widget Classes}{server widget gallery}.

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractHomeScreen, BasicHomeScreen
*/

/*!
  Constructs a new ThemedHomeScreen instance with the specified \a parent
  and widget \a flags
  */
ThemedHomeScreen::ThemedHomeScreen(QWidget *parent, Qt::WFlags flags)
    : BasicHomeScreen(parent, flags),
      themedView(new PhoneThemedView(parent, flags))
{
    // Try to avoid double layouting.
    if (parent)
        setGeometry(0, 0, parent->width(), parent->height());

    vsObject = new QValueSpaceObject("/UI/HomeScreen", this);

    connect(themedView, SIGNAL(itemClicked(ThemeItem*)),
            this, SLOT(themeItemClicked(ThemeItem*)));

    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addWidget(themedView);
    vbox->activate(); // Avoid double layouting.

    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if ( ctrl )
        ctrl->registerThemedView( themedView, "Home");
    else 
        qLog(Component) << "ThemedHomeScreen: ThemeControl not available, Theme will not work properly";

    board = qtopiaTask<MessageBoard>();
    if ( !board ) {
        qLog(Component) << "ThemedHomeScreen: MessageBoard component not available";
    } else {
        QObject::connect(board, SIGNAL(boardUpdated()), this, SLOT(updateInformation()));
    }

    // Listen to system channel
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(sysMessage(QString,QByteArray)) );
    connect( this, SIGNAL(showLockDisplay(bool,QString,QString)),
             this, SLOT(showPinboxInformation(bool,QString,QString)));

    QTimer::singleShot(0, this, SLOT(updateHomeScreenImage()));
    QTimer::singleShot(0, this, SLOT(updateBackground()));

    if (ctrl)
      QObject::connect(ctrl, SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    themeLoaded();
}

/*!
  \internal
 */
ThemedHomeScreen::~ThemedHomeScreen()
{
    delete themedView;
}

/*!
  \internal
 */
void ThemedHomeScreen::updateBackground()
{
    ThemePluginItem *background = (ThemePluginItem *)themedView->findItem("background-plugin", ThemedView::Plugin);
    if (background) {
        ThemeBackground *themeBackground = new ThemeBackground(themedView);
        background->setBuiltin(themeBackground);
    }
}

/*!
  \internal
 */
void ThemedHomeScreen::exportBackground()
{
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    ThemePluginItem *background = (ThemePluginItem *)themedView->findItem("background-plugin", ThemedView::Plugin);
    if (background) {
        int screen = QApplication::desktop()->screenNumber(this);
        QDesktopWidget *desktop = QApplication::desktop();
        QSize desktopSize = QExportedBackground::exportedBackgroundSize(screen);
        // Create a 16bpp pixmap is possible
        QImage::Format fmt = desktop->depth() <= 16 ? QImage::Format_RGB16 : QImage::Format_ARGB32_Premultiplied;
        QImage img(desktopSize.width(), desktopSize.height(), fmt);
        QPixmap pm = QPixmap::fromImage(img);
        QPainter p(&pm);
        QRect rect(QPoint(0,0), desktopSize);
        background->paint(&p, rect);
        QExportedBackground::setExportedBackground(pm, screen);
        QExportedBackground::polishWindows(screen);
    }
#endif
}

void ThemedHomeScreen::themeChanged()
{
    QTimer::singleShot(0, this, SLOT(show()));
    QTimer::singleShot(0, this, SLOT(updateBackground()));
    QTimer::singleShot(0, this, SLOT(exportBackground()));
    QTimer::singleShot(0, this, SLOT(updateHomeScreenImage()));
}

/*!
  \internal
 */
void ThemedHomeScreen::updateHomeScreenImage()
{
    ThemePluginItem *ip = (ThemePluginItem *)themedView->findItem("homescreen-image-plugin", ThemedView::Plugin);
    if (ip) {
        QSettings cfg("Trolltech", "Launcher");
        cfg.beginGroup("HomeScreen");
        QString pname = cfg.value("Plugin", "Background").toString();
        if (pname == "Background") {
            ThemedItemPlugin *bgIface = new HomeScreenImagePlugin(themedView);
            ip->setBuiltin(bgIface);
        } else {
            ip->setPlugin(pname);
        }
    }
}

void ThemedHomeScreen::sysMessage(const QString& message, const QByteArray &data)
{
    QDataStream stream(data);
    if (message == "updateHomeScreenImage()") {
        updateHomeScreenImage();
    } else if (message == "updateBackground()") {
        updateBackground();
    } else if (message == "exportBackground()") {
        exportBackground();
    } else if (message == "updateHomeScreenInfo()") {
        updateHomeScreenInfo();
    }
}
/*!
  \internal
 */
void ThemedHomeScreen::themeLoaded()
{
    ThemeTextItem *textItem = (ThemeTextItem *)themedView->findItem("infobox-text", ThemedView::Text);
    if (textItem)
        textItem->setTextFormat(Qt::RichText);
    updateBackground();
    exportBackground();
    updateHomeScreenInfo();
    updateHomeScreenImage();
}

/*!
  \internal
 */
void ThemedHomeScreen::updateHomeScreenInfo()
{
    if (!vsObject)
        return;

    QSettings config("Trolltech", "qpe");
    config.beginGroup("HomeScreen");
    vsObject->setAttribute("ShowOperator", config.value("ShowOperator", "true").toBool());
    vsObject->setAttribute("ShowProfile", config.value("ShowProfile", "true").toBool());
    vsObject->setAttribute("ShowDate", config.value("ShowDate", "true").toBool());
    vsObject->setAttribute("ShowTime", config.value("ShowTime", "true").toBool());
    vsObject->setAttribute("ShowLocation", config.value("ShowLocation", "true").toBool());
}

/*!
  \internal
 */
void ThemedHomeScreen::updateInformation()
{
    if ( !board )
        return;

    bool hideInfo = true;
    static int currentId = -1;
    ThemeTextItem *textItem;
    if (!board->isEmpty()) {
        MessageBoard::Note note = board->message();
        if (currentId != note.id) {
            textItem = (ThemeTextItem *)themedView->findItem("infobox-text", ThemedView::Text);
            if (textItem)
                textItem->setText(note.text);
            ThemeImageItem *imgItem = (ThemeImageItem *)themedView->findItem("infobox-image", ThemedView::Image);
            if (imgItem)
                imgItem->setImage(QPixmap(note.pixmap));
            hideInfo = note.text.isEmpty() && note.pixmap.isEmpty();
            currentId = note.id;
        } else {
            hideInfo = false;
        }
    } else {
        currentId = -1;
    }

    ThemeItem *item = themedView->findItem("infobox", ThemedView::Item);
    if (item)
        item->setActive(!hideInfo);
}

/*!
  \internal
 */
void ThemedHomeScreen::showPinboxInformation(bool enable, const QString &pix, const QString &text)
{
    ThemeTextItem *textItem = (ThemeTextItem*)themedView->findItem("pinbox", ThemedView::Text);
    if (textItem)
        textItem->setText(text);
    ThemeImageItem *imgItem = (ThemeImageItem*)themedView->findItem("pinbox", ThemedView::Image);
    if (imgItem) {
        imgItem->setImage(QPixmap(pix));
    }
    ThemeItem *item = themedView->findItem("pinbox", ThemedView::Layout);
    if (item)
        item->setActive(enable);
}

struct PhoneKeyDescription
{
    const char *name;
    int key;
};

PhoneKeyDescription keyMap [] = {
    {"zero", Qt::Key_0},
    {"one", Qt::Key_1},
    {"two", Qt::Key_2},
    {"three", Qt::Key_3},
    {"four", Qt::Key_4},
    {"five", Qt::Key_5},
    {"six", Qt::Key_6},
    {"seven", Qt::Key_7},
    {"eight", Qt::Key_8},
    {"nine", Qt::Key_9},
    {"hash", Qt::Key_NumberSign},
    {"star", Qt::Key_Asterisk},
    {0, 0}
};

/*!
  \internal
 */
void ThemedHomeScreen::themeItemClicked(ThemeItem *item)
{
    if (!item)
        return;

    if (!item->isInteractive())
        return;

    QString in = item->itemName();
#ifdef QTOPIA_CELL
    {
        int i = 0;
        while (keyMap[i].name != 0) {
            if (in == keyMap[i].name) {
                if (!emLock->processKeyEvent(new QKeyEvent(QEvent::KeyPress, keyMap[i].key, Qt::NoModifier)))
                    simLock->processKeyEvent(new QKeyEvent(QEvent::KeyPress, keyMap[i].key, Qt::NoModifier));
                break;
            }
            ++i;
        }
    }
    if (in == "star")
        QtopiaInputEvents::processKeyEvent('*', Qt::Key_Asterisk, Qt::NoModifier, true, false);
    if (keyLock->locked() || simLock->locked())
        return;
#endif
    if (in == "LauncherHSWidget") {
        ThemeWidgetItem* wItem = static_cast<ThemeWidgetItem*>(item);
        if (wItem != 0) {
            QWidget* launcher = wItem->widget();
            if ( launcher )
                QMetaObject::invokeMethod( launcher, "launch", Qt::DirectConnection );
        }
    } else if (in == "WorldmapHSWidget") {
        ThemeWidgetItem* wItem = static_cast<ThemeWidgetItem*>(item);
        if (wItem != 0) {
            QWidget *worldmap = wItem->widget();
            if (worldmap)
                QMetaObject::invokeMethod( worldmap, "showCity", Qt::DirectConnection );
        }
    } else if (in == "lock") {
        setLocked(true);
    } else if (in == "mainmenu") {
        emit showPhoneBrowser();
    } else if (in == "speeddial") {
        emit speedDial(QString());
    } else if (in == "favorites") {
        QtopiaServiceRequest e( "Favorites", "select()" );
        e.send();
    } else if (in == "contacts") {
        QtopiaIpcEnvelope e("QPE/Application/addressbook", "raise()");
    } else if( in == "dialer" ) {
        QtopiaServiceRequest e( "Dialer", "showDialer(QString)" );
        e << QString();
        e.send();
    } else if( in == "callhistory" ) {
        QtopiaServiceRequest e( "CallHistory", "showCallHistory(QCallList::ListType,QString)");
        e << 0; /*QCallList::All;*/
        e << QString();
        e.send();
    } else if( in == "caption" ) {
        QtopiaServiceRequest e( "Presence", "editPresence()" );
        e.send();
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractHomeScreen, ThemedHomeScreen);
