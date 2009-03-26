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

#ifndef QSIMCONTEXT_P_H
#define QSIMCONTEXT_P_H

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

#include <qtopiasql.h>
#include <qpimsource.h>
#include <qcontact.h>
#include <qphonebook.h>
#include <qcontactmodel.h>

#include "qpreparedquery_p.h"

class ContactSqlIO;
class QSimInfo;
class QContactSimSyncer;
class QValueSpaceItem;
class QTOPIAPIM_EXPORT QContactSimContext : public QContactContext
{
    Q_OBJECT
public:
    // could have constructor protected/private with friends class.
    QContactSimContext(QObject *parent, QObject *access);
    virtual ~QContactSimContext();

    QIcon icon() const; // default empty
    QString description() const;
    QString title() const;
    QString title(const QPimSource &) const;

    bool editable() const; // default true
    bool editable(const QUniqueId &) const; // default true

    QPimSource defaultSource() const;
    QPimSource serviceNumbersSource() const;

    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> visibleSources() const;
    QSet<QPimSource> sources() const;
    QUuid id() const;

    using QContactContext::exists;
    bool exists(const QUniqueId &) const;
    QPimSource source(const QUniqueId &) const;

    bool updateContact(const QContact &);
    bool removeContact(const QUniqueId &);
    QUniqueId addContact(const QContact &, const QPimSource &);

    bool importContacts(const QPimSource &, const QList<QContact> &);
    QList<QContact> exportContacts(const QPimSource &, bool &) const;
    QContact exportContact(const QUniqueId &, bool &) const;

    QString card() const;
    int firstIndex() const;
    int lastIndex() const;
    int labelLimit() const;
    int numberLimit() const;
    int nextFreeIndex() const;
    int indexCount() const
    { return lastIndex()-firstIndex()+1; }
    int freeIndexCount() const
    { return indexCount() - usedIndexCount(); }
    int usedIndexCount() const;


    // labels considered the unique id for a sim.
    QUniqueId idForLabel(const QString &storage, const QString &label) const
    { return idForLabel(card(), storage, label); }
    QString labelForId(const QString &storage, const QUniqueId &id) const
    { return labelForId(card(), storage, id); }
    void setIdForLabel(const QString &storage, const QString &label, const QUniqueId &id)
    { setIdForLabel(card(), storage, label, id); }
    void setLabelForId(const QString &storage, const QUniqueId &id, const QString &label)
    { setLabelForId(card(), storage, id, label); }
    void clearLabelsForId(const QString & storage, const QUniqueId &id)
    { clearLabelsForId(card(), storage, id); }

    static QUniqueId idForLabel(const QString &card, const QString &storage, const QString &label);
    static QString labelForId(const QString &card, const QString &storage, const QUniqueId &id);
    static void setIdForLabel(const QString &card, const QString &storage, const QString &label, const QUniqueId &);
    static void setLabelForId(const QString &card, const QString &storage, const QUniqueId &id, const QString &label);
    static void clearLabelsForId(const QString &card, const QString &storage, const QUniqueId &id);

    QUniqueId simCardId(const QString &storage, int index) const
    { return simCardId(card(), storage, index); }

    void setSimCardId(const QString &storage, int index, const QUniqueId &id) const
    { setSimCardId(card(), storage, index, id); }

    QList<int> simCardIndexes(const QString &storage, const QUniqueId &id) const
    { return simCardIndexes(card(), storage, id); }

    static QUniqueId simCardId(const QString &, const QString &, int);
    static void setSimCardId(const QString &, const QString &, int, const QUniqueId &);
    static QList<int> simCardIndexes(const QString &, const QString &, const QUniqueId &);


    bool loadingSim() const;

    static QContact parseSimLabel(const QString &simLabel, QString &label, QString &field);
    static QString simLabel(const QContact &contact);
signals:
    void simLoaded(const QPimSource &);

private slots:
    void checkSimLoaded();

private:
    QContact simContact(const QUniqueId &u) const;
    void addEntries(const QUniqueId &id, const QMap<QString, QString> &, const QString &);
    void removeEntries(const QList<int> &);
    QMap<QString, QString> mapPhoneNumbers(const QContact &contact, QString &label);

    ContactSqlIO *mAccess;
    QPhoneBook *mPhoneBook;
    QValueSpaceItem *simValueSpace;

    mutable QPreparedSqlQuery selectNameQuery;
    mutable QPreparedSqlQuery selectNumberQuery;
    mutable QPreparedSqlQuery cardIdQuery;
    mutable QPreparedSqlQuery firstIndexQuery;
    mutable QPreparedSqlQuery lastIndexQuery;
    mutable QPreparedSqlQuery labelLimitQuery;
    mutable QPreparedSqlQuery numberLimitQuery;
    mutable QPreparedSqlQuery cardLoadedQuery;

    static QPreparedSqlQuery selectIdQuery;
    static QPreparedSqlQuery insertIdQuery;
    static QPreparedSqlQuery updateIdQuery;
};
#endif
