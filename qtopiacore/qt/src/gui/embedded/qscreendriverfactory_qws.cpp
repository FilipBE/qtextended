/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qscreendriverfactory_qws.h"

#include "qscreen_qws.h"
#include "qapplication.h"
#include "qscreenlinuxfb_qws.h"
#include "qscreentransformed_qws.h"
#include "qscreenvfb_qws.h"
#include "qscreenvnc_qws.h"
#include "qscreenmulti_qws_p.h"
#include <stdlib.h>
#include "private/qfactoryloader_p.h"
#include "qscreendriverplugin_qws.h"

QT_BEGIN_NAMESPACE

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QScreenDriverFactoryInterface_iid,
     QLatin1String("/gfxdrivers"), Qt::CaseInsensitive))

#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL

/*!
    \class QScreenDriverFactory
    \ingroup qws

    \brief The QScreenDriverFactory class creates screen drivers in
    Qt for Embedded Linux.

    Note that this class is only available in \l{Qt for Embedded Linux}.

    QScreenDriverFactory is used to detect and instantiate the
    available screen drivers, allowing \l{Qt for Embedded Linux} to load the
    preferred driver into the server application at runtime.  The
    create() function returns a QScreen object representing the screen
    driver identified by a given key. The valid keys (i.e. the
    supported drivers) can be retrieved using the keys() function.


    \l{Qt for Embedded Linux} provides several built-in screen drivers. In
    addition, custom screen drivers can be added using Qt's plugin
    mechanism, i.e. by subclassing the QScreen class and creating a
    screen driver plugin (QScreenDriverPlugin). See the
    \l{Qt for Embedded Linux Display Management}{display management}
    documentation for details.

    \sa QScreen, QScreenDriverPlugin
*/

/*!
    Creates the screen driver specified by the given \a key, using the
    display specified by the given \a displayId.

    Note that the keys are case-insensitive.

    \sa keys()
*/
QScreen *QScreenDriverFactory::create(const QString& key, int displayId)
{
    QString driver = key.toLower();
#ifndef QT_NO_QWS_QVFB
    if (driver == QLatin1String("qvfb") || driver.isEmpty())
        return new QVFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_LINUXFB
    if (driver == QLatin1String("linuxfb") || driver.isEmpty())
        return new QLinuxFbScreen(displayId);
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    if (driver == QLatin1String("transformed"))
        return new QTransformedScreen(displayId);
#endif
#ifndef QT_NO_QWS_VNC
    if (driver == QLatin1String("vnc"))
        return new QVNCScreen(displayId);
#endif
#ifndef QT_NO_QWS_MULTISCREEN
    if (driver == QLatin1String("multi"))
        return new QMultiScreen(displayId);
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY

    if (QScreenDriverFactoryInterface *factory = qobject_cast<QScreenDriverFactoryInterface*>(loader()->instance(key)))
        return factory->create(driver, displayId);

#endif
#endif
    return 0;
}

/*!
    Returns the list of valid keys, i.e. the available screen drivers.

    \sa create()
*/
QStringList QScreenDriverFactory::keys()
{
    QStringList list;

#ifndef QT_NO_QWS_QVFB
    list << QLatin1String("QVFb");
#endif
#ifndef QT_NO_QWS_LINUXFB
    list << QLatin1String("LinuxFb");
#endif
#ifndef QT_NO_QWS_TRANSFORMED
    list << QLatin1String("Transformed");
#endif
#ifndef QT_NO_QWS_VNC
    list << QLatin1String("VNC");
#endif
#ifndef QT_NO_QWS_MULTISCREEN
    list << QLatin1String("Multi");
#endif

#if !defined(Q_OS_WIN32) || defined(QT_MAKEDLL)
#ifndef QT_NO_LIBRARY
    QStringList plugins = loader()->keys();
    for (int i = 0; i < plugins.size(); ++i) {
# ifdef QT_NO_QWS_QVFB
        // give QVFb top priority for autodetection
        if (plugins.at(i) == QLatin1String("QVFb"))
            list.prepend(plugins.at(i));
        else
# endif
        if (!list.contains(plugins.at(i)))
            list += plugins.at(i);
    }
#endif //QT_NO_LIBRARY
#endif //QT_MAKEDLL
    return list;
}

QT_END_NAMESPACE
