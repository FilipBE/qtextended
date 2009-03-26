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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qtopiasystemtestslave.h"

#include <QFile>
#include <QTextStream>
#include <qtopiabase/qtopianamespace.h>

#include <private/testslaveinterface_p.h>

// ***************************************

QtopiaSystemTestSlave::QtopiaSystemTestSlave()
    : QTestSlave()
{
    QFile f( Qtopia::homePath() + "/.qtestport" );
    quint16 port;
    if ( f.open( QIODevice::ReadOnly ) ) {
        QTextStream(&f) >> port;
        connect( "127.0.0.1", port );
        enableReconnect( true, 1000 );
    }
}

// *********************************************

class QTUITEST_EXPORT AppTestSlavePlugin : public QObject, public TestSlaveInterface
{
    Q_OBJECT
    Q_INTERFACES(TestSlaveInterface)

    public:
        AppTestSlavePlugin(QObject *parent = 0)
            : QObject(parent) {}

        virtual void postMessage(QString const &name, QVariantMap const &data)
        { realSlave.postMessage( QTestMessage( name, data ) ); }

        virtual bool isConnected() const
        { return realSlave.isConnected(); }

        virtual void showMessageBox(QWidget* widget, QString const& title, QString const& text)
        { realSlave.showMessageBox(widget,title,text); }

        virtual void showDialog(QWidget* widget, QString const& title)
        { realSlave.showDialog(widget,title); }

    private:
        mutable QtopiaSystemTestSlave realSlave;
};
Q_EXPORT_PLUGIN2(appslave, AppTestSlavePlugin)

#include "qtopiasystemtestslave.moc"

