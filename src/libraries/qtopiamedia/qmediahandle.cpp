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

#include "qmediacontent.h"

#include "qmediahandle_p.h"

/*!
    \class QMediaHandle
    \inpublicgroup QtMediaModule
    \internal

    \brief The QMediaHandle class is used to refer to an activated media content
           within the Media System.
    \ingroup multimedia
*/

/*!
    \fn QMediaHandle::QMediaHandle() {}
    \internal
*/

/*!
    \fn QMediaHandle::QMediaHandle(QUuid const& id):
    \internal
*/

/*!
   \fn QMediaHandle::QMediaHandle(QMediaHandle const& c):
    \internal
*/

/*!
    \fn QMediaHandle::operator=(QMediaHandle const& rhs);
    \internal
*/

/*!
    \fn QMediaHandle::id() const
    \internal
*/

/*!
    \fn QMediaHandle::getHandle(QMediaContent* content)
    \internal
*/

QMediaHandle QMediaHandle::getHandle(QMediaContent* content)
{
    return content->handle();
}
