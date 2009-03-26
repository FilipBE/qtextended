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

#include <qsimcontext_p.h>
#include <qsiminfo.h>
#include <qcontactsqlio_p.h>
#include <qtopialog.h>

#include <qfielddefinition.h>

#include <QValueSpaceItem>

#include <QDebug>
/***************
 * CONTEXT
 **************/

const char *userEntries = "SM";
const char *serviceNumbers = "SN";

struct ExtensionMap
{
    QContactModel::Field type;
    const char * text;
};

// these are english versions.  should translate?
// also, first ones are Qtopia export extensions.  Could add
// another list for fuzzy matching.
// extensions the same length for label consistency
static const ExtensionMap SIMextensions[] = {
    { QContactModel::HomePhone, "/hp" }, // no tr
    { QContactModel::HomeMobile, "/hm" }, // no tr
    { QContactModel::HomeFax, "/hf" }, // no tr
    { QContactModel::BusinessPhone, "/bp" }, // no tr
    { QContactModel::BusinessMobile, "/bm" }, // no tr
    { QContactModel::BusinessFax, "/bf" }, // no tr
    { QContactModel::BusinessPager, "/bg" }, // no tr
    { QContactModel::OtherMobile, "/m" }, // no tr
    { QContactModel::OtherPhone, "/o" }, // no tr
    { QContactModel::OtherFax, "/f" }, // no tr

    { QContactModel::HomePhone, "/h" }, // no tr
    { QContactModel::BusinessPhone, "/w" }, // no tr


    { QContactModel::Invalid, 0 }
};

struct RegExpExtensionMap
{
    QContactModel::Field type;
    QRegExp expression;
};

static const RegExpExtensionMap SIMRegExpExtensions[] = {
    { QContactModel::HomePhone, QRegExp("[ /](h|hp|home)$") }, // no tr
    { QContactModel::OtherMobile, QRegExp("[ /](m|hm|mob)$") }, // no tr
    { QContactModel::OtherPhone, QRegExp("[ /](o|other)$") }, // no tr
    { QContactModel::BusinessPhone, QRegExp("[ /](b|w|wk|bp|wp|work)$") }, // no tr
    { QContactModel::BusinessFax, QRegExp("[ /](bo|wo)$") }, // no tr
    { QContactModel::BusinessMobile, QRegExp("[ /](bm|wm)$") }, // no tr
    { QContactModel::BusinessPager, QRegExp("[ /](bpa|wpa)$") }, // no tr
    { QContactModel::Invalid, QRegExp() }
};

static QString typeToSIMExtension(QContactModel::Field type)
{
    const ExtensionMap *i = SIMextensions;
    while(i->type != QContactModel::Invalid) {
        if (i->type == type)
            return i->text;
        ++i;
    }
    return QString();
}

static QContactModel::Field SIMExtensionToType(QString &label)
{
    // doesn't guess at ones we don't write.
    const ExtensionMap *i = SIMextensions;
    QString llabel = label.toLower();
    while(i->type != QContactModel::Invalid) {
        QString e(i->text);
        if (llabel.right(e.length()) == e) {
            label = label.left(label.length() - e.length());
            return i->type;
        }
        ++i;
    }

    // failed of the Qtopia map, try some regexp.
    const RegExpExtensionMap *r = SIMRegExpExtensions;
    while(r->type != QContactModel::Invalid) {
        if (r->expression.indexIn(llabel) != -1) {
            label = label.left(label.length() - r->expression.matchedLength());
            return r->type;
        }
        ++r;
    }

    return QContactModel::Invalid;
}

/*!
  \class QContactSimContext
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
  \internal
  \ingroup pim
  \brief The QContactSimContext class provides functions to edit and maintain SIM contacts in the PIM storage.

  This includes the functions that make sure the SIM entries are updated along
  with the SQL entries when adding new contacts and convenience functions
  for determining free indexes on the current SIM card.

  This class is distinct from the QPhoneBook class in that it manages
  the PIM SQL contacts as a mirror to changes made to the SIM card.

*/

/*!
  Parses the given \a simLabel, and sets the given \a label to the 
  name part and the given \a field to the best matching phone field for
  the type part of the \a simLabel.

  Returns a contact with the appropriate label fields set correctly.
*/
QContact QContactSimContext::parseSimLabel(const QString &simLabel, QString &label, QString &field)
{
    QContact result;

    label = simLabel;
    field = QContactModel::fieldIdentifier(SIMExtensionToType(label));

    // split label into first and last names.
    label = label.simplified();
    int spaceIndex = label.indexOf(' ');
    if (spaceIndex > 0) {
        result.setFirstName(label.left(spaceIndex));
        result.setLastName(label.mid(spaceIndex+1));
    } else {
        result.setFirstName(label);
    }
    return result;
}

/*!
    Returns a label appropriate for using in the SIM phone book.
*/
QString QContactSimContext::simLabel(const QContact &contact)
{
    if (!contact.firstName().isEmpty() && !contact.lastName().isEmpty())
        return contact.firstName() + " " + contact.lastName();
    return contact.firstName();
}

static QString fieldExtension(const QString &field)
{
    return typeToSIMExtension(QContactModel::identifierField(field));
}

/*!
  Constructs a new QContactSimContext with the given \a parent and
  \a access to the SQL storage.  The given \a access must be able
  to cast to ContactSqlIO.
*/
QContactSimContext::QContactSimContext(QObject *parent, QObject *access)
    : QContactContext(parent),
    selectNameQuery("SELECT firstname FROM contacts WHERE recid = :i"),
    selectNumberQuery("SELECT phone_number from contactphonenumbers where recid=:id and phone_type=1"),
    cardIdQuery("SELECT cardid FROM currentsimcard WHERE storage = :simtype"),
    firstIndexQuery("SELECT firstindex FROM currentsimcard WHERE storage = :simtype"),
    lastIndexQuery("SELECT lastindex FROM currentsimcard WHERE storage = :simtype"),
    labelLimitQuery("SELECT labellimit FROM currentsimcard WHERE storage = :simtype"),
    numberLimitQuery("SELECT numberLimit FROM currentsimcard WHERE storage = :simtype"),
    cardLoadedQuery("SELECT loaded FROM currentsimcard WHERE storage = :simtype")
{
    mPhoneBook = new QPhoneBook( QString(), this );
    simValueSpace = new QValueSpaceItem("/SIM/Contacts");
    connect(simValueSpace, SIGNAL(contentsChanged()),
            this, SLOT(checkSimLoaded()));

    mAccess = qobject_cast<ContactSqlIO *>(access);
    Q_ASSERT(mAccess);
}

/*!
  Destroys the QContactSimContext.
*/
QContactSimContext::~QContactSimContext()
{
}

/*!
  Returns an icon representing this context.
*/
QIcon QContactSimContext::icon() const
{
    static QIcon simicon(":icon/sim-contact");
    return simicon;
}

/*!
  Returns a translated string describing this context.
*/
QString QContactSimContext::description() const
{
    return tr("SIM Card Contact Access");
}

/*!
  Returns a translated string representing this context
*/
QString QContactSimContext::title() const
{
    return tr("SIM Card Contact Access");
}

/*!
  Returns true if it is currently possible to edit entries for the
  SIM storage.
  
  \sa defaultSource(), loadingSim()
*/
bool QContactSimContext::editable() const
{
    return simValueSpace->value("Loaded").toBool() && !card().isEmpty();
}

/*!
  Returns true if the entries for the SIM card have not completed
  syncing to the SQL storage.
*/
bool QContactSimContext::loadingSim() const
{
    return !simValueSpace->value("Loaded").toBool();
}

/*!
  Causes the context to check if the current SIM card entries have
  completed syncing to the SQL storage, if so emits simLoaded().
*/
void QContactSimContext::checkSimLoaded()
{
    if (!loadingSim())
        emit simLoaded(defaultSource());
}

/*!
  Returns the identifier for the currently inserted SIM card.
*/
QString QContactSimContext::card() const
{
    cardIdQuery.prepare();
    cardIdQuery.bindValue(":simtype", "SM");

    QString result;
    if (cardIdQuery.exec() && cardIdQuery.next())
        result = cardIdQuery.value(0).toString();
    cardIdQuery.reset();
    return result;
}

/*!
  Returns the number for the first valid index of the SIM card.
*/
int QContactSimContext::firstIndex() const
{
    firstIndexQuery.prepare();
    firstIndexQuery.bindValue(":simtype", "SM");

    int result;
    if (firstIndexQuery.exec() && firstIndexQuery.next())
        result = firstIndexQuery.value(0).toInt();
    else
        result = 0;
    firstIndexQuery.reset();
    return result;
}

/*!
  Returns the number for the last valid index of the SIM card.
  */
int QContactSimContext::lastIndex() const
{
    lastIndexQuery.prepare();
    lastIndexQuery.bindValue(":simtype", "SM");

    int result;
    if (lastIndexQuery.exec() && lastIndexQuery.next())
        result = lastIndexQuery.value(0).toInt();
    else
        result = 0;
    lastIndexQuery.reset();
    return result;
}

/*!
  Returns the maximum length for labels stored on the current SIM card.
  Does not take into account part of the label that will be used
  for contact phone number type indication.
*/
int QContactSimContext::labelLimit() const
{
    labelLimitQuery.prepare();
    labelLimitQuery.bindValue(":simtype", "SM");

    int result;
    if (labelLimitQuery.exec() && labelLimitQuery.next())
        result = labelLimitQuery.value(0).toInt();
    else
        result = 0;
    labelLimitQuery.reset();
    return result;
}

/*!
  Returns the maximum length for numbers stored on the current SIM card.
*/
int QContactSimContext::numberLimit() const
{
    numberLimitQuery.prepare();
    numberLimitQuery.bindValue(":simtype", "SM");

    int result;
    if (numberLimitQuery.exec() && numberLimitQuery.next())
        result = numberLimitQuery.value(0).toInt();
    else
        result = 0;
    numberLimitQuery.reset();
    return result;
}

/*!
  Returns true of the contact for the given \a identifier is editable.
*/
bool QContactSimContext::editable(const QUniqueId &identifier) const
{
    return editable() && source(identifier) == defaultSource();
}

/*!
  Returns the PIM source for user contacts stored on the SIM card.
*/
QPimSource QContactSimContext::defaultSource() const
{
    QPimSource s;
    s.context = id();
    s.identity = "sim"; // compat with old code
    return s;
}

/*!
  Returns the PIM source for service numbers stored on the SIM card.
*/
QPimSource QContactSimContext::serviceNumbersSource() const
{
    QPimSource s;
    s.context = id();
    s.identity = serviceNumbers;
    return s;
}

/*!
  Returns the set of PIM sources represented by this context.
*/
QSet<QPimSource> QContactSimContext::sources() const
{
    QSet<QPimSource> set;
    set.insert(defaultSource());
    set.insert(serviceNumbersSource());
    return set;
}

/*!
  Returns a universally unique identifier representing this context
*/
QUuid QContactSimContext::id() const
{
    static QUuid u("b63abe6f-36bd-4bb8-9c27-ece5436a5130");
    return u;
}

/*!
  Returns a translated string representing the given \a source.
*/
QString QContactSimContext::title(const QPimSource &source) const
{
    if (source == defaultSource())
        return tr("Active SIM Card");
    if (source == serviceNumbersSource())
        return tr("SIM Card Service Numbers");
    return QString();
}

/*!
  Filters the model that created this context to only show records for the sources
  that are contained in given \a set.  Does not affect data from other contexts.
*/
void QContactSimContext::setVisibleSources(const QSet<QPimSource> &set)
{
    QSet<int> show;
    QSet<int> hide;
    QSet<QPimSource> list = sources();
    foreach (QPimSource s, list) {
        int context = QPimSqlIO::sourceContext(s);
        if (set.contains(s)) {
            show.insert(context);
        } else {
            hide.insert(context);
        }
    }

    QSet<int> filter = mAccess->contextFilter();
    filter.unite(hide);
    filter.subtract(show);
    mAccess->setContextFilter(filter);
}

/*!
  Returns the list of sources of record data that is currently shown by the
  context.

  \sa setVisibleSources(), availableSources()
*/
QSet<QPimSource> QContactSimContext::visibleSources() const
{
    QSet<int> filter = mAccess->contextFilter();
    QSet<QPimSource> result;

    QSet<QPimSource> list = sources();
    foreach (QPimSource s, list) {
        int context = QPimSqlIO::sourceContext(s);
        if (!filter.contains(context))
            result.insert(s);
    }
    return result;
}

/*!
  Returns true if a contact with the given \a identifier is stored in a
  source represented by this context.

  \sa contains()
*/
bool QContactSimContext::exists(const QUniqueId &identifier) const
{
    return !source(identifier).isNull();
}

/*!
  Returns the storage source that contains the contact with the specified
  \a identifier.  Returns a null source if the record does not exist.

  \sa availableSources()
*/
QPimSource QContactSimContext::source(const QUniqueId &identifier) const
{
    int itemContext = mAccess->context(identifier);
    QSet<QPimSource> list = sources();
    foreach(QPimSource s, list) {
        int context = QPimSqlIO::sourceContext(s);
        if (context == itemContext)
            return s;
    }
    return QPimSource();
}

/*!
  Updates the contact in the model with the same identifier as the specified \a contact to
  equal the specified contact.

  Returns true if the contact was successfully updated.
*/
bool QContactSimContext::updateContact(const QContact &contact)
{
    if (!editable(contact.uid()))
        return false;

    QString label;
    QMap<QString, QString> phoneExtensions = mapPhoneNumbers(contact, label);

    QList<int> indexes = simCardIndexes("SM", contact.uid());
    if (freeIndexCount() < phoneExtensions.count() - indexes.count())
        return false;

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    syncTime = syncTime.addMSecs(-syncTime.time().msec());

    if (mAccess->startTransaction(syncTime)) {
        if (mAccess->updateContact(contact)) {
            QString existingLabel = labelForId("SM", contact.uid());
            if (existingLabel != label)
                setLabelForId("SM", contact.uid(), label);
            removeEntries(indexes);
            addEntries(contact.uid(), phoneExtensions, label);
            if (mAccess->commitTransaction())
                return true;
        }
        mAccess->abortTransaction();
    }
    return false;
}

/*!
  Removes the contact from the model with the specified \a identifier.

  Returns true if the contact was successfully removed.
*/
bool QContactSimContext::removeContact(const QUniqueId &identifier)
{
    if (!editable(identifier))
        return false;
    if (mAccess->removeContact(identifier)) {
        clearLabelsForId("SM", identifier);
        QList<int> list = simCardIndexes("SM", identifier);
        removeEntries(list);
        return true;
    }
    return false;
}

/*!
  Removes the given \a list of indexes from the SIM phone book.
*/
void QContactSimContext::removeEntries(const QList<int> &list)
{
    foreach(int index, list)
        mPhoneBook->remove(index);
    mPhoneBook->flush();
}

/*!
  Adds the given contact to the database if the given pim \a source is the source for the "SM" phone book.  Will also modify the SIM to contain
  phone numbers for this contact.  If the contact contains too many phone numbers to fit on the SIM will return a null id and not add the contact.
*/
QUniqueId QContactSimContext::addContact(const QContact &contact, const QPimSource &source)
{
    if (source != defaultSource() || !editable())
        return QUniqueId();

    QString label;
    QMap<QString, QString> phoneExtensions = mapPhoneNumbers(contact, label);

    if (freeIndexCount() < phoneExtensions.count() || phoneExtensions.values().count() == 0)
        return QUniqueId();

    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());

    syncTime = syncTime.addMSecs(-syncTime.time().msec());

    if (mAccess->startTransaction(syncTime)) {
        static const QUuid appScope("b63abe6f-36bd-4bb8-9c27-ece5436a5130");
        QUniqueIdGenerator g(appScope);
        QContact c = contact;
        c.setUid(g.createUniqueId());
        if (!mAccess->addContact(c, source, false).isNull()) {
            setIdForLabel("SM", label, c.uid());
            addEntries(c.uid(), phoneExtensions, label);
            if (mAccess->commitTransaction())
                return c.uid();
        }
        mAccess->abortTransaction();
    }
    return QUniqueId();
}

/*!
  Adds the entries for the given \a numbers and \a label to the SIM phone book entries.
  Also sets the SIM card id mapping for the resulting indexes used to store the number entries.  Each label
  for a phone book entry will be appended with text representing the type of phone number stored for that entry.

  \sa mapPhoneNumbers()
*/
void QContactSimContext::addEntries(const QUniqueId &id, const QMap<QString,QString> &numbers, const QString & label) 
{
    QMapIterator<QString,QString> it(numbers);
    while (it.hasNext()) {
        it.next();
        QString ext = it.key();
        QString number = it.value();

        int index = nextFreeIndex();
        setSimCardId("SM", index, id);

        QPhoneBookEntry entry;
        entry.setIndex( index );
        entry.setText( label + ext );
        entry.setNumber( number );
        mPhoneBook->add(entry);
    }
}

/*!
  Returns a mapping of label to phone number for each of the phone numbers in the given \a contact.
  The labels in the mapping are made up from the given \a label conjoined to a string representing
  the phone number type.

  For instance, the Business Mobile number for the label "Bob" would be "Bob/bm".
*/
QMap<QString, QString> QContactSimContext::mapPhoneNumbers(const QContact &contact, QString &label)
{
    label = QContactSimContext::simLabel(contact);
    if (label.isEmpty())
        label = contact.label();

    QStringList pfields = QContactFieldDefinition::fields("phone");
    QMap<QString, QString> result;
    int extLen = 0;
    foreach(QString pfield, pfields) {
        QContactFieldDefinition def(pfield);
        QString number = def.value(contact).toString();
        if (!number.isEmpty()) {
            QString ext = fieldExtension(pfield);
            result.insert(ext, number);
            if (number.length() > numberLimit())
                return QMap<QString, QString>();
            extLen = qMax(extLen, ext.length());
        }
    }
    label = label.left(labelLimit() - extLen);
    return result;
}

QPreparedSqlQuery QContactSimContext::selectIdQuery("SELECT sqlid FROM simcardidmap WHERE cardid = :c AND cardindex = :i AND storage = :s");
QPreparedSqlQuery QContactSimContext::insertIdQuery("INSERT INTO simcardidmap (sqlid, cardid, cardindex, storage) VALUES (:s, :c, :i, :st)");
QPreparedSqlQuery QContactSimContext::updateIdQuery("UPDATE simcardidmap SET sqlid = :s WHERE cardid = :c AND cardindex = :i AND storage = :st");

/*!
  Returns the identity of the contact that is represented at the given
  \a index for the given SIM \a card and \a storage.  If no mapping
  is stored, returns a null identity.
*/
QUniqueId QContactSimContext::simCardId(const QString & card, const QString &storage, int index)
{
    if (!selectIdQuery.prepare())
        return QUniqueId();

    selectIdQuery.bindValue(":c", card);
    selectIdQuery.bindValue(":i", index);
    selectIdQuery.bindValue(":s", storage);
    selectIdQuery.exec();

    QUniqueId result;
    if (selectIdQuery.next())
        result = QUniqueId::fromUInt(selectIdQuery.value(0).toUInt());
    selectIdQuery.reset();
    return result;
}

/*!
  Sets the given \a index for the given SIM \a card and \a storage to
  the given \a identity.

  Note, only sets the mapping.  Does not otherwise update the SIM card
  or SQL storage.
*/
void QContactSimContext::setSimCardId(const QString &card, const QString &storage, int index, const QUniqueId &identity)
{
    if (simCardId(card, storage, index).isNull())
    {
        insertIdQuery.prepare();
        insertIdQuery.bindValue(":s", identity.toUInt());
        insertIdQuery.bindValue(":c", card);
        insertIdQuery.bindValue(":i", index);
        insertIdQuery.bindValue(":st", storage);
        insertIdQuery.exec();
        insertIdQuery.reset();
    } else {
        updateIdQuery.prepare();
        updateIdQuery.bindValue(":s", identity.toUInt());
        updateIdQuery.bindValue(":c", card);
        updateIdQuery.bindValue(":i", index);
        updateIdQuery.bindValue(":st", storage);
        updateIdQuery.exec();
        updateIdQuery.reset();
    }
}

/*!
  Returns the list of indexes in the SIM phone book storage on the given
  \a card and \a storage that store phone numbers that store phone numbers
  for the contact with the given \a id.
*/
QList<int> QContactSimContext::simCardIndexes(const QString &card, const QString &storage, const QUniqueId &id)
{
    QPreparedSqlQuery q(QPimSqlIO::database());

    q.prepare("SELECT cardindex FROM simcardidmap WHERE cardid = :c AND storage = :st AND sqlid = :s");
    q.bindValue(":c", card);
    q.bindValue(":st", storage);
    q.bindValue(":s", id.toUInt());

    q.exec();
    QList<int> result;
    while(q.next()) {
        result.append(q.value(0).toUInt());
    }
    return result;
}

/*!
    Returns the PIM database identifier that maps to the given contact \a label for the given SIM
    \a storage type on the given \a card.
*/
QUniqueId QContactSimContext::idForLabel(const QString &card, const QString &storage, const QString &label)
{
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT sqlid FROM simlabelidmap WHERE label = :l AND cardid = :c AND storage = :s");
    q.bindValue(":l", label);
    q.bindValue(":c", card);
    q.bindValue(":s", storage);
    if (q.exec() && q.next())
        return QUniqueId::fromUInt(q.value(0).toUInt());
    return QUniqueId();
}

/*!
    Returns the SIM contact label the given SIM
    \a storage type on the given \a card that maps to the given \a identifier.
*/
QString QContactSimContext::labelForId(const QString &card, const QString &storage, const QUniqueId &identifier)
{
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT label FROM simlabelidmap WHERE sqlid = :r AND cardid = :c AND storage = :s");
    q.bindValue(":c", card);
    q.bindValue(":s", storage);
    q.bindValue(":r", identifier.toUInt());
    if (q.exec() && q.next())
        return q.value(0).toString();
    return QString();
}

/*!
    Sets the SIM contact \a label for the given \a storage type and \a card to map to the given \a identifier.
*/
void QContactSimContext::setIdForLabel(const QString &card, const QString &storage, const QString &label, const QUniqueId &identifier)
{
    QSqlQuery q(QPimSqlIO::database());
    if (labelForId(card, storage, identifier).isEmpty()) {
        q.prepare("INSERT INTO simlabelidmap(sqlid, label, cardid, storage) VALUES (:r, :l, :c, :s)");
    } else {
        q.prepare("UPDATE simlabelidmap SET sqlid = :r WHERE cardid = :c and storage = :s AND label = :l");
    }
    q.bindValue(":l", label);
    q.bindValue(":c", card);
    q.bindValue(":s", storage);
    q.bindValue(":r", identifier.toUInt());
    q.exec();
}

/*!
    Sets the given \a identifier to map to the given SIM contact \a label, \a storage type and \a card.
*/
void QContactSimContext::setLabelForId(const QString &card, const QString &storage, const QUniqueId &identifier, const QString &label)
{
    QSqlQuery q(QPimSqlIO::database());

    q.prepare("UPDATE simlabelidmap SET label = :l WHERE cardid = :c and storage = :s AND sqlid = :r");
    q.bindValue(":l", label);
    q.bindValue(":c", card);
    q.bindValue(":s", storage);
    q.bindValue(":r", identifier.toUInt());
    if (!q.exec()) {
        q.prepare("INSERT INTO simlabelidmap(sqlid, label, cardid, storage)");
        q.bindValue(":l", label);
        q.bindValue(":c", card);
        q.bindValue(":s", storage);
        q.bindValue(":r", identifier.toUInt());

        q.exec();
    }
}

/*!
    Removes all label mappings for the given \a identifier for the given SIM \a card and \a storage type.
*/
void QContactSimContext::clearLabelsForId(const QString &card, const QString &storage, const QUniqueId &identifier)
{
    QSqlQuery q(QPimSqlIO::database());

    q.prepare("DELETE FROM simlabelidmap WHERE sqlid = :r AND storage = :s AND cardid = :c");

    q.bindValue(":c", card);
    q.bindValue(":s", storage);
    q.bindValue(":r", identifier.toUInt());

    q.exec();
}

/*!
  Returns the next free index in the SIM phone book storage.
*/
int QContactSimContext::nextFreeIndex() const
{
    QPreparedSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT cardindex FROM simcardidmap WHERE cardid = :c AND storage = :s ORDER BY cardindex");
    q.bindValue(":c", card());
    q.bindValue(":s", "SM");
    q.exec();

    int index = firstIndex();
    while (q.next()) {
        int pos = q.value(0).toInt();
        if (pos != index)
            break;
        index ++;
    }
    if (index > lastIndex())
        return -1;
    return index;
}

/*!
  Returns the number of indexes already used in the SIM phone book storage.
*/
int QContactSimContext::usedIndexCount() const
{
    QPreparedSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT count(*) FROM simcardidmap WHERE cardid = :c AND storage = :s");
    q.bindValue(":c", card());
    q.bindValue(":s", "SM");
    q.exec();
    if (q.next())
        return q.value(0).toInt();
    return 0;
}

/*!
  Imports the contacts in the given \a list to the given \a source.
  Merges contacts into the existing list where possible, rather than
  creating duplicates.

  Returns true if successful, false if failed.
*/
bool QContactSimContext::importContacts(const QPimSource &source, const QList<QContact> &list)
{
    if (!editable() || source != defaultSource())
        return false;


    QDateTime syncTime = QTimeZone::current().toUtc(QDateTime::currentDateTime());
    syncTime = syncTime.addMSecs(-syncTime.time().msec());

    if (mAccess->startTransaction(syncTime)) {
        foreach (QContact c, list) {
            // TODO
            // if (exist)
            //  merge to existing
            //  update existing
            // else
            //  add
            addContact(c, source);
        }
    }
    return false;
}

/*!
  Returns the contact for the given \a id.  Sets 
  \a ok to true if successful, false if failed.
  */
QContact QContactSimContext::exportContact(const QUniqueId &id, bool &ok) const
{
    if (!exists(id)) {
        ok = false;
        return QContact();
    }
    ok = true;
    return mAccess->contact(id);
}

/*!
  Returns the list of contacts found in the given \a source.  Sets 
  \a ok to true if successful, false if failed.
*/
QList<QContact> QContactSimContext::exportContacts(const QPimSource &source, bool &ok) const
{
    ok = true;
    QSqlQuery q(QPimSqlIO::database());
    q.prepare("SELECT DISTINCT sqlid, firstname FROM simcardidmap JOIN contacts ON sqlid = recid WHERE storage = :s");

    QPreparedSqlQuery phoneQuery(QPimSqlIO::database());
    phoneQuery.prepare("SELECT phone_type, phone_number from contactphonenumbers where recid=:id");

    if (source == defaultSource())
        q.bindValue(":s", "SM");
    else
        q.bindValue(":s", source.identity);
    q.exec();
    
    QList<QContact> result;
    while (q.next()) {
        QContact c = QContact::parseLabel(q.value(1).toString());
        c.setUid(QUniqueId::fromUInt(q.value(0).toUInt()));
        phoneQuery.bindValue(":id", q.value(0));
        phoneQuery.exec();
        while(phoneQuery.next()) {
            QString number;
            QContact::PhoneType type;
            type = (QContact::PhoneType)phoneQuery.value(0).toInt();
            number = phoneQuery.value(1).toString();
            c.setPhoneNumber(type, number);
        }
        result.append(c);
        phoneQuery.reset();
    }
    return result;
}
