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
#ifndef QDSYNC_H
#define QDSYNC_H

#include <QTextBrowser>

class QCopBridge;
class TransferServer;

class QUsbEthernetGadget;
class QUsbSerialGadget;

class QDSync : public QTextBrowser
{
    Q_OBJECT
public:
    QDSync( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~QDSync();

private slots:
    void appMessage( const QString &message, const QByteArray &data );
    void qdMessage( const QString &message, const QByteArray &data );
    void startDaemons();
    void stopDaemons();
    void ethernetActivated();
    void ethernetDeactivated();
    void serialActivated();
    void serialDeactivated();
    void gotConnection();
    void lostConnection();

private:
    void showEvent( QShowEvent *e );
    void closeEvent( QCloseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void keyReleaseEvent( QKeyEvent *e );

    QCopBridge *bridge;
    TransferServer *tserver;
    bool selectDown;
    bool connected;
    bool syncing;

    QUsbEthernetGadget *m_ethernetGadget;
    QUsbSerialGadget *m_serialGadget;

    enum SelectLabelState {
        Blank,
        Sync,
        Cancel,
    } selectLabelState;

    QStringList ports;
};

#endif
