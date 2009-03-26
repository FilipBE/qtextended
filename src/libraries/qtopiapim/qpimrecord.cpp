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

#include <qpimrecord.h>
#include "vobject_p.h"
#include <qcategorymanager.h>
#include <qdatastream.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <qtopiaapplication.h>
#include <qsettings.h>
#include <qobject.h>
#include <stdlib.h>

#include "qpimdependencylist_p.h"

/*!
  \class QPimRecord
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimRecord class is the base class for PIM data recorded in the
  Qt Extended database.

  The QPimRecord class contains data that is common to the differing kinds
  of PIM records such as contacts, tasks and appointments.  It is an also
  abstract and as such should not be created explicitly.  Instead use one
  of the subclasses, QContact, QAppointment or QTask.

  \sa {Pim Library}
*/

/*!
  \fn QMap<QString, QString> QPimRecord::customFields() const

  Returns a map of custom field key and value pairs for the record.

  \sa setCustomFields(), customField(), customFieldsRef()
*/


/*!
  \fn QString QPimRecord::notes() const
  Returns the notes for the record.

  \sa setNotes()
*/

/*!
  \fn QString QPimRecord::setNotes(const QString &text)
  Sets the notes for the record to the given \a text.

  \sa notes()
*/

/*!
  \fn QUniqueId &QPimRecord::uidRef()

  Subclasses should re-implement this function to return a reference to the identifier for this object.

  \sa uid()
*/

/*!
  \fn const QUniqueId &QPimRecord::uidRef() const
  Subclasses should re-implement this function to return a reference to the identifier for this object.

  \sa uid()
*/

/*!
  \fn QList<QString> &QPimRecord::categoriesRef()
  Subclasses should re-implement this function to return a reference to the categories for this object.

  \sa categories()
*/

/*!
  \fn const QList<QString> &QPimRecord::categoriesRef() const
  Subclasses should re-implement this function to return a reference to the categories for this object.

  \sa categories()
*/

/*!
  \fn QMap<QString, QString> &QPimRecord::customFieldsRef()
  Subclasses should re-implement this function to return a reference to the custom fields for this object.

  \sa customFields()
*/

/*!
  \fn const QMap<QString, QString> &QPimRecord::customFieldsRef() const
  Subclasses should re-implement this function to return a reference to the custom fields for this object.

  \sa customFields()
*/

/*!
  Destroys the record.
*/

QPimRecord::~QPimRecord()
{
}

/*!
  Returns true if this record and the \a other record have all the same:
  \list
  \o identifier as returned by uid()
  \o categories
  \o parentDependency and parentDependencyType
  \o custom fields
  \o notes
  \endlist
  and otherwise returns false.

  \sa operator!=()
*/
bool QPimRecord::operator==( const QPimRecord &other ) const
{
    if (uidRef() != other.uidRef())
        return false;
    if (categoriesRef().count() != other.categoriesRef().count())
        return false;
    if (QSet<QString>::fromList(categoriesRef()) != QSet<QString>::fromList(other.categoriesRef()))
        return false;
    if (customFieldsRef() != other.customFieldsRef())
        return false;
    if (notes() != other.notes())
        return false;
    return true;
}

/*!
  Returns true if this record and the \a other record are not equal according
  to the contract specified by operator==().

  \sa operator==()
*/
bool QPimRecord::operator!=( const QPimRecord &other ) const
{
    return !(*this == other);
}

/*!
  Sets the record to belong to the given set of \a categories.

  \sa categories(), Categories
*/
void QPimRecord::setCategories( const QList<QString> &categories )
{
    // remove duplicate categories.
    categoriesRef() = categories.toSet().toList();
}

/*!
  Sets the record to belong only to the given category \a identifier.

  \sa categories(), Categories
*/
void QPimRecord::setCategories( const QString & identifier )
{
    QList<QString> newcats;
    newcats.append(identifier);
    categoriesRef() = newcats;
}

/*!
  Renames category \a oldId in record to category \a newId.
*/
void QPimRecord::reassignCategoryId( const QString & oldId, const QString & newId )
{
    QList<QString> &cRef = categoriesRef();
    // workaround for qt bug which gives qWarnings on calling find on an empty array
    if ( !cRef.count() )
        return;

    QMutableListIterator<QString> it(cRef);
    if (it.findNext(oldId))
        it.setValue(newId);
}

/*!
  Removes categories from record that do not appear in the
  given set of valid \a categories.
  Returns true if the record was modified.
*/
bool QPimRecord::pruneDeadCategories(const QList<QString> &categories)
{
    QList<QString> &cRef = categoriesRef();
    QMutableListIterator<QString> it(cRef);
    bool ret = false;
    while(it.hasNext()) {
        QString id = it.next();
        if (!categories.contains(id)) {
            ret = true;
            it.remove();
        }
    }

    return ret;
}


/*!
  Returns the set of categories the record belongs to.

  \sa setCategories(), categoriesRef()
*/
QList<QString> QPimRecord::categories() const
{
    return categoriesRef();
}

/*!
  \fn QUniqueId QPimRecord::uid() const

  Returns the unique identifier for this record.
  
  \sa setUid(), uidRef()
*/

/*!
  \fn void QPimRecord::setUid(const QUniqueId &identifier)

  Sets the record to have given unique \a identifier.

  \sa uid()
*/

/*!
  Returns the string stored for the custom field \a key.
  Returns a null string if the field does not exist.

  \sa setCustomField(), customFields()
 */
QString QPimRecord::customField(const QString &key) const
{
    if (customFieldsRef().contains(key))
        return customFieldsRef()[key];

    return QString();
}

/*!
    Sets the custom fields for the record to the given \a fields.

    Custom fields allow storing data that doesn't fit into the existing
    fields for a given PIM record.

    Keys with empty values will not be stored.

    \sa customFields(), setCustomField()
*/
void QPimRecord::setCustomFields(const QMap<QString, QString> &fields)
{
    QMap<QString, QString> &cRef = customFieldsRef();
    cRef = fields;
    QMutableMapIterator<QString, QString> it(cRef);
    while(it.hasNext()) {
        it.next();
        if (it.value().isEmpty())
            it.remove();
    }
}

/*!
  Sets the string stored for the custom field \a key to the given \a value.

  Keys with empty values will not be stored, and is equivalent to calling removeCustomField().
 */
void QPimRecord::setCustomField(const QString &key, const QString &value)
{
    if (value.isEmpty())
        customFieldsRef().remove(key);
    else
        customFieldsRef().insert(key, value);

}

/*!
  Removes the custom field for the given \a key.
 */
void QPimRecord::removeCustomField(const QString &key)
{
    customFieldsRef().remove(key);
}

/*!
  If this record is a dependant of another record (for example, this
  record is the appointment that represents a contact's birthday),
  this function returns the identifier for the original record.

  Otherwise, returns a null identifier.

  \sa parentDependencyType()
*/
QUniqueId QPimRecord::parentDependency() const
{
    return QPimDependencyList::parentRecord(uid());
}

/*!
  If this record is a dependant of another record (for example, this
  record is the appointment that represents a contact's birthday),
  this function returns the type of the dependency.  The types are
  defined by the PIM library, and include:

  \list
  \o \c "birthday" - the parent record is a QContact, and the dependent
    record is the QAppointment representing that contact's birthday
  \o \c "anniversary" - the parent record is a QContact, and the dependent
    record is the QAppointment representing that contact's anniversary
  \o \c "duedate" - the parent record is a QTask, and the dependent
    record is the QAppointment representing that task's due date.
  \endlist

  If there is no dependency, this will return a null string.

  \sa parentDependency()
*/
QString QPimRecord::parentDependencyType() const
{
    return QPimDependencyList::parentDependencyType(uid());
}


/*!
  Returns the record identifiers for any records that have the
  specified \a type of dependency on this record.

  For example, if this record is a QContact, there may be QAppointment
  objects that have a \c "birthday" or \c "anniversary" dependency
  on this record.

  \sa dependentChildren()
*/
QList <QUniqueId> QPimRecord::dependentChildrenOfType(const QString& type) const
{
    return QPimDependencyList::typedChildrenRecords(uid(), type);
}

/*!
  Returns the identifiers and type of dependency for any records
  that depend on this record.

  \sa dependentChildrenOfType()
*/
QMap<QString, QUniqueId> QPimRecord::dependentChildren() const
{
    return QPimDependencyList::childrenRecords(uid());
}

/* VObject stuff */
static QTextCodec* vobj_codec=0;
static QStringList* comps=0;

void qpe_startVObjectInput()
{
}

void qpe_startVObjectOutput()
{
    QSettings pimConfig("Trolltech","Beam");
    pimConfig.beginGroup("Send");
    QString cs = "UTF-8";
    QString dc = pimConfig.value("DeviceConfig").toString();
    if ( !dc.isEmpty() ) {
        QSettings devcfg(pimConfig.value("DeviceConfig").toString(), QSettings::IniFormat);
        if ( devcfg.status()==QSettings::NoError ) {
            devcfg.beginGroup("Send");
            cs = devcfg.value("CharSet","UTF-8").toString();
            QString comp = devcfg.value("Compatibility").toString();
            comps = new QStringList(comp.split(" "));
        }
    }
    vobj_codec = QTextCodec::codecForName(cs.toLatin1());
}

bool qpe_vobjectCompatibility(const char* misfeature)
{
    return comps && comps->contains(misfeature);
}

void qpe_setVObjectProperty(const QString& name, const QString& value, const char* type, QPimRecord* r)
{
    QCategoryManager m(type, 0);
    if ( name == VCCategoriesProp ) {
        QStringList cl = value.split(",");
        QList<QString> ca;
        for (QStringList::ConstIterator it=cl.begin(); it!=cl.end(); ++it) {
            QString cname = *it;
            if ( cname.left(2) == "X-" ) // compatibility with weird senders
                cname = cname.mid(2);
            if (cname != "Unfiled") { // No tr
                QString cid = m.idForLabel(cname);
                if (cid.isEmpty())
                    cid = m.add(cname);
                if (!cid.isEmpty())
                    ca.append(cid);
            }
        }
        r->setCategories(ca);
    }
}

VObject *qpe_safeAddPropValue( VObject *o, const char *prop, const QString &value )
{
    VObject *ret = 0;
    if ( o && !value.isEmpty() ) {
        if ( vobj_codec )
            ret = addPropValue( o, prop, vobj_codec->fromUnicode(value) );
        else
            ret = addPropValue( o, prop, value.toLatin1() ); // NOT UTF-8, that is by codec
    }
    return ret;
}

VObject *qpe_safeAddProp( VObject *o, const char *prop)
{
    VObject *ret = 0;
    if ( o )
        ret = addProp( o, prop );
    return ret;
}

void qpe_endVObjectInput()
{
}

void qpe_endVObjectOutput(VObject *o, const char* /*type*/, const QPimRecord* r)
{
    QStringList c = r->categories();
    qpe_safeAddPropValue( o, VCCategoriesProp, c.join(",") );
    delete comps;
    comps = 0;
}
