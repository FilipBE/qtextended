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
#include "buddysyncertask.h"

#include <qtopiaserverapplication.h>
#include <QContactModel>
#include <QContactFieldDefinition>
#include <QCollectivePresenceInfo>
#include <QCollectivePresence>
#include <QCommServiceManager>
#include <QtopiaServiceRequest>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlQuery>
#include <QValueSpaceObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QtopiaSql>

/*
   Function duplicated from QPimSqlIO::database, so we don't export
   a private symbol.
*/
static QSqlDatabase PIMDatabase()
{
    static QSqlDatabase *pimdb = 0;
    if (!pimdb)
        pimdb = new QSqlDatabase( QtopiaSql::instance()->systemDatabase() );
    if (!pimdb->isOpen())
        qWarning() << "Failed to open pim database";
    return *pimdb;
}

/*
   Listen for presence changes and update QContactModel.

   Also, automatically create new contacts from buddy lists
   if we don't recognize them.
*/

BuddySyncerTask::BuddySyncerTask(QObject *parent)
: QObject(parent)
{
    mModel = new QContactModel();
    mServiceManager = new QCommServiceManager();

    connect(mServiceManager, SIGNAL(servicesChanged()), this, SLOT(providersChanged()));

    /* changes under this key will cause any QContactSqlIOs to reset their cache */
    const QString valueSpaceKey(QLatin1String("PIM/Contacts/Presence"));
    mVSObject = new QValueSpaceObject(valueSpaceKey);
    mVSValue = 1;

    /* clear out stale presence information */
    // XXX - not needed when the table is volatile
    clearAllPresenceInformation();

    /* and kick off any existing providers */
    providersChanged();
}

BuddySyncerTask::~BuddySyncerTask()
{
    foreach (QCollectivePresence *presence, mPresences)
        delete presence;
    delete mModel;
    delete mServiceManager;
}

void BuddySyncerTask::providersChanged()
{
    QSet<QString> providers = mServiceManager->supports<QCollectivePresence>().toSet();
    qLog(Telepathy) << "BuddySyncer: providers changed:" << providers;

    /* See what we don't already have */
    QSet<QString> added = providers - mPresenceNames;
    QSet<QString> removed = mPresenceNames - providers;

    foreach(QString provider, added) {
        qLog(Telepathy) << "BuddySyncer: provider" << provider << "added";
        QCollectivePresence *presence = new QCollectivePresence(provider);

        mPresences.append(presence);

        connect(presence, SIGNAL(peerPresencesChanged(QStringList)), this, SLOT(presenceChanged(QStringList)));
        connect(presence, SIGNAL(pendingPublishRequestsChanged()), this, SLOT(pendingPublishRequestsChanged()));

        // Do the initial processing as well
        presenceChanged(presence, presence->subscribedPeers());
        pendingPublishRequestsChanged(presence);
    }

    /* Now check removed */
    if (removed.count() > 0) {
        QList<QCollectivePresence*>::iterator it = mPresences.begin();

        while (it != mPresences.end()) {
            if (removed.contains((*it)->groupName())) {
                qLog(Telepathy) << "BuddySyncer: provider" << (*it)->groupName() << "removed";

                // Remove the presence information from this provider
                clearPresenceInformation((*it)->groupName());

                delete *it;
                it = mPresences.erase(it);
            } else {
                ++it;
            }
        }
    }

    mPresenceNames = providers;
}

void BuddySyncerTask::presenceChanged(QStringList list)
{
    // Retrieve which actual QCollectivePresence let us know..
    QCollectivePresence *presence = qobject_cast<QCollectivePresence*>(sender());

    if (presence)
        presenceChanged(presence, list);
}

void BuddySyncerTask::presenceChanged(QCollectivePresence *presence, QStringList list)
{
    QString provider = presence->groupName();

    /* Build up the PresenceInfo blobs */
    QList<QCollectivePresenceInfo> presences;

    qLog(Telepathy) << "Presence " << provider << " changed:" << list;
    foreach(QString uri, list) {
        QCollectivePresenceInfo info = presence->peerInfo(uri);
        if (!info.isNull()) {
            presences.append(info);
        }
    }

    QList<QContact> added;
    QList<QContact> updated;

    /* Find the right contact field for this provider, if any */
    QStringList chatFields = QContactFieldDefinition::fields("chat");
    QString providerField;
    foreach (QString chatField, chatFields) {
        const QContactFieldDefinition field(chatField);
        if (field.provider() == provider) {
            providerField = chatField;
            break;
        }
    }

    /* Check each URI to see if we have a matching contact */
    foreach(QCollectivePresenceInfo p, presences) {
        QString email = p.properties()["email"].toString();
        bool matchedChat = false;

        /* Ideally we could get a list of matches and decide ourselves */
        QContact match = mModel->matchChatAddress(p.uri(), provider);

        if (!match.uid().isNull())
            matchedChat = true;

        /* If that didn't work, see if we can match the email address, if it's specified */
        if (match.uid().isNull() && !email.isEmpty()) {
            match = mModel->matchEmailAddress(email);
        }

        /* Try treating the uri as an email address if that didn't work */
        if (match.uid().isNull()) {
            match = mModel->matchEmailAddress(p.uri());
        }

        /* Now try matching on the label */
        if (match.uid().isNull() && !p.displayName().isEmpty()) {
            /* There's a slight problem that the display name may be in reverse order */
            QContact parsed = QContact::parseLabel(p.displayName());
            match = mModel->contact(mModel->match(QContactModel::Label, parsed.label(), Qt::MatchFlags(Qt::MatchContains)).value(0));
        }

        /* If nothing matched, create a new contact */
        if (match.uid().isNull()) {
            /* Hopefully we have a nice name to use */
            QString label = p.displayName();
            if (label.isEmpty())
                label = p.uri(); // <-- not a nice name to use

            QContact newContact = QContact::parseLabel(label);

            // Add this address as a chat address, if we handle this provider
            if (!providerField.isEmpty()) {
                const QContactFieldDefinition field(providerField);
                field.setValue(newContact, p.uri());

                qLog(Telepathy) << "Uri" << p.uri() << "(" << p.displayName() << ") didn't match existing contacts, adding new";

                QImage i(p.avatar());
                newContact.changePortrait(i);
                newContact.setCustomField("buddysyncerportraitfile", newContact.portraitFile());
                if (!email.isEmpty())
                    newContact.insertEmail(email);

                added.append(newContact);
            } else {
                qLog(Telepathy) << "Ignoring presence info for unhandled provider" << QCollective::encodeUri(provider, p.uri()) << "(" << p.displayName() << ")";
            }
        } else {
            /* Existing contact - see if we can add anything to it... */
            bool changed = false;
            QCollectivePresenceInfo oldPresence = presenceInformation(provider, p.uri());

            qLog(Telepathy) << "Uri " << p.uri() << "matched existing contact" << match.label();

            if (!matchedChat && !providerField.isEmpty()) {
                // We matched a contact that didn't have a chat address defined
                // (e.g. we found a corresponding email address), so add the
                // chat address as well
                const QContactFieldDefinition field(providerField);
                field.setValue(match, p.uri());
            }

            // Unfortunately we can't use portraitFile() to match against, so
            // use the custom fields.
            if (match.portraitFile().isEmpty()) {
                if (!p.avatar().isEmpty()) {
                    qLog(Telepathy) << " adding avatar as portrait";
                    QImage i(p.avatar());
                    match.changePortrait(i);
                    match.setCustomField("buddysyncerportraitfile", match.portraitFile());
                    changed = true;
                } else {
                    if (!match.customField("buddysyncerportraitfile").isEmpty()) {
                        match.removeCustomField("buddysyncerportraitfile");
                        changed = true;
                    }
                }
            } else {
                /* If they previously used a presence derived portrait, see if we need to change */
                if (match.portraitFile() == match.customField("buddysyncerportraitfile")) {
                    if (oldPresence.avatar() != p.avatar()) {
                        /* Note that this can clear the portrait as well */
                        qLog(Telepathy) << " updating avatar portrait";
                        QImage i(p.avatar());
                        match.changePortrait(i);
                        match.setCustomField("buddysyncerportraitfile", match.portraitFile());
                        changed = true;
                    }
                } else {
                    /* They've moved on.. */
                    match.removeCustomField("buddysyncerportraitfile");
                    changed = true;
                }
            }

            if (!email.isEmpty()) {
                if (!match.emailList().contains(email)) {
                    match.insertEmail(email);
                    changed = true;
                }
            }

            // See if the display name can be updated
            if (!p.displayName().isEmpty() && (match.label() == p.uri() || match.label() == oldPresence.displayName())) {
                if (match.label() != p.displayName()) {
                    qLog(Telepathy) << "Updating contact label from "<< match.label() << "to" << p.displayName();
                    match = QContact::parseLabel(p.displayName(), match);
                    changed = true;
                }
            }

            if (changed)
                updated.append(match);
        }
    }

    /* Now do the actual changes */
    if (updated.count() > 0 || added.count() > 0) {
        bool commit = mModel->startSyncTransaction(QDateTime::currentDateTime());
        foreach(QContact a, added) {
            mModel->addContact(a); // XXX needs a new context
        }

        foreach(QContact u, updated) {
            mModel->updateContact(u);
        }

        if (commit)
            mModel->commitSyncTransaction();
    }

    // and push the presence to QContactModel
    setPresenceInformation(provider, presences);
}

void BuddySyncerTask::pendingPublishRequestsChanged()
{
    // Retrieve which actual QCollectivePresence let us know..
    QCollectivePresence *presence = qobject_cast<QCollectivePresence*>(sender());

    if (presence)
        pendingPublishRequestsChanged(presence);
}

void BuddySyncerTask::pendingPublishRequestsChanged(QCollectivePresence *p)
{
    /*
        send a QCOP off to address book - let it query the actual uris, since
        authorizing/denying a uri results in another change message and another
        QSR here, you get way too many messages.
    */
    QtopiaServiceRequest req("Contacts", "peerPublishRequest(QString)");
    req << p->groupName();
    req.send();
}


void BuddySyncerTask::setPresenceInformation(const QString& provider, QList<QCollectivePresenceInfo> presences)
{
    /*
        Note:  The contactpresence table could be constructed without a recid field, but having one
        (and manually updating it when presence or phone numbers change) makes the select queries
        a lot simpler (avoids another LEFT JOIN on contactphonenumbers, a GROUP BY and a HAVING, or
        a sub select)
        Since selects happen more often than presence updates, and that happens more often than
        phone number updates, this should be ok.

        Also, some performance testing over 1000 contacts shows that having one table with
        recid & uri is nearly 100x faster than a combination of joins/subselects (70ms versus 6000ms).

        We also store the presence information with a UID of zero, in case we don't have any current
        contacts with this presence information, but we add one (or change an existing one).

    */
    QSqlDatabase db = PIMDatabase();

    QSqlQuery deletePresence("DELETE FROM contactpresence WHERE uri=:u", db);
    QSqlQuery setPhonePresence("REPLACE INTO contactpresence(recid,uri,status,statusstring,message,displayname,updatetime,capabilities,avatar) SELECT contactphonenumbers.recid,:u,:t,:st,:m,:dn,:ut,:cap,:a FROM contactphonenumbers WHERE contactphonenumbers.phone_number=:pn", db);
    QSqlQuery stashPresence("REPLACE INTO contactpresence(recid,uri,status,statusstring,message,displayname,updatetime,capabilities,avatar) VALUES(0,:u,:t,:st,:m,:dn,:ut,:cap,:a)", db);
    QSqlQuery setCustomPresence("REPLACE INTO contactpresence(recid,uri,status,statusstring,message,displayname,updatetime,capabilities,avatar) SELECT contactcustom.recid,:u,:t,:st,:m,:dn,:ut,:cap,:a FROM contactcustom WHERE contactcustom.fieldname=:cn AND contactcustom.fieldvalue=:fv", db);

    /* Early out if no presences.. */
    if (presences.count() == 0)
        return;

    /* Start a transaction .. */
    bool commit = db.transaction();

    if (!commit)
        qLog(Telepathy) << "Failed to get database transaction for presence update:" << db.lastError().text();

    /* Pesky custom fields */
    QStringList fields = QContactFieldDefinition::fields("chat");
    QList<QContactFieldDefinition> customFields;

    foreach(QString field, fields) {
        QContactFieldDefinition def(field);
        if (def.provider() == provider) {
            QContactModel::Field f = QContactModel::identifierField(def.id());
            /* We assume that valid fields are handled by the contactphonenumber query... */
            if (f == QContactModel::Invalid)
                customFields.append(def);
        }
    }

    /* Now create them all */
    foreach (QCollectivePresenceInfo info, presences) {
        if (!info.isNull()) {
            QString uri = QCollective::encodeUri(provider,info.uri());

            qLog(Telepathy) << "updating presence info for: "<< uri << ", type" << info.presenceType();

            deletePresence.bindValue(":u", uri);
            deletePresence.exec();

            /* If the type is "None", we don't store it to the db */
            if (info.presenceType() != QCollectivePresenceInfo::None) {
                setPhonePresence.bindValue(":u", uri);
                setPhonePresence.bindValue(":t", info.presenceType());
                setPhonePresence.bindValue(":st", info.presence());
                setPhonePresence.bindValue(":m", info.message());
                setPhonePresence.bindValue(":dn", info.displayName());
                setPhonePresence.bindValue(":ut", info.lastUpdateTime());
                setPhonePresence.bindValue(":cap", info.capabilities().join(QLatin1String(",")));
                setPhonePresence.bindValue(":a", info.avatar());
                setPhonePresence.bindValue(":pn", info.uri()); // SIP matches on naked 'URI's
                setPhonePresence.exec();

                foreach(QContactFieldDefinition def, customFields) {
                    /* Check that this field definition has the same provider */
                    if (def.provider() == provider) {
                        setCustomPresence.bindValue(":u", uri);
                        setCustomPresence.bindValue(":t", info.presenceType());
                        setCustomPresence.bindValue(":st", info.presence());
                        setCustomPresence.bindValue(":m", info.message());
                        setCustomPresence.bindValue(":dn", info.displayName());
                        setCustomPresence.bindValue(":ut", info.lastUpdateTime());
                        setCustomPresence.bindValue(":cap", info.capabilities().join(QLatin1String(",")));
                        setCustomPresence.bindValue(":a", info.avatar());
                        setCustomPresence.bindValue(":cn", def.id()); // XXX this is the custom field name
                        setCustomPresence.bindValue(":fv", info.uri());
                        setCustomPresence.exec();
                    }
                }

                stashPresence.bindValue(":u", uri);
                stashPresence.bindValue(":t", info.presenceType());
                stashPresence.bindValue(":st", info.presence());
                stashPresence.bindValue(":m", info.message());
                stashPresence.bindValue(":dn", info.displayName());
                stashPresence.bindValue(":ut", info.lastUpdateTime());
                stashPresence.bindValue(":cap", info.capabilities().join(QLatin1String(",")));
                stashPresence.bindValue(":a", info.avatar());
                stashPresence.exec();
            }
        }
    }

    if (commit) {
        bool done = db.commit();
        if (!done)
            qLog(Sql) << "Failed to commit database transaction for presence update:" << db.lastError().text();
    }

    // Now update any models [this should be pickier]
    mVSObject->setAttribute(QLatin1String("SerialNumber"), ++mVSValue);
    QValueSpaceObject::sync();
}

QCollectivePresenceInfo BuddySyncerTask::presenceInformation(const QString& provider, const QString& identity)
{
    /*
        The presence table can have multiple copies of the presence for uri.  In theory they should all
        be the same, however.  We grab the one with recid 0, since the combination is indexed.
    */
    QCollectivePresenceInfo ret;

    if (!identity.isEmpty()) {
        QSqlDatabase db = PIMDatabase();
        QSqlQuery getPresenceQuery("SELECT status,statusstring,displayname,message,updatetime,capabilities,avatar FROM contactpresence WHERE uri=:u AND recid=0", db);
        QString uri = QCollective::encodeUri(provider, identity);
        getPresenceQuery.bindValue(":u", uri);
        getPresenceQuery.exec();
        if (getPresenceQuery.next()) {
            ret.setUri(identity);
            ret.setPresence(getPresenceQuery.value(1).toString(), (QCollectivePresenceInfo::PresenceType) getPresenceQuery.value(0).toUInt());
            ret.setDisplayName(getPresenceQuery.value(2).toString());
            ret.setMessage(getPresenceQuery.value(3).toString());
            ret.setLastUpdateTime(getPresenceQuery.value(4).toDateTime());
            ret.setCapabilities(getPresenceQuery.value(5).toString().split(QLatin1Char(','), QString::SkipEmptyParts));
            ret.setAvatar(getPresenceQuery.value(6).toString());
        }
    }
    return ret;
}

void BuddySyncerTask::clearAllPresenceInformation()
{
    /* Called at start up .. */
    QSqlDatabase db = PIMDatabase();
    QSqlQuery clearPresenceQuery("DELETE FROM contactpresence", db);
    clearPresenceQuery.exec();

    // Push
    mVSObject->setAttribute(QLatin1String("SerialNumber"), ++mVSValue);
    QValueSpaceObject::sync();
}

void BuddySyncerTask::clearPresenceInformation(const QString& provider)
{
    QSqlDatabase db = PIMDatabase();
    QSqlQuery clearPresenceQuery("DELETE FROM contactpresence WHERE uri GLOB :u", db);

    // This might be a little brittle if the collective encoding changes
    clearPresenceQuery.bindValue(":u", QCollective::encodeUri(provider, "*"));
    clearPresenceQuery.exec();

    // Push
    mVSObject->setAttribute(QLatin1String("SerialNumber"), ++mVSValue);
    QValueSpaceObject::sync();
}


QTOPIA_TASK(BuddySyncer, BuddySyncerTask);
