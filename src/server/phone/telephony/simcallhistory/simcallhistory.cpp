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

#include "simcallhistory.h"
#include "dialercontrol.h"
#include "qtopiaserverapplication.h"
#include "servercontactmodel.h"

#include <QSimInfo>

#include <QSettings>
#include <QContact>

class SimCallHistoryPrivate
{
public:
    SimCallHistoryPrivate()
        : m_phoneBook(0) {}

    QPhoneBook *m_phoneBook;
    QString m_simIdentity;
};

/*!
  \class SimCallHistory
    \inpublicgroup QtCellModule
  \brief The SimCallHistory class populates the in-memory call history list with 
  entries provided by the call history list stored on a SIM card.
  \ingroup QtopiaServer::Telephony

  This class is a Qt Extended server task and is automatically started by the server. It is part
  of the Qt Extended server and cannot be used by other Qt Extended applications.
*/

/*!
  Creates a new SimCallHistory instance with the given \a parent.
  */
SimCallHistory::SimCallHistory( QObject *parent )
    : QObject( parent ), d(new SimCallHistoryPrivate)
{
    QSimInfo *simInfo = new QSimInfo( "modem", this );
    connect( simInfo, SIGNAL(inserted()), this, SLOT(simInserted()) );

    QSettings cfg("Trolltech","Phone");
    cfg.beginGroup("SIM");
    d->m_simIdentity = cfg.value("Identity").toString();
}

/*!
  \internal
  */
SimCallHistory::~SimCallHistory()
{
    if (d)
        delete d;
}
void SimCallHistory::simInserted()
{
    if ( !d->m_phoneBook ) {
        d->m_phoneBook = new QPhoneBook( "modem", this ); // No tr
        connect( d->m_phoneBook, SIGNAL(ready()),
                this, SLOT(readCallHistory()) );
    }
}

void SimCallHistory::readCallHistory()
{
    QSimInfo simInfo( "modem", this );
    // if it is not a new sim. do not need to read SIM phone books for call history.
    if ( simInfo.identity() == d->m_simIdentity )
        return;

    d->m_simIdentity = simInfo.identity();

    // record new sim identity
    QSettings cfg("Trolltech", "Phone");
    cfg.beginGroup("SIM");
    cfg.setValue("Identity", d->m_simIdentity);

    // Read SIM phone books for last dialed, missed, received calls
    fetchCallHistory();
}

void SimCallHistory::fetchCallHistory()
{
    if ( d->m_phoneBook ) {
        connect( d->m_phoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
            this, SLOT(callHistoryEntriesFetched(QString,QList<QPhoneBookEntry>)) );
        d->m_phoneBook->getEntries( "DC" ); // Dialed calls
        d->m_phoneBook->getEntries( "LD" ); // Last dialed numbers
        d->m_phoneBook->getEntries( "MC" ); // Missed calls
        d->m_phoneBook->getEntries( "RC" ); // Received calls
    }
}
    
    
void SimCallHistory::callHistoryEntriesFetched
    ( const QString& store, const QList<QPhoneBookEntry>& list )
{
    QCallListItem::CallType type;
    if ( store == "DC" || store == "LD" )
        type = QCallListItem::Dialed;
    else if ( store == "MC" )
        type = QCallListItem::Missed;
    else if ( store == "RC" )
        type = QCallListItem::Received;
    else
        return;

    QCallList &callList = DialerControl::instance()->callList();

    QStringList numberList;
    foreach ( QPhoneBookEntry entry, list ) {
        if ( !numberList.contains( entry.number() ) )
            numberList.append( entry.number() );
    }

    foreach ( QString number, numberList ) {
        QContact contact = ServerContactModel::instance()->matchPhoneNumber( number );
        QCallListItem item( type, number );
        callList.record( item );
    }

    if ( store == "RC" ) { // no need to read phone books anymore.
        disconnect( d->m_phoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
            this, SLOT(callHistoryEntriesFetched(QString,QList<QPhoneBookEntry>)) );
    }
}

QTOPIA_TASK(SimCallHistory,SimCallHistory);
