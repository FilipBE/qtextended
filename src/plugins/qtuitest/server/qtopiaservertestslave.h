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

#ifndef QTOPIASERVERTESTSLAVE_H
#define QTOPIASERVERTESTSLAVE_H

#include <QWSServer>
#include <qtestslave.h>
#include <QLabel>
#include <QDialog>

class QValueSpaceItem;
class KeyFilter;
class QEvent;
class QWSEvent;
class QWSMouseEvent;
class QWSWindow;

// Server test slave
class QtopiaServerQueryMaster;
class QtopiaServerTestSlavePrivate;
class QtUiTestProtocolServer;
class QtopiaServerTestSlave : public QTestSlave
{
    Q_OBJECT
public:
    QtopiaServerTestSlave();
    virtual ~QtopiaServerTestSlave();

    void keyboardFilter(int keycode, int modifiers, bool isPress, bool autoRepeat);

    virtual QTestMessage constructReplyToMessage( QTestMessage const &msg );
    virtual uint postMessage( QTestMessage const &message );

    virtual void onConnected();

protected:
    QString realAppName( const QString &appName );

    QtopiaServerQueryMaster* findSlaveForApp(QString const &app);

    virtual void recordEvent(RecordEvent::Type,QString const&,QString const&,QVariant const&);

private slots:
    void waitTrigger();
    void onWindowEvent( QWSWindow *window, QWSServer::WindowEvent event );
    void onNewMessage( const QTestMessage &msg );

private:
    QValueSpaceItem *waitVS;
    KeyFilter *m_keyfilter;
    QVariant waitValue;
    QString active_title;
    QtopiaServerTestSlavePrivate *d;
    friend class QtopiaServerTestSlavePrivate;
    friend class QtopiaServerQueryMaster;
};

#endif
