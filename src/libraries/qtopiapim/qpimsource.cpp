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

#include "qpimsource.h"
#include <QIcon>

/*!
  \fn uint qHash(const QPimSource &s)
  \ingroup pim
  \relates QPimSource

  Returns a hash value for the source \a s.
*/
QTOPIAPIM_EXPORT uint qHash(const QPimSource &s)
{
    return qHash(s.context) + qHash(s.identity);
}


/*!
  \class QPimSource
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimSource class holds identifying information for a storage source of PIM data.

  The QPimSource class includes:
  \list
  \o a universal identifier representing the QPimContext that created the source
  \o an identity string representing the source within that context.
  \endlist

  \sa QAppointmentModel, QContactModel, QTaskModel, {Pim Library}
*/

/*!
  \variable QPimSource::context
  \brief the unique id representing the QPimContext that controls the source.
*/

/*!
  \variable QPimSource::identity
  \brief the context string to identify multiple sources from a single QPimContext.
*/

/*!
  \fn bool QPimSource::operator==(const QPimSource &other) const

  Returns true if this source is equal to \a other, otherwise false.
*/

/*!
  \fn bool QPimSource::operator!=(const QPimSource &other) const

  Returns true if this source is not equal to \a other, otherwise false.
*/

/*!
  \fn bool QPimSource::operator<(const QPimSource &other) const
  Returns true if this source is less than \a other, otherwise false.
*/

/*!
  \fn bool QPimSource::isNull() const
  Returns true if the source is null, otherwise false.
*/


/*!
  \class QPimContext
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QPimContext class represents a storage context of PIM data.

  This includes storage such as SIM Card contacts or data stored on the
  device in the system PIM database.  The class can be used to perform
  operations that relate to a specific context of PIM data.

  QPimContext should not be instantiated directly.  Instead, one of the data type
  specific contexts should be used.  These are \l QContactContext,
  \l QTaskContext and \l QAppointmentContext.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.

  \sa QAppointmentModel, QContactModel, QTaskModel, {Pim Library}
*/

/*!
  Constructs a QPimContext with parent \a parent.
*/
QPimContext::QPimContext(QObject *parent)
    : QObject(parent)
{
}

/*!
  Returns an icon associated with the context.  The default implementation
  returns a null icon.
*/
QIcon QPimContext::icon() const { return QIcon(); }

/*!
  Returns true if any PIM records stored by the context can be edited.
  The default implementation returns false.
*/
bool QPimContext::editable() const { return false; }

/*!
  \fn QString QPimContext::description() const

  Returns text describing the PIM context, in a format suitable for displaying to the user.
*/

/*!
  \fn QString QPimContext::title() const

  Returns the title of the PIM context, in a format suitable for displaying to the user.
*/

/*!
  \fn QString QPimContext::title(const QPimSource &source) const

  Returns the title of the PIM data \a source, in a format suitable for displaying to the user.
  Returns the title for the context by default.
*/


/*!
  \fn bool QPimContext::editable(const QUniqueId &id) const

  Returns true if the PIM record identified by \a id can be edited by this context,
  otherwise false.
*/

/*!
  \fn void QPimContext::setVisibleSources(const QSet<QPimSource> &visible)

  Filters the model that created this context to only show records for the sources
  that are contained in \a visible.  Does not affect data from other contexts.
*/

/*!
  \fn QSet<QPimSource> QPimContext::visibleSources() const

  Returns the set of PIM data sources that are controlled by this context and are
  visible in the model that created the context.
*/

/*!
  \fn QSet<QPimSource> QPimContext::sources() const

  Returns the list of PIM data sources that are controlled by this context.
*/

/*!
  \fn QPimSource QPimContext::defaultSource() const

  Returns the default PIM data source that is controlled by this context.

  If there is no default PIM data source returns a null source.
*/

/*!
  \fn QUuid QPimContext::id() const

  Returns a unique identifier for this context.
*/

/*!
  \fn bool QPimContext::exists(const QUniqueId &id) const

  Returns true if the PIM record identified by \a id exists in a PIM data source
  controlled by this context.  Otherwise returns false.
  \sa source()
*/

/*!
  \fn bool QPimContext::exists(const QUniqueId &id, const QPimSource &source) const

  Returns true if the contact identified by \a id exists in the PIM data \a source
  and the \a source is controlled by this context.  Otherwise returns false.
  \sa source()
*/

/*!
  \fn QPimSource QPimContext::source(const QUniqueId &id) const

  Returns the PIM data source identified by \a id where the record is stored, if it exists.
  Returns a null PIM data source if there is no record for \a id.
  \sa exists()
*/

/*!
  \fn void QPimSource::serialize(Stream &value) const
  \internal

  Serializes the QPimSource out to a template
  type \c{Stream} \a stream.
*/
template <typename Stream> void QPimSource::serialize(Stream &out) const
{
    out << context;
    out << identity;
    return out;
}

/*!
  \fn void QPimSource::deserialize(Stream &value)
  \internal

  Deserializes the QPimSource out to a template
  type \c{Stream} \a stream.
*/
template <typename Stream> void QPimSource::deserialize(Stream &in)
{
    in >> context;
    in >> identity;
    return in;
}

/*!
  \class QContactContext
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QContactContext class represents a storage context of contact data.

  This includes storage contexts such as SIM Card contacts or contacts stored
  on the phone in the system PIM database.  The class can be used to perform
  operations that relate to a specific context of PIM data.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.

  \sa QContactModel, {Pim Library}
*/

/*!
  \fn bool QContactContext::updateContact(const QContact &contact)

  Updates the contact with the same identifier as \a contact if it exists in this context.
  Returns true upon success; otherwise returns false.
*/

/*!
  \fn bool QContactContext::removeContact(const QUniqueId &id)

  Removes the contact with the identifier \a id if it exists in this context.
  Returns true upon success; otherwise returns false.
*/

/*!
  \fn QUniqueId QContactContext::addContact(const QContact &contact, const QPimSource &source)

  Adds the \a contact to the PIM data \a source if it is controlled by this context.
  If the contact is successfully added, returns the new unique id for this contact,
  otherwise returns a null id.
*/

/*!
  \fn QList<QContact> QContactContext::exportContacts(const QPimSource &source, bool &ok) const

  Exports the contacts stored in the PIM data \a source and returns them as a list.
  The source must be controlled by this context.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  There isn't necessarily going to be a one to one match to contacts in the PIM
  data source.  Contacts may be merged or split over multiple contacts to form
  the list.
*/

/*!
  \fn bool QContactContext::importContacts(const QPimSource &source, const QList<QContact> &contacts)

  Imports the \a contacts and merges them with the contacts listed in the
  PIM data \a source.  The source must be controlled by this context.
  If successful returns true, otherwise returns false.

  There isn't necessarily going to be a one to one match to contacts in the PIM
  data source.  Contacts may be merged or split over multiple contacts.  Also
  contacts that match in name will be updated from the list rather than additional
  contacts created.
*/

/*!
  \fn QContact QContactContext::exportContact(const QUniqueId &id, bool &ok) const

  Exports a single contact identified by \a id.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  \sa exportContacts()
*/

/*!
  \fn bool QContactContext::importContact(const QPimSource &source, const QContact &contact)

  Imports a single \a contact to the PIM data \a source.  If successful returns
  true, otherwise returns false.

  \sa importContacts()
*/

/*!
  \fn QContactContext::QContactContext(QObject *parent)

  Constructs a QContactContext with parent \a parent.
*/

/*!
  \class QAppointmentContext
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QAppointmentContext class represents a storage context of appointment data.

  The class can be used to perform operations that relate to a specific context
  of PIM data, such as appointments stored on a device in the native Qt Extended format, or appointments stored in an on-line calendaring service.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.

  \sa QAppointmentModel, {Pim Library}
*/

/*!
  \fn bool QAppointmentContext::updateAppointment(const QAppointment &appointment)

  Updates the appointment with the same identifier as \a appointment if it exists in this context.
  Returns true upon success; otherwise returns false.
*/

/*!
  \fn bool QAppointmentContext::removeAppointment(const QUniqueId &id)

  Removes the appointment with the identifier \a id if it exists in this context.
  Returns true upon success; otherwise returns false.
*/

/*!
  \fn QUniqueId QAppointmentContext::addAppointment(const QAppointment &appointment, const QPimSource &source)

  Adds the \a appointment to the PIM data \a source if it is controlled by this context.
  If the appointment is successfully added, returns the new unique id for this appointment,
  otherwise returns a null id.
*/

/*!
  \fn bool QAppointmentContext::removeOccurrence(const QUniqueId &id, const QDate &date)

  Mark the repeating appointment identified by \a id in this context so as not
  to occur on \a date.  Returns true if the appointment was successfully updated,
  otherwise returns false.
*/

/*!
  \fn QUniqueId QAppointmentContext::replaceOccurrence(const QUniqueId &id, const QOccurrence &occurrence, const QDate& date)

  Replaces an occurrence that occurs on \a date of the appointment identified
  by \a id with \a occurrence.  If \a date is null, the date of the supplied
  \a occurrence will be used.
  Returns true if the appointment was successfully updated, otherwise returns false.
*/

/*!
  \fn bool QAppointmentContext::restoreOccurrence(const QUniqueId &identifier, const QDate &date)

  If an recurring appointment with the specified \a identifier
  has an exception listed for the given \a date, restores the original
  occurrence for that date.  If the exception included a replacement
  appointment will also remove the replacement.

  Returns true if the appointment was successfully updated.
*/

/*!
  \fn QUniqueId QAppointmentContext::replaceRemaining(const QUniqueId &id, const QAppointment &appointment, const QDate& date)

  Modifies the appointment identified by \a id to not repeat after \a date and
  adds \a appointment to the PIM data source that stores the appointment
  identified by \a id.  If \a date is null, the date of the first occurrence
  of \a appointment will be used.
  Returns the unique id for the new appointment if successful, otherwise returns a null id.
*/

/*!
  \fn QList<QAppointment> QAppointmentContext::exportAppointments(const QPimSource &source, bool &ok) const

  Exports the appointments stored in the PIM data \a source and returns them as a list.
  The source must be controlled by this context.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  There isn't necessarily going to be a one to one match to appointments in the PIM
  data source.  Appointments may be merged or split over multiple appointments to form
  the list.
*/

/*!
  \fn bool QAppointmentContext::importAppointments(const QPimSource &source, const QList<QAppointment> &appointments)

  Imports the \a appointments and merges them with the appointments listed in the
  PIM data \a source.  The source must be controlled by this context.
  If successful returns true, otherwise returns false.

  There isn't necessarily going to be a one to one match to appointments in the PIM
  data source.  Appointments may be merged or split over multiple appointments.  Also
  appointments that match in name will be updated from the list rather than additional
  appointments created.
*/

/*!
  \fn QAppointment QAppointmentContext::exportAppointment(const QUniqueId &id, bool &ok) const

  Exports a single appointment identified by \a id.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  \sa exportAppointments()
*/

/*!
  \fn bool QAppointmentContext::importAppointment(const QPimSource &source, const QAppointment &appointment)

  Imports a single \a appointment to the PIM data \a source.  If successful returns
  true, otherwise returns false.

  \sa importAppointments()
*/

/*!
  \fn QAppointmentContext::QAppointmentContext(QObject *parent)

  Constructs a QAppointmentContext with parent \a parent.
*/

/*!
  \class QTaskContext
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QTaskContext class represents a storage context of task data.

  The class can be used to perform operations that relate to a specific context
  of PIM data, such as tasks stored on a device in the native Qt Extended format.

  Currently there is no way for applications to implement their own contexts.
  This feature is being considered for future versions of Qtopia.

  \sa QTaskModel, {Pim Library}
*/

/*!
  \fn bool QTaskContext::updateTask(const QTask &task)

  Updates the task with the same identifier as \a task if it exists in this context.
  Returns true upon success; otherwise returns false.
*/

/*!
  \fn bool QTaskContext::removeTask(const QUniqueId &id)

  Removes the task with the identifier \a id if it exists in this context.
  Returns true upon success; otherwise returns false.

*/

/*!
  \fn QUniqueId QTaskContext::addTask(const QTask &task, const QPimSource &source)

  Adds the \a task to the PIM data \a source if it is controlled by this context.
  If the task is successfully added, returns the new unique id for this task,
  otherwise returns a null id.
*/

/*!
  \fn QList<QTask> QTaskContext::exportTasks(const QPimSource &source, bool &ok) const

  Exports the tasks stored in the PIM data \a source and returns them as a list.
  The source must be controlled by this context.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  There isn't necessarily going to be a one to one match to tasks in the PIM
  data source.  Tasks may be merged or split over multiple tasks to form
  the list.
*/

/*!
  \fn bool QTaskContext::importTasks(const QPimSource &source, const QList<QTask> &tasks)

  Imports \a tasks and merges them with the tasks listed in the
  PIM data \a source.  The source must be controlled by this context.
  If successful returns true, otherwise returns false.

  There isn't necessarily going to be a one to one match to tasks in the PIM
  data source.  Tasks may be merged or split over multiple tasks.  Also
  tasks that match in name will be updated from the list rather than additional
  tasks created.
*/

/*!
  \fn QTask QTaskContext::exportTask(const QUniqueId &id, bool &ok) const

  Exports a single task identified by \a id.  If successful sets \a ok to true,
  otherwise sets \a ok to false.

  \sa exportTasks()
*/

/*!
  \fn bool QTaskContext::importTask(const QPimSource &source, const QTask &task)

  Imports a single \a task to the PIM data \a source.  If successful returns
  true, otherwise returns false.

  \sa importTasks()
*/

/*!
  \fn QTaskContext::QTaskContext(QObject *parent)

  Constructs a QTaskContext with parent \a parent.
*/
