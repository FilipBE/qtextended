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

#include "qaudionamespace.h"

/*!
    \class QAudio
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule

    \brief The QAudio namespace provides a container for miscellaneous Audio functionality.

    The QAudio namespace defines various functions and enums that
    are used globally by the QAudio library.
  */

/*!
    \enum QAudio::AudioCapability
    Defines the types of audio features that a particular audio device
    is capable of.  The QAudio::AudioCapabilities defines all capabilities
    supported by the audio device.  For instance, InputOnly | OutputOnly
    signifies that the Audio Device is capable of Input or Output capability
    at a particular time, but not both.  InputOnly | OutputOnly | InputAndOuput
    specifies that the device can be used in full duplex or half duplex modes.

    \value None No capabilities.
    \value InputOnly Only input capability is supported.
    \value OutputOnly Only output capability is supported.
    \value InputAndOutput Both input and output capabilities are supported.
*/
