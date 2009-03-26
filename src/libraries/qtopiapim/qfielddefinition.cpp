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

#include "qfielddefinition.h"
#include <QtopiaIpcEnvelope>
#include <QtopiaService>
#include <QTranslatableSettings>
#include <QContactModel>
#include <QAppointmentModel>
#include <QTaskModel>
#include <QVariant>
#include <QStringList>
#include <QDebug>

#define ql1 QLatin1String

struct QFieldAction
{
    QString mIconString;
    QString mLabel;
    QString mService;
    QString mRequest;
    QStringList mArguments;
};

static const ql1 contactWebService("ContactIntegration");

typedef QMap<QString, QSet<QString> > TagMap;
typedef QMap<QString, QFieldDefinition> FieldMap;
typedef QMap<QString, QFieldAction> ActionMap;

TagMap& appointmentTagMap() { static TagMap m; return m; }
FieldMap& appointmentFieldMap() { static FieldMap m; return m; }
ActionMap& appointmentActionMap() { static ActionMap m; return m; }

static bool appointmentInfoLoaded;

TagMap& contactTagMap() { static TagMap m; return m; }
FieldMap& contactFieldMap() { static FieldMap m; return m; }
ActionMap& contactActionMap() { static ActionMap m; return m; }

static bool contactInfoLoaded;

TagMap& taskTagMap() { static TagMap m; return m; }
FieldMap& taskFieldMap() { static FieldMap m; return m; }
ActionMap& taskActionMap() { static ActionMap m; return m; }

static bool taskInfoLoaded;

class QFieldDefinitionData : public QSharedData
{
public:
    QString mIdentity;
    QString mLabel;
    QIcon mIcon;
    QString mIconString;
    QString mInputHint;
    QStringList mEditActions;
    QStringList mBrowseActions;
    QStringList mTags;
    QMap<QString, QVariant> mAttributes;

    static QMap<QString, QFieldAction> readActions(QTranslatableSettings &);
    static QList<QFieldDefinition> readFields(QTranslatableSettings &);
    static QFieldAction fieldAction(QTranslatableSettings &);
    static QFieldDefinition fieldDefinition(QTranslatableSettings &);

    static void init(TagMap &tagMap, FieldMap &fieldMap, ActionMap &actionMap, QTranslatableSettings &config);
};

void QFieldDefinitionData::init(TagMap &tagMap, FieldMap &fieldMap, ActionMap &actionMap, QTranslatableSettings &config)
{
    QList<QFieldDefinition> fields;
    QMap<QString, QFieldAction> actions;

    fields = QFieldDefinitionData::readFields(config);
    actions = QFieldDefinitionData::readActions(config);

    foreach(QFieldDefinition def, fields) {
        if (!fieldMap.contains(def.id())) {
            fieldMap.insert(def.id(), def);
            QStringList tags = def.tags();
            foreach(QString tag, tags)
            {
                QSet<QString> taggedFields;
                if (tagMap.contains(tag))
                    taggedFields = tagMap.value(tag);
                taggedFields.insert(def.id());
                tagMap.insert(tag, taggedFields);
            }
        }
    }

    QMapIterator<QString, QFieldAction> it(actions);
    while(it.hasNext()){
        it.next();
        if (!actionMap.contains(it.key()))
            actionMap.insert(it.key(), it.value());
    }
}

/*!
    Construct an empty field definition
*/
QFieldDefinition::QFieldDefinition()
{
    d = new QFieldDefinitionData;
}

/*!
    Constructs a new field definition as a copy of \a other.
*/
QFieldDefinition::QFieldDefinition(const QFieldDefinition &other)
{
    d = other.d;
}

/*!
    Destroys the field definition.
*/
QFieldDefinition::~QFieldDefinition()
{
}


/*!
    Returns the string identifier used to construct this field definition.
*/
QString QFieldDefinition::id() const
{
    return d->mIdentity;
}

/*!
    Returns a label appropriate for use in the user interface describing
    this field.
*/
QString QFieldDefinition::label() const
{
    return d->mLabel;
}

/*!
    Returns a icon appropriate for use in the user interface representing
    this field.
*/
QIcon QFieldDefinition::icon() const
{
    return d->mIcon;
}

/*!
    Returns a icon resource string appropriate for use in the user interface representing
    this field.
*/
QString QFieldDefinition::iconString() const
{
    return d->mIconString;
}

/*!
    Returns the input hint that should be used when editing values for this field.
*/
QString QFieldDefinition::inputHint() const
{
    return d->mInputHint;
}

/*!
    Returns the list of additional attribute keys for this field.
*/
QStringList QFieldDefinition::attributeKeys() const
{
    return d->mAttributes.keys();
}

/*!
    Returns the value of the given attribute \a key for this field.
*/
QVariant QFieldDefinition::attribute(const QString &key) const
{
    if (d->mAttributes.contains(key))
        return d->mAttributes.value(key);
    return QVariant();
}

/*!
    Returns a list of encapsulated edit actions appropriate for this field
*/
QStringList QFieldDefinition::editActions() const
{
    return d->mEditActions;
}

/*!
    Returns a list of encapsulated browse actions appropriate for this field
*/
QStringList QFieldDefinition::browseActions() const
{
    return d->mBrowseActions;
}

/*!
    Returns true if this field has the given \a tag.  For
    instance a field may be tagged with 'phone', 'home' and 'mobile'
    indicating it relates to the users home details, is a mobile number
    and is part of the set of phone numbers.
*/
bool QFieldDefinition::hasTag(const QString &tag) const
{
    return d->mTags.contains(tag);
}

/*!
    Returns the set of tags for this field.
*/
QStringList QFieldDefinition::tags() const
{
    return d->mTags;
}

/*
    Returns the field definition in the currently set group or array index
    of the given \a config.
 */
QFieldDefinition QFieldDefinitionData::fieldDefinition(QTranslatableSettings &config)
{
    QLatin1String keyIdentity("Identity");
    QLatin1String keyIcon("Icon");
    QLatin1String keyLabel("Label");
    QLatin1String keyInputHint("InputHint");
    QLatin1String keyTags("Tags");

    QFieldDefinition field;

    QStringList keys = config.childKeys();
    foreach(QString key, keys) {
        if (key == keyIdentity) {
            field.d->mIdentity = config.value(keyIdentity).toString();
        } else if (key == keyIcon) {
            field.d->mIconString = config.value(keyIcon).toString();
            field.d->mIcon = QIcon(field.d->mIconString);
        } else if (key == keyLabel) {
            field.d->mLabel = config.value(keyLabel).toString();
        } else if (key == keyInputHint) {
            field.d->mInputHint = config.value(keyInputHint).toString();
        } else if (key == keyTags) {
            field.d->mTags = config.value(keyTags).toStringList();
        } else {
            field.d->mAttributes.insert(key, config.value(key));
        }
    }
    return field;
}

/*
    Returns the field action in the currently set group or array index
    of the given \a config.
*/
QFieldAction QFieldDefinitionData::fieldAction(QTranslatableSettings &config)
{
    QFieldAction action;
    action.mIconString = config.value(ql1("Icon")).toString();
    action.mLabel = config.value(ql1("Label")).toString();

    action.mService = config.value(ql1("Service")).toString();
    action.mRequest = config.value(ql1("Request")).toString();
    action.mArguments = config.value(ql1("RequestArgs")).toStringList();
    return action;
}

/*
    Returns the set of field definitions found in the given \a config.
*/
QMap<QString, QFieldAction> QFieldDefinitionData::readActions(QTranslatableSettings &config)
{
    int i, count;
    QMap<QString, QFieldAction> actionMap;
    count = config.beginReadArray(ql1("FieldActions"));
    for (i = 0; i < count; ++i) {
        config.setArrayIndex(i);
        QString id = config.value(ql1("Identity")).toString();
        actionMap.insert(id, QFieldDefinitionData::fieldAction(config));
    }
    config.endArray();
    return actionMap;
}

QList<QFieldDefinition> QFieldDefinitionData::readFields(QTranslatableSettings &config)
{
    int i, count;
    QList<QFieldDefinition> result;
    count = config.beginReadArray(ql1("FieldDefinitions"));
    for (i = 0; i < count; ++i) {
        config.setArrayIndex(i);
        QFieldDefinition f = QFieldDefinitionData::fieldDefinition(config);
        if (config.value("Selected").toBool())
            f.d->mTags.append("selected");

        f.d->mBrowseActions = config.value("BrowseActions").toStringList();
        f.d->mEditActions = config.value("EditActions").toStringList();

        result.append(f);
    }
    config.endArray();

    return result;
}

/*!
    Makes a shallow copy of \a definition and assigns it to this QFieldDefinition.
*/
QFieldDefinition &QFieldDefinition::operator=(const QFieldDefinition &definition)
{
    d = definition.d;
    return *this;
}

/*!
    Construct a new field definition with the properties of the given
    field \a id.
    \sa serviceFields(), phoneFields()
*/
QContactFieldDefinition::QContactFieldDefinition(const QString &id)
    : QFieldDefinition(field(id))
{
}

/*!
    Constructs a new empty field definition.
*/
QContactFieldDefinition::QContactFieldDefinition()
    : QFieldDefinition()
{
}

/*!
    Destroys the field definition.
*/
QContactFieldDefinition::~QContactFieldDefinition()
{
}

/*!
    Returns the value of for the field this field definition represents
    from the given \a contact.
*/
QVariant QContactFieldDefinition::value(const QContact &contact) const
{
    QContactModel::Field f = QContactModel::identifierField(id());
    /* QContactSqlIO::setPresenceInformation may need to be changed if this code changes */
    if (f != QContactModel::Invalid)
        return QContactModel::contactField(contact, f);
    else
        return contact.customField(id());
}

/*!
    Returns the provider for this field.  Only some contact fields
    have providers defined.  For example, a presence related field may
    have the name of a QCollectivePresence provider.  If there is
    no provider defined for the field returns a null string.
*/
QString QContactFieldDefinition::provider() const
{
    return attribute("Provider").toString();
}

/*!
    Sets the value for the field this field definition represents in
    the given \a contact to the given \a value.
*/
void QContactFieldDefinition::setValue(QContact &contact, const QVariant &value) const
{
    QContactModel::Field f = QContactModel::identifierField(id());
    if (f != QContactModel::Invalid)
        QContactModel::setContactField(contact, f, value);
    else
        contact.setCustomField(id(), value.toString());
}

/*
    Returns the set of fields with tags matching the given \a searchString.
    The search strings should be made of a space separated list of required
    tags.  A tag that is to be excluded can be preceded with a '-' character.
*/
QStringList q_fields(const QString &searchString, const QStringList &candidates, const QMap<QString, QSet<QString> > &tagMap)
{
    QStringList searchTerms = searchString.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    QSet<QString> result = QSet<QString>::fromList(candidates);
    foreach(QString term, searchTerms) {
        if (term[0] == '-') {
            if (term.length() > 1)
                result -= tagMap.value(term.mid(1));
        } else {
            result &= tagMap.value(term);
        }
    }
    return result.toList();
}

/*!
    Returns the set of fields with tags matching the given \a searchString.
    The search strings should be made of a space separated list of required
    tags.  A tag that is to be excluded can be preceded with a '-' character.

    \code
    QString nonFaxPhoneFields = QContactFieldDefinition::fields("phone -fax");
    foreach(QString field, nonFaxPhoneFields) {
        QContactFieldDefinition definition(field);
        assert(!definition.hasTag("fax"));
        assert(definition.hasTag("phone"));
    }
    \endcode
*/
QStringList QContactFieldDefinition::fields(const QString &searchString)
{
    init();
    return q_fields(searchString, contactFieldMap().keys(), contactTagMap());
}

void QContactFieldDefinition::init()
{
    if (!contactInfoLoaded)
    {
        {
            QTranslatableSettings config( "Trolltech", "Contacts" );
            config.beginGroup("PhoneFields");
            QFieldDefinitionData::init(contactTagMap(), contactFieldMap(), contactActionMap(), config);
        }

        QStringList apps = QtopiaService::apps( contactWebService );  // No tr
        foreach(QString app, apps) {
            QString settingsFile = QtopiaService::appConfig(contactWebService, app);
            QTranslatableSettings config(settingsFile, QSettings::IniFormat);

            config.beginGroup(ql1("Features"));
            QFieldDefinitionData::init(contactTagMap(), contactFieldMap(), contactActionMap(), config);
        }
        contactInfoLoaded = true;
    }
}

QFieldDefinition QContactFieldDefinition::field(const QString &id)
{
    init();
    if (contactFieldMap().contains(id))
        return contactFieldMap()[id];
    return QFieldDefinition();
}

/*!
    Returns a service description for the given \a action identifier.
    Note, the service request portion of the service description will not
    be set at this point.  This part can be completed using the actionRequest()
    function.
*/
QtopiaServiceDescription QContactFieldDefinition::actionDescription(const QString &action)
{
    QtopiaServiceDescription result;
    if (contactActionMap().contains(action))
    {
        QFieldAction fieldAction = contactActionMap()[action];
        result.setIconName(fieldAction.mIconString);
        result.setLabel(fieldAction.mLabel);
    }
    return result;
}

/*!
    Returns the translated label for the given \a action.
*/
QString QContactFieldDefinition::actionLabel(const QString &action)
{
    if (contactActionMap().contains(action))
        return contactActionMap()[action].mLabel;
    return QString();
}

/*!
    Returns the icon resource string for the given \a action.
*/
QString QContactFieldDefinition::actionIconString(const QString &action)
{
    if (contactActionMap().contains(action))
        return contactActionMap()[action].mIconString;
    return QString();
}

/*!
    \fn QIcon QContactFieldDefinition::actionIcon(const QString &action)

    Returns the icon for the given \a action.
*/

/*!
    Returns the service request for the given \a action, with arguments populated
    from the given \a contact and field \a value.
*/
QtopiaServiceRequest QContactFieldDefinition::actionRequest(const QString &action, const QContact &contact, const QString &value)
{
    if (!contactActionMap().contains(action))
        return QtopiaServiceRequest();
    QFieldAction fieldAction = contactActionMap()[action];

    QtopiaServiceRequest result(fieldAction.mService, fieldAction.mRequest);
    foreach(QString arg, fieldAction.mArguments)
    {
        if (arg == "id")
            result << contact.uid();
        else if (arg == "label")
            result << contact.label();
        else if (arg == "field")
            result << value;
        else if (arg == "contact")
            result << contact;
    }
    return result;
}

/*!
    Construct a new field definition with the properties of the given
    field \a id.
    \sa serviceFields(), phoneFields()
*/
QTaskFieldDefinition::QTaskFieldDefinition(const QString &id)
    : QFieldDefinition(field(id))
{
}

/*!
    Constructs a new empty field definition.
*/
QTaskFieldDefinition::QTaskFieldDefinition()
    : QFieldDefinition()
{
}

/*!
    Destroys the field definition.
*/
QTaskFieldDefinition::~QTaskFieldDefinition()
{
}

/*!
    Returns the value of for the field this field definition represents
    from the given \a task.
*/
QVariant QTaskFieldDefinition::value(const QTask &task) const
{
    QTaskModel::Field f = QTaskModel::identifierField(id());
    if (f != QTaskModel::Invalid)
        return QTaskModel::taskField(task, f);
    else
        return task.customField(id());
}

/*!
    Sets the value for the field this field definition represents in
    the given \a task to the given \a value.
*/
void QTaskFieldDefinition::setValue(QTask &task, const QVariant &value) const
{
    QTaskModel::Field f = QTaskModel::identifierField(id());
    if (f != QTaskModel::Invalid)
        QTaskModel::setTaskField(task, f, value);
    else
        task.setCustomField(id(), value.toString());
}

/*!
    Returns the set of fields with tags matching the given \a searchString.
    The search strings should be made of a space separated list of required
    tags.  A tag that is to be excluded can be preceded with a '-' character.
*/
QStringList QTaskFieldDefinition::fields(const QString &searchString)
{
    init();
    return q_fields(searchString, taskFieldMap().keys(), taskTagMap());
}

void QTaskFieldDefinition::init()
{
    if (!taskInfoLoaded)
    {
        QTranslatableSettings config( "Trolltech", "Tasks" );
        config.beginGroup("Fields");
        QFieldDefinitionData::init(taskTagMap(), taskFieldMap(), taskActionMap(), config);
        taskInfoLoaded = true;
    }
}

QFieldDefinition QTaskFieldDefinition::field(const QString &id)
{
    if (taskFieldMap().contains(id))
        return taskFieldMap()[id];
    return QFieldDefinition();
}

/*!
    Returns a service description for the given \a action identifier.
    Note, the service request portion of the service description will not
    be set at this point.  This part can be completed using the actionRequest()
    function.
*/
QtopiaServiceDescription QTaskFieldDefinition::actionDescription(const QString &action)
{
    QtopiaServiceDescription result;
    if (taskActionMap().contains(action))
    {
        QFieldAction fieldAction = taskActionMap()[action];
        result.setIconName(fieldAction.mIconString);
        result.setLabel(fieldAction.mLabel);
    }
    return result;
}

/*!
    Returns the translated label for the given \a action.
*/
QString QTaskFieldDefinition::actionLabel(const QString &action)
{
    if (taskActionMap().contains(action))
        return taskActionMap()[action].mLabel;
    return QString();
}

/*!
    Returns the icon resource string for the given \a action.
*/
QString QTaskFieldDefinition::actionIconString(const QString &action)
{
    if (taskActionMap().contains(action))
        return taskActionMap()[action].mIconString;
    return QString();
}

/*!
    \fn QIcon QTaskFieldDefinition::actionIcon(const QString &action)

    Returns the icon for the given \a action.
*/

/*!
    Returns the service request for the given \a action, with arguments populated
    from the given \a task and field \a value.
*/
QtopiaServiceRequest QTaskFieldDefinition::actionRequest(const QString &action, const QTask &task, const QString &value)
{
    if (!taskActionMap().contains(action))
        return QtopiaServiceRequest();
    QFieldAction fieldAction = taskActionMap()[action];

    QtopiaServiceRequest result(fieldAction.mService, fieldAction.mRequest);
    foreach(QString arg, fieldAction.mArguments)
    {
        if (arg == "id")
            result << task.uid();
        else if (arg == "field")
            result << value;
        else if (arg == "task")
            result << task;
    }
    return result;
}

/*!
    Construct a new field definition with the properties of the given
    field \a id.
    \sa serviceFields(), phoneFields()
*/
QAppointmentFieldDefinition::QAppointmentFieldDefinition(const QString &id)
    : QFieldDefinition(field(id))
{
}

/*!
    Constructs a new empty field definition.
*/
QAppointmentFieldDefinition::QAppointmentFieldDefinition()
    : QFieldDefinition()
{
}

/*!
    Destroys the field definition.
*/
QAppointmentFieldDefinition::~QAppointmentFieldDefinition()
{
}

/*!
    Returns the value of for the field this field definition represents
    from the given \a appointment.
*/
QVariant QAppointmentFieldDefinition::value(const QAppointment &appointment) const
{
    QAppointmentModel::Field f = QAppointmentModel::identifierField(id());
    if (f != QAppointmentModel::Invalid)
        return QAppointmentModel::appointmentField(appointment, f);
    else
        return appointment.customField(id());
}

/*!
    Sets the value for the field this field definition represents in
    the given \a appointment to the given \a value.
*/
void QAppointmentFieldDefinition::setValue(QAppointment &appointment, const QVariant &value) const
{
    QAppointmentModel::Field f = QAppointmentModel::identifierField(id());
    if (f != QAppointmentModel::Invalid)
        QAppointmentModel::setAppointmentField(appointment, f, value);
    else
        appointment.setCustomField(id(), value.toString());
}

/*!
    Returns the set of fields with tags matching the given \a searchString.
    The search strings should be made of a space separated list of required
    tags.  A tag that is to be excluded can be preceded with a '-' character.
*/
QStringList QAppointmentFieldDefinition::fields(const QString &searchString)
{
    init();
    return q_fields(searchString, appointmentFieldMap().keys(), appointmentTagMap());
}

void QAppointmentFieldDefinition::init()
{
    if (!appointmentInfoLoaded)
    {
        QTranslatableSettings config( "Trolltech", "Calendar" );
        config.beginGroup("Fields");
        QFieldDefinitionData::init(appointmentTagMap(), appointmentFieldMap(), appointmentActionMap(), config);
        appointmentInfoLoaded = true;
    }
}

QFieldDefinition QAppointmentFieldDefinition::field(const QString &id)
{
    if (appointmentFieldMap().contains(id))
        return appointmentFieldMap()[id];
    return QFieldDefinition();
}

/*!
    Returns a service description for the given \a action identifier.
    Note, the service request portion of the service description will not
    be set at this point.  This part can be completed using the actionRequest()
    function.
*/
QtopiaServiceDescription QAppointmentFieldDefinition::actionDescription(const QString &action)
{
    QtopiaServiceDescription result;
    if (appointmentActionMap().contains(action))
    {
        QFieldAction fieldAction = appointmentActionMap()[action];
        result.setIconName(fieldAction.mIconString);
        result.setLabel(fieldAction.mLabel);
    }
    return result;
}

/*!
    Returns the translated label for the given \a action.
*/
QString QAppointmentFieldDefinition::actionLabel(const QString &action)
{
    if (appointmentActionMap().contains(action))
        return appointmentActionMap()[action].mLabel;
    return QString();
}

/*!
    Returns the icon resource string for the given \a action.
*/
QString QAppointmentFieldDefinition::actionIconString(const QString &action)
{
    if (appointmentActionMap().contains(action))
        return appointmentActionMap()[action].mIconString;
    return QString();
}

/*!
    \fn QIcon QAppointmentFieldDefinition::actionIcon(const QString &action)

    Returns the icon for the given \a action.
*/
/*!
    Returns the service request for the given \a action, with arguments populated
    from the given \a appointment and field \a value.
*/
QtopiaServiceRequest QAppointmentFieldDefinition::actionRequest(const QString &action, const QAppointment &appointment, const QString &value)
{
    if (!appointmentActionMap().contains(action))
        return QtopiaServiceRequest();
    QFieldAction fieldAction = appointmentActionMap()[action];

    QtopiaServiceRequest result(fieldAction.mService, fieldAction.mRequest);
    foreach(QString arg, fieldAction.mArguments)
    {
        if (arg == "id")
            result << appointment.uid();
        else if (arg == "field")
            result << value;
        else if (arg == "appointment")
            result << appointment;
    }
    return result;
}
