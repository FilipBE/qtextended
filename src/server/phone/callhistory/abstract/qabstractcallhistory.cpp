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

#include "qabstractcallhistory.h"
#include "qtopiaserverapplication.h"
#include "applicationlauncher.h"
#include <QPointer>

/*!
    \class QAbstractCallHistory
    \inpublicgroup QtTelephonyModule
    \brief The QAbstractCallHistory class allows developers to replace the "call history screen" portion of the Phone UI.

    The QAbstractCallHistory is part of the \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework}
    and represents the portion of the phone UI that users can access to see missing calls, dialed calls and received calls.
    A small tutorial on how to develop new server widgets using one of the abstract widgets as base can be found in QAbstractServerInterface class documentation.
    In addition it is marked as singleton interface. For more details
    about the concept of singleton server widgets refer to the \l {QtopiaServerApplication#singleton-pattern}{server widget documentation}.


    This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \ingroup QtopiaServer::PhoneUI::TTSmartPhone
*/

/*!
  \fn QAbstractCallHistory::QAbstractCallHistory(QWidget *parent, Qt::WFlags flags)

  Constructs a new QAbstractCallHistory instance, with the specified \a parent
  and widget \a flags.
*/

/*!
  \fn void QAbstractCallHistory::reset()

  Reset the filtering criterias of the views.
*/

/*!
  \fn void QAbstractCallHistory::showMissedCalls()

 Returns true if the missed calls have been seen, false otherwise.
*/

/*!
  \fn void QAbstractCallHistory::refresh()
  Refresh the views.
*/

/*!
  \fn void QAbstractCallHistory::setFilter(const QString &f)
  
  Sets \a f as the filtering criteria of the calls view.
  This will replace any existing filtering criteria.
*/

/*!
  \fn void QAbstractCallHistory::requestedDial( const QString& number, const QUniqueId& uid)

  Emitted when a dial is requested. The \a number sent is the phone number requested and
  the \a uid is the unique identifier for this contact.
*/

/*!
  \fn void QAbstractCallHistory::viewedMissedCalls()

  Emitted when the missed calls have been seen.
  \sa showMissedCalls()
*/

// "callhistory" builtin
static QWidget *callhistory()
{
    static QPointer<QAbstractCallHistory> history = 0;
    if (!history) {
       history = qtopiaWidget<QAbstractCallHistory>();
    }
    if (history)
        history->setFilter(QLatin1String(""));

    return history;
}
QTOPIA_SIMPLE_BUILTIN(callhistory, callhistory);
