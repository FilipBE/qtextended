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

#include <QWidget>
#include <QWSDisplay>
#include <QScreen>
#include <QValueSpaceObject>
#include <QApplication>
#include <QDesktopWidget>
#include <QtopiaApplication>

#ifdef Q_WS_QWS
#include <QWSDisplay>
#include <QWSServer>
#endif
#include <QTimer>

#include "qtopiarotationmanager.h"

/*!
  Constructs a RotationManager instance.
*/
RotationManager::RotationManager( QObject *parent )
    : QObject(parent),
      vso(0)
{
    qLog(Hardware) << __PRETTY_FUNCTION__;
    vso = new QValueSpaceObject( "/UI/Rotation", this );

    //initialize default rotation
    vso->setAttribute("Default", defaultOrientation() * 90);

    //initialize current rotation
    vso->setAttribute("Current", getCurrentOrientation() * 90 );

    (void)new RotationService( this, this );
 }

/*!
  \internal
*/
RotationManager::~RotationManager()
{
}

/*!
  \internal
*/
int RotationManager::getCurrentOrientation()
{
#ifdef Q_WS_QWS
    qLog(Hardware) << __PRETTY_FUNCTION__;
    QScreen *screen;
    screen = QScreen::instance();

    qWarning() << screen->transformOrientation();
    return screen->transformOrientation();
#else
    return 0;
#endif

}

/*!
  \internal
*/
int RotationManager::defaultOrientation()
{
#ifdef Q_WS_QWS
    qLog(Hardware) << __PRETTY_FUNCTION__;
    int deforientation;
    QString str;

    QStringList strlist = QString(getenv("QWS_DISPLAY")).split(":");
    for (int i = 0; i < strlist.size(); ++i) {
        str = strlist.at(i);
        if (str.contains("Rot")) {
                str = str.right(str.length() - 3);
                break;
        } else {
            str = "0";
        }
    }

    bool ok;
    deforientation = str.toInt(&ok);

    if (!ok)
        deforientation = 0;

    return deforientation;
#else
    return 0;
#endif
}


/*!
    This function sets the rotation for the rotation manager.
    It expects an int \a rotationDegree containing the rotation value

*/
void RotationManager::setRotation(int rotationDegree)
{

#ifdef Q_WS_QWS
    qLog(Hardware) << __PRETTY_FUNCTION__;

#ifndef QTOPIA_TEST
    // Disable painting updates temporarily so that the
    // user doesn't see the widget resize artifacts.
    if (qwsServer)
        qwsServer->enablePainting(false);
#endif

    QWSDisplay::setTransformation( rotationDegree / 90);
    vso->setAttribute("Current", rotationDegree );
    vso->sync();
    QTimer::singleShot(500, this, SLOT(restartPainting()));
#endif

}

/*!
    \fn void RotationManager::defaultRotation()

    Sets the rotation to the default configuration of the rotation manager.
    The default values are defined in the QWS_DISPLAY environmental variable.

    \sa {Qt for Embedded Linux Display Management}
*/

void RotationManager::defaultRotation()
{
    setRotation( defaultOrientation());
}

void RotationManager::restartPainting()
{
#ifdef Q_WS_QWS
#ifndef QTOPIA_TEST
    if (qwsServer) {
        QScreen *screen = qt_screen;
        if (screen->classId() == QScreen::MultiClass)
            screen = screen->subScreens()[0];
        screen->solidFill(Qt::black, screen->region());
        qwsServer->enablePainting(true);
        qwsServer->refresh();
    }
#endif
#endif
}


#ifndef QTOPIA_TEST
QTOPIA_TASK(RotationManager, RotationManager);
#endif


/*!
    \service RotationService RotationManager
    \inpublicgroup QtBaseModule
    \brief The RotationService class provides the RotationManager service.

    The \i RotationManager service enables applications to control the
    rotation of Qt Extended.

    Applications wishing to know the default and current orientation status
    should use the \c{QValueSpaceItem} /UI/Rotation/Default and /UI/Rotation/Current,
    respectively.

    The Transformed display driver has to be enabled to be able to use dynamic rotation.

    \sa {Qt for Embedded Linux Display Management}

*/

/*!
    \internal
*/
RotationService::RotationService(RotationManager *manager, QObject *parent)
    : QtopiaAbstractService( "RotationManager", parent )
{
    rotationManager = manager;
    publishAll();
}

/*!
    \internal
*/
RotationService::~RotationService()
{
}

/*!
    Set the current rotation to \a degree. The values can be 0, 90, 180, 270.

    This slot corresponds to the QCop service message
    \c{RotationManager::setCurrentRotation(int)}.
*/
void RotationService::setCurrentRotation(int degree)
{
    if ( degree == 0
         || degree == 90
         || degree == 180
         || degree == 270 ) {
        rotationManager->setRotation( degree);
    }
}

/*!
    Set the current rotation to the default value.

    This slot corresponds to the QCop service message
    \c{RotationManager::defaultRotation()}.
*/
void RotationService::defaultRotation()
{
    rotationManager->defaultRotation();
}

