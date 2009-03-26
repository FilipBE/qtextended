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

#include <qsimsync_p.h>
#include <qsimcontext_p.h>
#include <qpimsqlio_p.h>
#include <qcontactsqlio_p.h>
#include <qfielddefinition.h>

#include <QValueSpaceObject>
#include <QTimer>

#include <qtopialog.h>

/*!
  \class QContactSimSyncer
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
  \ingroup pim
  \internal
  \brief The QContactSimSyncer class provides a mechanism to sync SIM contacts to the PIM database.

  To use it, simply create the object with the appropriate type, e.g. SN for service numbers, SM for the standard SIM phone book.

  The initial step is to clear all existing contacts in phone memory for
  the SIM storage type that could be made to fit solely on the SIM.  This is so that any contacts that may have been from a temporarily inserted card
  are removed as per the GSM specification, while not loosing user data for contacts that have data extending beyond the capability of a SIM.

  Following this, when all information is available for the SIM card in
  question the contacts are merged into the PIM database.  Similar contacts on the SIM phone book such as "Bob/hp" and "Bob/wo" are merged into one contact.  In this case it would be "Bob", with a home phone and a work phone number.  The phone numbers as found on the SIM will override any currently
  stored for the contact in the PIM database.

  While it is not necessary, the state of the SIM synchronization process can be monitired using the query functions state() and error(), or using the signals done() and stateChanged().
*/

/*!
  \enum QContactSimSyncer::State
  \internal

  Represents the current state of synchronizing the SIM card contact storage
  to the PIM database.

  \value Idle
    No work is pending.
  \value ReadingId
    The QContactSimSyncer is waiting on the id of the SIM card.
  \value ReadingLimits
    The QContactSimSyncer is waiting on the limits of indexes for the
    SIM storage type.
  \value ReadingEntries
    The QContactSimSyncer is waiting on the entries stored in the SIM
    storage type.
  \value UpdatingSqlTable
    The QContactSimSyncer is updating entries in the PIM database.
*/

/*!
 \enum QContactSimSyncer::Error

    \value NoError
        No error has occurred.
    \value UknownError
        An unspecified error has occurred.
*/

/*!
  Constructs a new QContactSimSyncer as a child of the given \a parent for SIM storage of the given \a type.
*/
QContactSimSyncer::QContactSimSyncer(const QString &type, QObject *parent)
    : QObject(parent), readState(Idle), mError(NoError), mSimType(type),
    SIMLabelLimit(20), SIMNumberLimit(60),
    SIMListStart(1), SIMListEnd(200), 
    addNameQuery("INSERT INTO contacts (recid, firstname, context) VALUES (:i, :fn, :c)"),
    addNumberQuery("INSERT INTO contactphonenumbers (recid, phone_type, phone_number) VALUES (:i, 1, :pn)"),
    updateNameQuery("UPDATE contacts SET firstname = :fn WHERE recid = :i"),
    updateNumberQuery("UPDATE contactphonenumbers SET phone_number = :pn WHERE recid = :i AND phone_type = 1"),
    removeNameQuery("DELETE FROM contacts WHERE recid = :i"),
    removeNumberQuery("DELETE FROM contactphonenumbers WHERE recid = :i"),
    selectNameQuery("SELECT firstname FROM contacts WHERE recid = :i"),
    selectNumberQuery("SELECT phone_number from contactphonenumbers where recid=:id and phone_type=1"),
    mPhoneBook(0), mSimInfo(0), simValueSpace(0), mReadyTimer(0)
{
    static QUuid u("b63abe6f-36bd-4bb8-9c27-ece5436a5130");
    // construct source for contact data.
    mSource.context = u;
    if (type == "SM")
        mSource.identity = "sim";
    else 
        mSource.identity = type;

    mAccess = new ContactSqlIO(this);

    mPhoneBook = new QPhoneBook( QString(), this );
    mSimInfo = new QSimInfo( QString(), this );

    connect(mPhoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
            this, SLOT(updatePhoneBook(QString,QList<QPhoneBookEntry>)));
    connect(mPhoneBook, SIGNAL(limits(QString,QPhoneBookLimits)),
            this, SLOT(updatePhoneBookLimits(QString,QPhoneBookLimits)));

    connect(mSimInfo, SIGNAL(inserted()), this, SLOT(updateSimIdentity()));
    connect(mSimInfo, SIGNAL(removed()), this, SLOT(updateSimIdentity()));

    readState = ReadingId | ReadingLimits | ReadingEntries;

    resetSqlState();

    if (mPhoneBook->storages().contains(mSimType))
        sync();
    else {
        qLog(SimPhoneBook) << mSimType << "start timeout timer and connect to phonebook ready";
        connect(mPhoneBook, SIGNAL(ready()), this, SLOT(sync()));
        mReadyTimer = new QTimer(this);
        mReadyTimer->setSingleShot(true);
        mReadyTimer->setInterval(30000); // 30 seconds
        connect(mReadyTimer, SIGNAL(timeout()), this, SLOT(simInfoTimeout()));
        mReadyTimer->start();
    }
}

/*!
  Destroys the QContactSimSyncer.
*/
QContactSimSyncer::~QContactSimSyncer()
{
}

/*!
  Resets the SQL representation of the SIM Card for this storage.  This
  includes removing any contacts that can be stored completely on the SIM,
  resetting the mapping of contact id to SIM card index, and in the case
  of the "SM" storage type, resetting the information on the available
  indexes to store contact phone numbers.

  A contact that can be stored completely on the sim is defined as
  one that is equal to a contact that has copied only the original contacts phone numbers, unique identity, and first name.

  \sa storage()
*/
void QContactSimSyncer::resetSqlState()
{
    qLog(SimPhoneBook) << mSimType << "::resetSqlState()";

    if (mSimType == "SM") {
        simValueSpace = new QValueSpaceObject("/SIM/Contacts");
        simValueSpace->setAttribute("Loaded", false);
    } else
        simValueSpace = 0;

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    syncTime = syncTime.addMSecs(-syncTime.time().msec());

    if (mAccess->startTransaction(syncTime)) {
        QPreparedSqlQuery q(QPimSqlIO::database());

        qLog(SimPhoneBook) << mSimType << "::resetSqlState() - get cardid";

        q.prepare("SELECT cardid FROM currentsimcard WHERE storage = :simtype");
        q.bindValue(":simtype", mSimType);
        q.exec();
        if (!q.next()) {
            qLog(SimPhoneBook) << mSimType << "::resetSqlState() - no card found";
            // already cleared the card.
            if (mAccess->commitTransaction())
                return;
        } else {
            QString lastActiveCard = q.value(0).toString();
            qLog(SimPhoneBook) << mSimType << "::resetSqlState() - clear" << lastActiveCard;

            q.prepare("SELECT recid, simlabelidmap.label FROM contacts JOIN simlabelidmap ON sqlid = recid WHERE cardid = :c AND storage = :s");
            q.bindValue(":s", mSimType);
            q.bindValue(":c", lastActiveCard);

            q.exec();
            QList<QUniqueId> removeTargets;
            while(q.next()) {
                QContact existing, compare;
                QUniqueId recid = QUniqueId::fromUInt(q.value(0).toUInt());
                QString label = q.value(1).toString();
                existing = mAccess->contact(recid);
                qLog(SimPhoneBook) << mSimType << "::resetSqlState() - check if" << recid.toString() << "should be removed";
                if (QContactSimContext::simLabel(existing) != label) {
                    qLog(SimPhoneBook) << mSimType << "::resetSqlState() - label mis-match" << QContactSimContext::simLabel(existing) << label;
                    // truncated label, don't remove or will lose info.
                    continue;
                }
                compare.setUid(existing.uid());
                compare.setFirstName(existing.firstName());
                compare.setLastName(existing.lastName());
                compare.setPhoneNumbers(existing.phoneNumbers());
                compare.setDefaultPhoneNumber(existing.defaultPhoneNumber());
                if (compare == existing) { // e.g. is only data that fits on the sim.
                    removeTargets.append(QUniqueId::fromUInt(q.value(0).toUInt()));
                    qLog(SimPhoneBook) << mSimType << "::resetSqlState() - remove contact" << q.value(0).toUInt();
                } else {
                    qLog(SimPhoneBook) << mSimType << "::resetSqlState() - contact == mismatch";
                }
            }

            if (mAccess->removeContacts(removeTargets))
            {
                qLog(SimPhoneBook) << mSimType << "::resetSqlState() - remove current simcard for storage";
                q.clearErrors();
                q.prepare("DELETE FROM currentsimcard WHERE storage = :simtype");
                q.bindValue(":simtype", mSimType);
                q.exec();

                qLog(SimPhoneBook) << mSimType << "::resetSqlState() - remove idmap for storage";
                q.prepare("DELETE FROM simcardidmap WHERE cardid = :c AND storage = :simtype");
                q.bindValue(":simtype", mSimType);
                q.bindValue(":c", lastActiveCard);
                q.exec();

                // only remove 'forgotten' records.
                qLog(SimPhoneBook) << mSimType << "::resetSqlState() - remove label idmap for storage";
                q.prepare("DELETE FROM simlabelidmap WHERE sqlid = :r");
                foreach (QUniqueId id, removeTargets) {
                    q.bindValue(":r", id.toUInt());
                    q.exec();
                }

                if (q.errorCount() == 0 && mAccess->commitTransaction())
                    return;
            }
        }
    }
    qLog(SimPhoneBook) << mSimType << "::resetSqlState() - abort transaction";
    mAccess->abortTransaction();
}

/*!
  Returns the error code for the last synchronization operation.
  \sa errorString()
*/
QContactSimSyncer::Error QContactSimSyncer::error() const
{
    return mError;
}

/*!
  Returns a translated string describing the error for the last
  synchronization operation.  If no error occurred will return
  an empty string.
  \sa error()
*/
QString QContactSimSyncer::errorString() const
{
    switch(mError) {
        case NoError:
            break;
        case UknownError:
            return tr("An unknown error occurred.");
    }
    return QString();
}

/*!
  Returns the current state of synchronization.
*/
int QContactSimSyncer::state() const
{
    return readState;
}

/*!
  Instructs the object to start syncing the SIM storage for this object.

  It is not necessary to call this function as it is done so from
  the constructor.
*/
void QContactSimSyncer::sync()
{
    qLog(SimPhoneBook) << mSimType << "::sync()";
    if (mReadyTimer)
    {
        disconnect(mPhoneBook, SIGNAL(ready()), this, SLOT(sync()));
        disconnect(mReadyTimer, SIGNAL(timeout()), this, SLOT(simInfoTimeout()));
        mReadyTimer->stop();
    }
    mPhoneBook->getEntries(mSimType);
    mPhoneBook->requestLimits(mSimType);
    updateSimIdentity();
}

/*!
    Called if no indication of the SIM state is received for an
    internally set period of time (5 seconds).

    Marks the sync as complete, but leaves door open for slow 
    SIM responders to add info.
*/
void QContactSimSyncer::simInfoTimeout()
{
    if (simValueSpace) simValueSpace->setAttribute("Loaded", true);
    emit done(false);

    sync();
}

/*!
  Aborts the current sync operation, if possible
*/
void QContactSimSyncer::abort()
{
    // TODO, currently cannot abort.  However would be reasonable to
    // set up a mechanism such that if the sql update hasn't occurred, to
    // mark that it should no longer be attempted.
}

/*!
  If there already exists a mapped unique identity for the given \a index,
  returns that identity.  Otherwise returns a null identity.
*/
QUniqueId QContactSimSyncer::simCardId(int index) const
{
    return QContactSimContext::simCardId(mActiveCard, mSimType, index);
}

/*!
  Sets the given \a index for the current storage to be mapped to the given
  \a identity.
*/
void QContactSimSyncer::setSimCardId(int index, const QUniqueId &identity) const
{
    QContactSimContext::setSimCardId(mActiveCard, mSimType, index, identity);
}

/*!
  Updates the QContactSimSyncer's record of the SIM identity.  If
  both the entries and limits are already known will also update the
  SQL entries.
*/
void QContactSimSyncer::updateSimIdentity()
{
    qLog(SimPhoneBook) << mSimType << "::updateSimIdentity() - " << mActiveCard << "to" << mSimInfo->identity();
    // only react on changes to the identity.  Since the system
    // starts off in the 'sim empty' case we don't have to treat
    // it in any special way, it is still ignored.
    if (mActiveCard == mSimInfo->identity()) {
        if (simValueSpace)
            simValueSpace->setAttribute("Loaded", true);
        return;
    }

    setSimIdentity(mSimInfo->identity(), mSimInfo->insertedTime());
}

void QContactSimSyncer::setSimIdentity(const QString &id, const QDateTime &time)
{
    qLog(SimPhoneBook) << mSimType << "::setSimIdentity() - " << mActiveCard << "to" << id;
    mActiveCard = id;
    mInsertTime = time;

    if (mActiveCard.isEmpty())
    {
        resetSqlState();
        return;
    }

    // TODO, may need to put in a 'no sim card' handler here.

    if (readState == ReadingId || readState == Idle) {
        readState = UpdatingSqlTable;
        updateSqlEntries();
    } else {
        readState &= ~ReadingId;
    }
    emit stateChanged(readState);
}

/*!
  If the given \a store is equal to the storage specified for this object,
  updates the QContactSimSyncer's record of contact entries to the given
  \a list.

  If both the SIM identity and limits are already known will also update
  the SQL entries.
*/
void QContactSimSyncer::updatePhoneBook( const QString &store, const QList<QPhoneBookEntry> &list )
{
    if (store != mSimType)
        return;

    qLog(SimPhoneBook) << mSimType << "::updatePhoneBook() - " << list.count() << "entries";

    phoneData = list;

    if (readState == ReadingEntries || readState == Idle) {
        readState = UpdatingSqlTable;
        updateSqlEntries();
    } else {
        readState &= ~ReadingEntries;
    }
    emit stateChanged(readState);
}

/*!
  If the given \a store is equal to the storage specified for this object,
  updates the QContactSimSyncer's record of the limits to the given \a value.

  If both the SIM idenity and entries are already known will also update
  the SQL entries.
*/
void QContactSimSyncer::updatePhoneBookLimits( const QString &store, const QPhoneBookLimits &value )
{
    if (store != mSimType)
        return;

    SIMNumberLimit = value.numberLength();
    SIMLabelLimit = value.textLength();
    SIMListStart = value.firstIndex();
    SIMListEnd = value.lastIndex();

    qLog(SimPhoneBook) << mSimType << "::updatePhoneBookLimits()";

    if (readState == ReadingLimits || readState == Idle) {
        readState = UpdatingSqlTable;
        updateSqlEntries();
    } else {
        readState &= ~ReadingLimits;
    }
    emit stateChanged(readState);
}

/*!
    Sets the synchronization state to Idle and emits the done() signal
    with the given \a error.
*/
void QContactSimSyncer::setComplete(bool error)
{
    qLog(SimPhoneBook) << mSimType << "::setComplete(" << error <<")";
    readState = Idle;
    emit stateChanged(readState);
    if (simValueSpace) simValueSpace->setAttribute("Loaded", true);
    emit done(error);

    disconnect(mPhoneBook, SIGNAL(entries(QString,QList<QPhoneBookEntry>)),
            this, SLOT(updatePhoneBook(QString,QList<QPhoneBookEntry>)));
    disconnect(mPhoneBook, SIGNAL(limits(QString,QPhoneBookLimits)),
            this, SLOT(updatePhoneBookLimits(QString,QPhoneBookLimits)));

    disconnect(mSimInfo, SIGNAL(inserted()), this, SLOT(updateSimIdentity()));
    disconnect(mSimInfo, SIGNAL(removed()), this, SLOT(updateSimIdentity()));

    if (mReadyTimer)
    {
        disconnect(mPhoneBook, SIGNAL(ready()), this, SLOT(sync()));
        disconnect(mReadyTimer, SIGNAL(timeout()), this, SLOT(simInfoTimeout()));
        mReadyTimer->stop();
    }
}

/*!
  Merges entries in the SIM contact list that have equivalent labels into
  single contacts, and returns the resulting list.  Phone type
  specifies will be stripped off the end of the entry text to form
  the label and phone number type.  

  While constructing the list will also update the mapping of contact 
  identity to SIM storage index.

  \sa QContactSimContext::parseSimLabel()
*/
QList<QContact> QContactSimSyncer::mergedContacts() const
{
    QMap<QString, QContact> result;
    QStringList pfields = QContactFieldDefinition::fields("phone");

    QSqlQuery q(QPimSqlIO::database());

    foreach(QPhoneBookEntry entry, phoneData)
    {
        QString text = entry.text();
        QString number = entry.number();

        QString field, label;

        QContact c = QContactSimContext::parseSimLabel(text, label, field);
        if (result.contains(label)) {
            c = result[label];
            q.bindValue(":c", mActiveCard);
            setSimCardId(entry.index(), c.uid());
        } else {
            QUniqueId id = QContactSimContext::idForLabel(mActiveCard, mSimType, label);
            if (id.isNull())
            {
                // no previous mapping for this label, add one.
                static const QUuid appScope("b63abe6f-36bd-4bb8-9c27-ece5436a5130");
                QUniqueIdGenerator g(appScope);
                id = g.createUniqueId();
                QContactSimContext::setIdForLabel(mActiveCard, mSimType, label, id);
            }

            setSimCardId(entry.index(), id);
            c.setUid(id);
        }

        if (field.isEmpty())
            field = "homephone";
        QContactFieldDefinition def(field);

        if (def.value(c).toString().isEmpty()) {
            def.setValue(c, number);
        } else {
            foreach(QString candidate, pfields) {
                QContactFieldDefinition alt(field);
                if (alt.value(c).toString().isEmpty()) {
                    alt.setValue(c, number);
                    break;
                }
            }
        }

        result.insert(label, c);
    }
    return result.values();

    // want label to id mapping, most matched id?
}


/*!
  Updates the SQL entries to match those read from the SIM card.
*/
void QContactSimSyncer::updateSqlEntries()
{
    if (mActiveCard.isEmpty()) // no card, no contacts, no limits.
        return;

    qLog(SimPhoneBook) << mSimType << "::updateSqlEntries()";

    // we want to work in utc, siminfo works in local.  convert.
    QDateTime syncTime = QTimeZone::current().toUtc(mInsertTime);
    syncTime = syncTime.addMSecs(-syncTime.time().msec());

    /* Could do a proper merge, but list is small enough that it
       is hardly justified.
    */
    if (mAccess->startSyncTransaction(mSource, syncTime)) {
        QPreparedSqlQuery q(QPimSqlIO::database());

        QList<QContact> currentList = mergedContacts();

        // 2. Update any that are.
        QMutableListIterator<QContact> it(currentList);
        while(it.hasNext()) {
            QContact c = it.next();
            if (mAccess->exists(c.uid())) {
                it.remove();
                QContact old = mAccess->contact(c.uid());
                // sim, remove all phone numbers.
                old.setPhoneNumbers(c.phoneNumbers());
                mAccess->updateContact(old);
            }
        }

        // 3. Add new contacts from sim list.
        foreach(QContact c, currentList) {
            if (mAccess->addContact(c, mSource, false).isNull()) {
                mAccess->abortSyncTransaction();
                setComplete(true);
                return;
            }
        }

        // update current simcard id info
        q.prepare("INSERT INTO currentsimcard (cardid, storage, firstindex, lastindex, labellimit, numberlimit, loaded) VALUES (:c, :s, :f, :l, :la, :nu, :lo)");
        q.bindValue(":c", mActiveCard);
        q.bindValue(":s", mSimType);
        q.bindValue(":f", SIMListStart);
        q.bindValue(":l", SIMListEnd);
        q.bindValue(":la", SIMLabelLimit);
        q.bindValue(":nu", SIMNumberLimit);
        q.bindValue(":lo", true);

        if (!q.exec()) {
            mAccess->abortSyncTransaction();
            setComplete(true);
            return;
        }

        if (!mAccess->commitSyncTransaction()) {
            mAccess->abortSyncTransaction();
            setComplete(true);
            return;
        }
    }
    setComplete(false);
}
