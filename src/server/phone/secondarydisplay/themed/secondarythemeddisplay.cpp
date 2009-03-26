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

#include "secondarythemeddisplay.h"
#include "serverthemeview.h"
#include "qtopiaserverapplication.h"
#include "themecontrol.h"
#include "windowmanagement.h"
#include "themebackground_p.h"
#include <custom.h>
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
#include <qexportedbackground.h>
#endif
#include <QVBoxLayout>
#include <QDesktopWidget>
#include <QPainter>
#include <QtopiaChannel>


class SecondaryHomeScreen : public PhoneThemedView
{
    Q_OBJECT
public:
    SecondaryHomeScreen(QWidget *parent=0, Qt::WFlags f=0);
    ~SecondaryHomeScreen();

public slots:
    void updateBackground();
    void updateHomeScreenImage();
    void exportBackground();

protected:
    void themeLoaded(const QString &);

private:
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    QExportedBackground *exportedBg;
#endif
};

SecondaryHomeScreen::SecondaryHomeScreen(QWidget *parent, Qt::WFlags f)
    : PhoneThemedView(parent, f)
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    , exportedBg(0)
#endif
{
    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if ( ctrl )
        ctrl->registerThemedView( this, "SecondaryHome");
    else 
        qLog(Component) << "SecondaryHomeScreen: ThemeControl not available, Theme will not work properly";

#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    int screen = QApplication::desktop()->screenNumber(this);
    QSize desktopSize = QExportedBackground::exportedBackgroundSize();
    QExportedBackground::initExportedBackground(desktopSize.width(),
                                                desktopSize.height(),
                                                screen);

    if (ctrl && ctrl->exportBackground())
        exportedBg = new QExportedBackground(1, this);
#endif

    if (ctrl)
        connect(ctrl, SIGNAL(themeChanged()), this, SLOT(updateBackground()));
    updateBackground();
    exportBackground();
    updateHomeScreenImage();
}

SecondaryHomeScreen::~SecondaryHomeScreen()
{
}

void SecondaryHomeScreen::updateHomeScreenImage()
{
    ThemePluginItem *ip = (ThemePluginItem *)findItem("homescreen-image-plugin", Plugin);
    if (ip) {
        QSettings cfg("Trolltech","Launcher");
        cfg.beginGroup("SecondaryHomeScreen");
        QString pname = cfg.value("Plugin", "Background").toString();
        if (pname == "Background") {
            // DON'T delete the existing object here - it causes it to crash. Something else
            // must be deleting it, "ip" presumably <sigh>.
            ThemedItemPlugin *bgIface = new HomeScreenImagePlugin(this);
            ip->setBuiltin(bgIface);
        } else {
            ip->setPlugin(pname);
        }
    }
}

void SecondaryHomeScreen::updateBackground()
{
    ThemePluginItem *background = (ThemePluginItem *)findItem("background-plugin", ThemedView::Plugin);
    if (background) {
        ThemeBackground *themeBackground = new ThemeBackground(this);
        background->setBuiltin(themeBackground);
    }
}

/*!
  \internal
 */
void SecondaryHomeScreen::exportBackground()
{
#ifdef QTOPIA_ENABLE_EXPORTED_BACKGROUNDS
    ThemePluginItem *background = (ThemePluginItem *)findItem("background-plugin", ThemedView::Plugin);
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

void SecondaryHomeScreen::themeLoaded(const QString&)
{
    ThemeTextItem *textItem = (ThemeTextItem *)findItem("infobox", Text);
    if (textItem)
        textItem->setTextFormat(Qt::RichText);
}

//===========================================================================

class SecondaryTitle : public PhoneThemedView
{
public:
    SecondaryTitle(QWidget *parent=0, Qt::WFlags f=0);
};

SecondaryTitle::SecondaryTitle(QWidget *parent, Qt::WFlags f)
    : PhoneThemedView(parent, f)
{
    ThemeControl *ctrl = qtopiaTask<ThemeControl>();
    if ( ctrl )
        ctrl->registerThemedView(this, "SecondaryTitle");
    else 
        qLog(Component) << "SecondaryTitle: ThemeControl not available, Theme will not work properly";

    WindowManagement::dockWindow(this, WindowManagement::Top, 1);
}

//===========================================================================

/*!
    \class ThemedSecondaryDisplay
    \inpublicgroup QtUiModule
    \brief The ThemedSecondaryDisplay class implements the secondary display for Qt Extended Phone.
    \ingroup QtopiaServer::PhoneUI

    This widget is \l{Theming}{themeable}.

    To integration purposes any secondary display should support the following features:

    \list
    \o The secondary display should listen on the \c {QPE/System} qcop channel for
    \c applySecondaryBackgroundImage() messages.
    \endlist

    This class is a Qt Extended \l{QtopiaServerApplication#qt-extended-server-widgets}{server widget}.
    It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa QAbstractServerInterface, QAbstractSecondaryDisplay
*/

/*!
  Constructs a ThemedSecondaryDisplay instance with the specified
  \a parent and widget flags \a f.
  */
ThemedSecondaryDisplay::ThemedSecondaryDisplay(QWidget *parent, Qt::WFlags f)
    : QAbstractSecondaryDisplay(parent, f)
{
    title = new SecondaryTitle(0, Qt::FramelessWindowHint
                                    | Qt::Tool
                                    | Qt::WindowStaysOnTopHint);
    title->show();

    QVBoxLayout *vbox = new QVBoxLayout;

    // ensure home screen image stretches to edges of screen
    vbox->setContentsMargins(0, 0, 0, 0);

    setLayout(vbox);
    home = new SecondaryHomeScreen;

    vbox->addWidget(home);
    
    // Listen to system channel
    QtopiaChannel* sysChannel = new QtopiaChannel( "QPE/System", this );
    connect( sysChannel, SIGNAL(received(QString,QByteArray)),
             this, SLOT(sysMessage(QString,QByteArray)) );
}

/*!
  \internal
  */
void ThemedSecondaryDisplay::sysMessage(const QString& message, const QByteArray &data)
{
    QDataStream stream(data);
    if (message == "updateHomeScreenImage()") {
        home->updateHomeScreenImage();
    } else if (message == "updateBackground()") {
        home->updateBackground();
    } else if (message == "exportBackground()") {
        home->exportBackground();
    }
}

QTOPIA_REPLACE_WIDGET(QAbstractSecondaryDisplay, ThemedSecondaryDisplay);

#include "secondarythemeddisplay.moc"
