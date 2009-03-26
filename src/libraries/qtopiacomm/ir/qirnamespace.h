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

#ifndef QIRNAMESPACE_H
#define QIRNAMESPACE_H

#include <QFlags>

#ifndef Q_QDOC
namespace QIr
{
#else
class QIr
{
public:
#endif
    enum DeviceClass {
        PlugNPlay = 0x0001,
        PDA = 0x0002,
        Computer = 0x0004,
        Printer = 0x0008,
        Modem = 0x0010,
        Fax = 0x0020,
        LAN = 0x0040,
        Telephony = 0x0080,
        FileServer = 0x0100,
        Communications = 0x0200,
        Message = 0x0400,
        HTTP = 0x0800,
        OBEX = 0x1000,
        All = 0xffff
    };

    Q_DECLARE_FLAGS(DeviceClasses, DeviceClass)

    QByteArray convert_charset_to_string(int charset);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QIr::DeviceClasses)

#endif
