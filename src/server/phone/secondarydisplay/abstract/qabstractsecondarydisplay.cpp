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

#include "qabstractsecondarydisplay.h"

/*!
  \class QAbstractSecondaryDisplay
    \inpublicgroup QtUiModule
  \brief The QAbstractSecondaryDisplay class allows developers to replace the "secondary screen" portion of the Phone UI.


  The QAbstractSecondaryDisplay interface is part of is part of the 
  \l {QtopiaServerApplication#qt-extended-server-widgets}{server widgets framework}
  and represents the portion of the phone UI that is shown on a smaller
  utility screen such as on clam shell phones.
  
  A small tutorial on how to develop new server widgets using one of the abstract widgets as base can
  be found in QAbstractServerInterface class documentation.
  
  \section1 Integration hints and requirements

  \list
    \o The secondary display should listen on the \c {QPE/System} qcop channel for
    \c applySecondaryBackgroundImage() messages. This message is sent when the user sets 
    a new background image for the secondary display.
    \o The image information are stored
    in the \c {Trolltech/qpe} settings file. The \c HomeScreen/SecondaryHomeScreenPicture key stores
    the image path and \c HomeScreen/SecondaryHomeScreenPictureMode gives an indication how the
    image should be layed out (ScaleAndCrop, Center/Tile, Scale and Stretch).
    \o The secondary display is closely linked to the HomeScreen settings application which is used to set 
    these values.
  \endlist

  Note that it is not required to support all the features mentioned above.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.

  \ingroup QtopiaServer::PhoneUI::TTSmartPhone

  */

/*! \fn QAbstractSecondaryDisplay::QAbstractSecondaryDisplay(QWidget *parent, Qt::WFlags flags)

  Construct a new QAbstractSecondaryDisplay with the specified \a parent and
  widget \a flags.
 */

