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

#include "qtopiapowermanagerservice.h"
#include "qtopiapowermanager.h"
#include <qtopiaipcenvelope.h>
#include <custom.h>

/*!
    \service QtopiaPowerManagerService QtopiaPowerManager
    \inpublicgroup QtBaseModule
    \brief The QtopiaPowerManagerService class provides the QtopiaPowerManager service.

    The \i QtopiaPowerManager service enables applications to control the
    behavior of the Qt Extended power manager.  Normally an application will use
    QtopiaApplication::setPowerConstraint() for this,
    but finer control is available for settings applications.
*/

/*!
    \internal
*/
QtopiaPowerManagerService::QtopiaPowerManagerService( QtopiaPowerManager *manager, QObject *parent )
    : QtopiaAbstractService( "QtopiaPowerManager", parent )
{
    m_powerManager = manager;
    publishAll();
}

/*!
    \internal
*/
QtopiaPowerManagerService::~QtopiaPowerManagerService()
{
}

/*!
    Set the power manager intervals to \a dim, \a lightOff, and \a suspend,
    for the three levels of power saving.  The values are in seconds.

    This slot corresponds to the QCop service message
    \c{QtopiaPowerManager::setIntervals(int,int,int)}.
*/
void QtopiaPowerManagerService::setIntervals( int dim, int lightOff, int suspend )
{
    int ivals[3];
    ivals[0] = dim;
    ivals[1] = lightOff;
    ivals[2] = suspend;

    m_powerManager->setIntervals( ivals, sizeof(ivals)/sizeof(int) );
}

/*!
    Set the power manager intervals to their default values.

    This slot corresponds to the QCop service message
    \c{QtopiaPowerManager::setDefaultIntervals()}.
*/
void QtopiaPowerManagerService::setDefaultIntervals()
{
    m_powerManager->setDefaultIntervals();
}

/*!
    Set the backlight level to \a brightness, between 0 and 255.

    This slot corresponds to the QCop service message
    \c{QtopiaPowerManager::setBacklight(int,int)}.

    Once the brightnes has been updated, the \c{brightnessChanged(int)}
    message will be sent to the \c{Qtopia/PowerStatus} channel.

    If \a brightness is -3, then the backlight will be forced on.
    If \a brightness is -2, then the backlight will be toggled from
    its current state.  If \a brightness is -1, then the backlight
    will be set to its default state.
*/
void QtopiaPowerManagerService::setBacklight( int brightness )
{
    m_powerManager->setBacklight( brightness );
}

/*!
    Activate power management if \a on is true; otherwise deactivate power management.

    This slot corresponds to the QCop service message
    \c{QtopiaPowerManager::setActive(bool)}.
*/
void QtopiaPowerManagerService::setActive( bool on )
{
    m_powerManager->setActive( on );
}

/*!
    Apply the power management constraint \a constraint for the application \a app.
    The \a constraint value is one of the values from QtopiaApplication::PowerConstraint.

    Normally client applications will use
    QtopiaApplication::setPowerConstraint().

    This slot corresponds to the QCop service message
    \c{QtopiaPowerManagerService::setConstraint(int,QString)}.

    \sa QtopiaApplication::setPowerConstraint()
*/
void QtopiaPowerManagerService::setConstraint(int constraint, const QString &app)
{
    QtopiaPowerConstraintManager *man = QtopiaPowerConstraintManager::instance();
    if ( man )
        man->setConstraint((QtopiaApplication::PowerConstraint) constraint, app);
}
