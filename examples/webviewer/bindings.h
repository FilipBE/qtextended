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

#ifndef BINDINGS_H
#define BINDINGS_H

#include <QObject>
#include <QValueSpaceItem>
#include <QHash>

#if defined(QTOPIA_TELEPHONY)
#include <QCallList>
#endif
#include <QUniqueId>
#include <QtopiaService>
#include <QtopiaServiceRequest>
#include <QMailAddress>
#include <QMailMessage>
#include <QVariant>
#include <QOccurrenceModel>
#include <QDateTime>
#include <QDebug>
class VSIWrapper : public QObject
{
    Q_OBJECT;
public:
    VSIWrapper(const QString& vsroot, QObject* parent);

    Q_PROPERTY(QString itemName READ itemName);
    QString itemName() {return m_item.itemName();}

    Q_PROPERTY(QList<QString> subPaths READ subPaths);
    QList<QString> subPaths() const {return m_item.subPaths();}

signals:
    void contentsChanged();
public slots:
    void remove() {m_item.remove();}
    void remove(const QString& path) {m_item.remove(path);}
    void setValue(const QVariant& value) {m_item.setValue(value);}
    void setValue(const QString& path, const QVariant& value) {m_item.setValue(path, value);}
    QVariant value(const QString& path = QString(), const QVariant& def = QVariant()) {return m_item.value(path, def);}
    bool sync() {return m_item.sync();}

    QString toString() {return value().toString();}

protected:
    QValueSpaceItem m_item;
    QString m_root;
};

class ServiceWrapper : public QObject
{
    Q_OBJECT;
public:
    ServiceWrapper(QObject *parent, const QString &name);
public slots:
    void raise();
protected:
    QString m_name;
};

class ContactServiceWrapper : public ServiceWrapper
{
    Q_OBJECT;
public:
    ContactServiceWrapper(QObject *parent) : ServiceWrapper(parent, "Contacts") {}
public slots:
    void showContact(const QString& id) {QtopiaServiceRequest req("Contacts", "showContact(QUniqueId)"); req << QUniqueId(id); req.send();}
    void addPhoneNumberToContact(const QString& number) {QtopiaServiceRequest req("Contacts", "addPhoneNumberToContact(QString)"); req << number; req.send();}
    void editPersonal() {QtopiaServiceRequest req("Contacts", "editPersonal()"); req.send();}
    void editPersonalAndClose() {QtopiaServiceRequest req("Contacts", "editPersonalAndClose()"); req.send();}
};

#if defined(QTOPIA_TELEPHONY)
class CallHistoryServiceWrapper : public ServiceWrapper
{
    Q_OBJECT;
public:
    CallHistoryServiceWrapper(QObject *parent) : ServiceWrapper(parent, "CallHistory") {}
public slots:
    void showCallHistory(int type=0, const QString& filter = QString()) {QtopiaServiceRequest req("CallHistory", "showCallHistory(QCallList::ListType,QString)"); req << (QCallList::ListType) type << filter; req.send();}
};

class DialerServiceWrapper : public ServiceWrapper
{
    Q_OBJECT;
public:
    DialerServiceWrapper(QObject *parent) : ServiceWrapper(parent, "Dialer") {}
public slots:
    void showDialer(const QString& filter = QString()) {QtopiaServiceRequest req("Dialer", "showDialer(QString)"); req << filter; req.send();}
};

#endif

class CalendarServiceWrapper : public ServiceWrapper
{
    Q_OBJECT;
public:
    CalendarServiceWrapper(QObject *parent) : ServiceWrapper(parent, "Calendar") {}
public slots:
    void showOccurrence(const QString& id, const QDate& dt = QDate())
    {
        QUniqueId uid(id);
        if (dt.isValid())
        {
            QtopiaServiceRequest req("Calendar", "showAppointment(QUniqueId,QDate)");
            req << QUniqueId(id) << dt;
            req.send();
        } else {
            QtopiaServiceRequest req("Calendar", "showAppointment(QUniqueId)");
            req << QUniqueId(id);
            req.send();
        }
    }
    void raiseToday() {QtopiaServiceRequest req("Calendar", "raiseToday()"); req.send();}
};

class MessagesServiceWrapper : public ServiceWrapper
{
    Q_OBJECT;
public:
    MessagesServiceWrapper(QObject *parent) : ServiceWrapper(parent, "Messages") {}
public slots:
    void viewNewMessages(bool user=false) {QtopiaServiceRequest req("Messages", "viewNewMessages(bool)"); req << user; req.send();}
    void composeMessage(int type, QStringList addresses, QString subject, QString text)
    {
        QtopiaServiceRequest req("Messages", "composeMessage(QMailMessage::MessageType, QMailAddressList, QString, QString)");
        req << type << QMailAddress::fromStringList(addresses) << subject << text;
        req.send();
    }
};

class ServicesWrapper : public QObject
{
    Q_OBJECT;
public:
    ServicesWrapper(QObject *parent);
protected:
};


class OccurrenceModelWrapper : public QOccurrenceModel
{
    Q_OBJECT;
public:
    OccurrenceModelWrapper(QObject *parent, const QDateTime& start, int count);
    OccurrenceModelWrapper(QObject *parent, const QDateTime& start, const QDateTime& end);

public slots:
    QVariantMap appointment(int index);
    QVariantMap appointment(const QString& uid, const QDate& = QDate());
    QDateTime rangeStart() const {return QOccurrenceModel::rangeStart();}
    QDateTime rangeEnd() const {return QOccurrenceModel::rangeEnd();}
    void setRange(const QDateTime& start, const QDateTime& end) {QOccurrenceModel::setRange(start, end);}
    void setRange(const QDateTime& start, int count) {QOccurrenceModel::setRange(start, count);}
    int rowCount() const {return QOccurrenceModel::rowCount();}

protected:
    QVariantMap wrapOccurrence(const QOccurrence& a);
};

class QtExtendedWrapper : public QObject
{
    Q_OBJECT;
public:
    QtExtendedWrapper(QObject *parent, QWidget *parentWidget);

public slots:
    QObject* valuespace(const QString&);
    QVariantMap currentLocation();
    QVariantMap weather(QVariantMap location);
    void showLauncher();
    void showFavorites();
    void lockDevice();
    void unlockDevice();
    QObject* appointments(const QDateTime& start, int count);
    QObject* appointments(const QDateTime& start, const QDateTime& end);
    QString formatDate(const QDate& date, const QDate& today = QDate());
    QString formatDateTime(const QDateTime& date, const QDate& today = QDate());

    QString tr(const QString& context, const QString& msg, const QString& comment = QString(), int n=-1);

    void setCaption(const QString& msg);

signals:
    void weatherChanged();
    void locationChanged();

protected:
    // Our super secret collection of cached valuespace entries
    QHash<QString, VSIWrapper*> _vsis;
    QPointer<QWidget> m_widget;
    QPointer<QDialog> m_lockDialog;
};

QObject *createBindingHierarchy(QObject *parent, QWidget* widget);

#endif
