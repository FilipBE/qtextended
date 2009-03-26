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

#include "qmediaengineinformation.h"


/*!
    \class QMediaEngineInformation
    \inpublicgroup QtMediaModule
    \preliminary
    \brief The QMediaEngineInformation class is a base class used to provide information about a Media Engine.

    Media Engines derive a class from QMediaEngineInformation to provide information about the
    Engine to the Media Server

    \code
    Example
    {
        class Info : public QMediaEngineInformation
        {
        public:
            Info()
            {
                // do stuff, make sessionbuilders
            }

            ~Info() {}

            QString name() const { return "Example"; }
            QString version() const { return "1.0.0"; }

            int idleTime() const return { 10 * 1000; }  // 10 seconds

            QMediaSessionBuilderList sessionBuilders() const { return m_sessionBuilderList; }

        private:
            QMediaSessionBuilderList    m_sessionBuilderList;
        }:
    }
    \endcode

*/



/*!
    Destruct a QMediaEngineInformation.
*/

QMediaEngineInformation::~QMediaEngineInformation()
{
}

/*!
    \fn QString QMediaEngineInformation::name() const

    Returns the name of the Media Engine.
*/

/*!
    \fn QString QMediaEngineInformation::version() const

    Returns the version string of the Media Engine.
*/

/*!
    \fn int QMediaEngineInformation::idleTime() const

    Return the number of milliseconds before the engine should be suspended.
    Suspension based on idle time is optional, a value of -1 will indicate to
    the Media Server that this Engine should never be suspended based on a
    timeout.
*/

/*!
    \fn bool QMediaEngineInformation::hasExclusiveDeviceAccess() const

    Returns true if the engine uses a audio output device that is not able to
    be shared with other engines or processes.
*/

/*!
    \fn QMediaSessionBuilderList QMediaEngineInformation::sessionBuilders() const

    Return the list of QMediaSessionBuilders that are used by this Engine. The
    Media Server uses the type of the builders to appropriately route session
    creation requests to Media Engines.

    \sa QMediaSessionBuilder
*/
