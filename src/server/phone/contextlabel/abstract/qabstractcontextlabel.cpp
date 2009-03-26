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

#include "qabstractcontextlabel.h"

/*!
    \class QAbstractContextLabel
    \inpublicgroup QtUiModule
    \brief The QAbstractContextLabel class allows developers to replace the "Context Label" portion of the Phone UI.
    \ingroup QtopiaServer::PhoneUI::TTSmartPhone

    The QAbstractContextLabel interface is part of the
    \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework} and allows developers
    to replace the standard header in the Qt Extended phone UI.
    A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
    be found in QAbstractServerInterface class documentation.

    The QAbstractContextLabel interface is marked as singleton interface. For more details
    about the concept of singleton server widgets refer to the \l {QtopiaServerApplication#singleton-pattern}{server widget documentation}.

    The BaseContextLabel should be used as starting point for custom context labels. It already
    provides the required button and key handling for context labels. It does not
    provide a UI yet. Therefore most customized context labels subclass BaseContextLabel
    and add the missing UI elements as part of their implementation. On the other hand it is
    possible to start a customized context label from scratch by directly deriving from
    QAbstractContextLabel.

    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa BaseContextLabel
*/

/*!
    \fn QAbstractContextLabel::QAbstractContextLabel(QWidget *parent = 0, Qt::WFlags flags = 0)

    Constructs a new QAbstractContextLabel instance, with the specified \a parent
    and \a flags.
 */

/*!
    \fn QAbstractContextLabel::~QAbstractContextLabel()

    Destroys the asbtract context label.
*/
/*!
    \fn QAbstractContextLabel::reservedSize() const

    Return the size that needs to be reserved for the context label on the screen in order to display it properly.
*/


