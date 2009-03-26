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

#include "obexclientwindow.h"
#include "obexquoteserver.h"
#include <qtopiaapplication.h>

/*
    This demonstrates the basics of using OBEX support in Qtopia to set up
    an OBEX server and then connect OBEX clients to that server.

    For this example, the OBEX sessions are run over TCP using QTcpSocket,
    but they could have used any transport provided by a subclass of
    QIODevice. For example, the QBluetoothRfcommSocket class could have
    been used to run the OBEX sessions over Bluetooth instead.

    More example code is provided in the QObexClientSession and
    QObexServerSession class documentation.
*/

QSXE_APP_KEY
int main( int argc, char **argv )
{
    QSXE_SET_APP_KEY(argv[0])
    QtopiaApplication app( argc, argv );

    ObexQuoteServer server;
    if (server.run()) {
        ObexClientWindow win(server.serverAddress(), server.serverPort());
        win.show();
    } else {
        qWarning("Unable to run OBEX example server");
    }

    return app.exec();
}
