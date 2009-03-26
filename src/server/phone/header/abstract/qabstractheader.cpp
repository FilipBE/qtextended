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

#include "qabstractheader.h"

/*!
  \class QAbstractHeader
    \inpublicgroup QtUiModule
  \brief The QAbstractHeader class allows developers to replace the "Header" portion of the Phone UI.
  \ingroup QtopiaServer::PhoneUI::TTSmartPhone

  The QAbstractHeader interface is part of the
  \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework} and allows developers 
  to replace the standard header in the Qt Extended phone UI.
  A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
  be found in QAbstractServerInterface class documentation.

  The QAbstractHeader interface is marked as singleton interface. For more details 
  about the concept of singleton server widgets refer to the \l {QtopiaServerApplication#singleton-pattern}{server widget documentation}.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
  */

/*!
  \fn QAbstractHeader::QAbstractHeader(QWidget *parent = 0, Qt::WFlags flags = 0)

  Constructs a new QAbstractHeader instance, with the specified \a parent
  and widget \a flags.
 */

/*!
  \fn QAbstractHeader::~QAbstractHeader()

  Destroys the asbtract header.
*/
/*!
  \fn QAbstractHeader::reservedSize() const

  Return the size that needs to be reserved for the header on the screen in order to display it properly.

*/
