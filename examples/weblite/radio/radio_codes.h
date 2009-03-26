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
#ifndef RADIOCODES_H
#define RADIOCODES_H
namespace RadioCodes
{
    enum Status
    {
        InfoBase = 100,
            AddressFound = 101,
            Connecting = 102,
            RequestSent = 103,
            CreatingFile = 104,
            StreamTitleChanged = 105,
            NoMetadata = 106,
            Header = 107,
            
        SuccessBase = 200,
            Ready = 200,
        RedirectBase = 300,
        ErrorBase = 400,
        BadRequest = 400,
        UsrErrorBase = 450,
        SendMismatch = 451,
        HostNotFound= 452,
        NoMetadataInterval = 453,
        FailedToCreateNamedPipe = 454,
        FailedToOpenFile = 455,
        FailedToCreateSocket = 456,
        FailedToConnectWithServer = 457,
        FailedToReceive = 458,
        InvalidUrl = 459,
        Timeout = 460
    };
};
#endif
