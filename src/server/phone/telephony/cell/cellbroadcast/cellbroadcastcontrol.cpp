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

#include "cellbroadcastcontrol.h"
#include "qabstractmessagebox.h"
#include "qabstractcallpolicymanager.h"
#include "messageboard.h"
#include <QSettings>
#include <QString>
#include <QCellBroadcast>
#include <QTimer>

/*!
    \class CellBroadcastControl
    \inpublicgroup QtCellModule
    \ingroup QtopiaServer::Telephony
    \brief The CellBroadcastControl class monitors incoming cell broadcast messages.

    This task opens a message box if a cellbroadcast message of type CellBroadcastControl::Popup
    is received and notfies the message board task about any background broadcasts.
    If this is not a desired behavior or such a notification
    would be handled by a different class the message box can be disabled by setting the
    \c {CellBroadcast\DisablePopup} key to \c true in the \c {Trolltech/Phone} configuration file. If this setting
    is missing, the CellBroadcastControl class opens the above mentioned popup box.

    This class is a Qt Extended server task. It is part of the Qt Extended server and cannot be used by other Qt Extended applications.

    \sa MessageBoard
 */

/*!
    \enum CellBroadcastControl::Type

    Represents the type of a received cell broadcast message.

    \value Popup A popup message.  The text should be shown to the user immediately.
    \value Background A background message.  The text should be shown on the homescreen.
*/

/*!
  Constructs a new CellBroadcastControl instance with the specified \a parent.
  */
CellBroadcastControl::CellBroadcastControl(QObject *parent)
: QObject(parent), infoMsgId(-1)
{
    cb = new QCellBroadcast(QString(), this);
    QObject::connect(cb, SIGNAL(broadcast(QCBSMessage)),
                    this, SLOT(cellBroadcast(QCBSMessage)));
    QObject::connect(cb, SIGNAL(setChannelsResult(QTelephony::Result)),
                    this, SLOT(cellBroadcastResult(QTelephony::Result)));

    firstSubscribe = false;
    QAbstractCallPolicyManager *cellModem = QAbstractCallPolicyManager::managerForCallType("Voice");
    if (cellModem)
        connect(cellModem,
            SIGNAL(registrationStateChanged(QTelephony::RegistrationState)),
            this,
            SLOT(registrationChanged(QTelephony::RegistrationState)));
    else 
        qLog(Component) << "CellBroadcastControl: No Cellmodem manager component available";
}

static bool isLocationSubscribed()
{
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    int count = cfg.value( "count", 0 ).toInt();
    for ( int i = 0; i < count; ++i ) {
        if ( cfg.value( "num" + QString::number( i ) ).toInt() == 50
                && cfg.value( "on" + QString::number( i ) ).toBool() )
            return true;
    }
    return false;
}

/*!
  \fn void CellBroadcastControl::broadcast(CellBroadcastControl::Type type, const QString &channel, const QString &text)

  Emitted whenever a new cell broadcast message of the given \a type is
  received.  \a channel indicates the channel on which the message was received
  and \a text its text.
 */

/*
    cell broadcast message has arrived.
    process the message according to the user preferences
*/
void CellBroadcastControl::cellBroadcast(const QCBSMessage &m)
{
    // check if any channel is registered
    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    int count = cfg.value( "count",0 ).toInt();
    if ( count == 0 )
        return;

    // check if the registred channel is currently activated
    uint channel = m.channel();
    bool active = false;
    int i = 0;
    for ( ; i < count ;i++ ) {
        if ( channel == (uint)cfg.value( "num" + QString::number(i) ).toInt()
            && cfg.value( "on" + QString::number(i) ).toBool() ) {
            active = true;
            break;
        }
    }

    if ( channel == 50 ) {
        cellLocation = m.text();
        QAbstractCallPolicyManager *cellModem = QAbstractCallPolicyManager::managerForCallType("Voice");
        if (cellModem && isLocationSubscribed())
            cellModem->setCellLocation( m.text() );
    }

    if ( !active )
        return;

    // check if transmitted languages matches user's preferences
    QCBSMessage::Language lang = m.language();
    QStringList lstLang = cfg.value( "languages"+QString::number(i)).toString().split(',', QString::SkipEmptyParts );
    if ( !lstLang.contains( QString::number( lang ) ) )
        return;

    // check which display mode user prefers
    int mode = cfg.value("mode"+QString::number(i)).toInt();
    QString label = cfg.value("label"+QString::number(i)).toString();
    // mode 0 = Background - Home screen
    // mode 1 = Foreground - Popup message

    QString channelText = QString("%1 %2").arg(label).arg(channel);
    emit broadcast((mode == 0)?Background:Popup, channelText, m.text());

    if ( cfg.value("DisablePopup", false ).toBool() )
        return;

    if ( 1 == mode ) {
        showPopup( channelText, m.text() );
    } else {
        //send background notification to message board for display
        MessageBoard *board = qtopiaTask<MessageBoard>();
        if ( board ) {
            // clear pervious message if not yet deleted
            // otherwise it will be shown forever
            if ( infoMsgId > -1  )
                hideCBSMessage();

            QString disp = channelText + "\n" + m.text();
            MessageBoard *board = qtopiaTask<MessageBoard>();
            if ( board )
                infoMsgId = board->postMessage(":image/antenna", disp);
            else 
                qLog(Component) << "CellBroadcastControl: Cannot post message due to missing MessageBoard";
            QTimer::singleShot(10000, this, SLOT(hideCBSMessage()));
     
        }
    }
}

void CellBroadcastControl::registrationChanged(QTelephony::RegistrationState state)
{
    switch ( state ) {
    case QTelephony::RegistrationHome:
    case QTelephony::RegistrationUnknown:
    case QTelephony::RegistrationRoaming:
        // If this is the first time we've seen registration, then
        // subscribe with the settings in Phone.conf.  Further changes
        // will be done by the "phonesettings" application.
        if ( !firstSubscribe ) {
            firstSubscribe = true;
            subscribe();
        }
        break;
    default: break;
    }
}

void CellBroadcastControl::subscribe()
{
    QList<int> list;

    QSettings cfg( "Trolltech", "Phone" );
    cfg.beginGroup( "CellBroadcast" );
    int count = cfg.value( "count", 0 ).toInt();
    if ( count > 0 ) {
        for ( int i = 0; i < count; ++i ) {
            if ( cfg.value( "on" + QString::number( i ) ).toBool() ) // active
                list << cfg.value( "num" + QString::number( i ) ).toInt();
        }
    }
    if ( !list.contains( 50 ) )
        list << 50; // location channel, default
    cb->setChannels( list );
}

void CellBroadcastControl::showPopup( const QString &channel, const QString &text )
{
    static QAbstractMessageBox *cbsMessageBox = 0;
    
    // remove the previous message box if there is any
    // user will see the latest one only
    if ( cbsMessageBox )
        delete cbsMessageBox;

    cbsMessageBox = QAbstractMessageBox::messageBox(0, channel,
                                   text, QAbstractMessageBox::Information,
                                   QAbstractMessageBox::No);
    QtopiaApplication::showDialog(cbsMessageBox);
}

/*!
  \internal
  */
void CellBroadcastControl::hideCBSMessage()
{
    if ( infoMsgId == -1 )
        return;

    MessageBoard *board = qtopiaTask<MessageBoard>();
    if ( board )
        board->clearMessage(infoMsgId);
    infoMsgId = -1;
}

/*
    When cell broadcast subscription is requested from other applications
    for example, call options app, we need to make sure
    the cell location is shown on the homescreen when the user thinks
    it is subscribed.
*/
void CellBroadcastControl::cellBroadcastResult(QTelephony::Result result)
{
    if (result != QTelephony::OK)
        return;

    QAbstractCallPolicyManager *cellModem = QAbstractCallPolicyManager::managerForCallType("Voice");
    if (!cellModem)
        return;

    if (isLocationSubscribed())
        cellModem->setCellLocation( cellLocation );
    else
        cellModem->setCellLocation( QString() );
}
#
QTOPIA_TASK(CellBroadcastControl, CellBroadcastControl);
QTOPIA_TASK_PROVIDES(CellBroadcastControl,CellBroadcastControl);
