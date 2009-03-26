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

#ifndef QSIMSYNC_P_H
#define QSIMSYNC_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <qphonebook.h>
#include <qsiminfo.h>
#include <qpimsource.h>
#include "qpreparedquery_p.h"

class ContactSqlIO;
class QValueSpaceObject;
class QTimer;
// exported as is used by the server... the intent is to make this class
// public once its API is considered stable.
class QTOPIAPIM_EXPORT QContactSimSyncer : public QObject
{
    Q_OBJECT
public:
    QContactSimSyncer(const QString &cardType = "SM", QObject *parent = 0);
    virtual ~QContactSimSyncer();

    enum State
    {
        Idle = 0x0,
        ReadingId = 0x01,
        ReadingLimits = 0x02,
        ReadingEntries = 0x04,
        UpdatingSqlTable = 0x08,
    };

    enum Error
    {
        NoError,
        UknownError
    };

    int state() const;
    Error error() const;
    QString errorString() const;

    QString storage() const { return mSimType; }

public slots:
    void abort();
    void sync();

signals:
    void done(bool error);
    void stateChanged(int state);

private slots:
    void updatePhoneBook(const QString &, const QList<QPhoneBookEntry> &);
    void updateSimIdentity();
    void updatePhoneBookLimits(const QString &, const QPhoneBookLimits &);

    void simInfoTimeout();

private:
    void setComplete(bool error);
    void resetSqlState();

    QUniqueId simCardId(int index) const;
    void setSimCardId(int index, const QUniqueId &) const;
    void setSimIdentity(const QString &, const QDateTime &);

    QList<QContact> mergedContacts() const;
    void updateSqlEntries();
    
    ContactSqlIO *mAccess;
    
    int readState;
    Error mError;

    const QString mSimType;
    QPimSource mSource;
    int SIMLabelLimit;
    int SIMNumberLimit;
    int SIMListStart;
    int SIMListEnd;
    QList<QPhoneBookEntry> phoneData;
    QString mActiveCard;
    QDateTime mInsertTime;

    mutable QPreparedSqlQuery addNameQuery;
    mutable QPreparedSqlQuery addNumberQuery;
    mutable QPreparedSqlQuery updateNameQuery;
    mutable QPreparedSqlQuery updateNumberQuery;
    mutable QPreparedSqlQuery removeNameQuery;
    mutable QPreparedSqlQuery removeNumberQuery;
    mutable QPreparedSqlQuery selectNameQuery;
    mutable QPreparedSqlQuery selectNumberQuery;

    mutable QPreparedSqlQuery selectIdQuery;
    mutable QPreparedSqlQuery insertIdQuery;
    mutable QPreparedSqlQuery updateIdQuery;

    QPhoneBook *mPhoneBook;
    QSimInfo *mSimInfo;
    QValueSpaceObject *simValueSpace;
    QTimer *mReadyTimer;
};

#endif
