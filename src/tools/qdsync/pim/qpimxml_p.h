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

#ifndef QPIMXML_P_H
#define QPIMXML_P_H

#include <QByteArray>
#include <QSet>
#include <QContact>
#include <QTask>
#include <QAppointment>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class QTaskModel;
class QContactModel;
class QAppointmentModel;

class QCategoryManager;

namespace PIMXML_NAMESPACE 
{


struct QPimXmlException {
    QDate originalDate;
    QString serverId;
    bool replacement;
    QAppointment appointment;
};


class QPimXmlStreamReader : public QXmlStreamReader
{
public:
    QPimXmlStreamReader();
    QPimXmlStreamReader(QIODevice *);
    QPimXmlStreamReader(const char *);
    QPimXmlStreamReader(const QString &);
    QPimXmlStreamReader(const QByteArray &);
    ~QPimXmlStreamReader();

    void readStartContactList();
    void readStartTaskList();
    void readStartAppointmentList();

    QContact readContact(QString &serverId, const QContactModel * = 0);
    QTask readTask(QString &serverId, const QTaskModel * = 0);
    QAppointment readAppointment(QString &serverId, QList<QPimXmlException> &, const QAppointmentModel * = 0);

    QSet<QString> missedLabels() const { return mMissedLabels; }
    void clearMissedLabels() { mMissedLabels.clear(); }

    bool readStartElement(const QString &qualifiedName);
    void readEndElement();
    QString readTextElement(const QString &qualifiedName);
    QDate readDateElement(const QString &qualifiedName);
    QDateTime readDateTimeElement(const QString &qualifiedName, bool utc);
    QUniqueId readIdentifierElement(QString &serverId);
    bool readBooleanElement(const QString &qualifiedName);
    QString readAttribute(const QString &qualifiedName);
private:
    void setCategoryScope(const QString &);
    QString categoryId(const QString &categoryLabel);

    int maxItems();
    void readCategoryElements(QPimRecord &);
    void readCustomFieldElements(QPimRecord &);

    void readBaseAppointmentFields(QAppointment &);

    QXmlStreamAttributes a;
    bool readPast;
    QCategoryManager *categoryManager;
    QSet<QString> mMissedLabels;
};

class QPimXmlStreamWriter : public QXmlStreamWriter
{
public:
    QPimXmlStreamWriter();
    QPimXmlStreamWriter(QByteArray *);
    QPimXmlStreamWriter(QIODevice *);
    QPimXmlStreamWriter(QString *);
    ~QPimXmlStreamWriter();

    void writeStartContactList();
    void writeStartTaskList();
    void writeStartAppointmentList();

    void writeContact(const QContact &);
    void writeTask(const QTask &);
    void writeAppointment(const QAppointment &, const QList<QPimXmlException> &);

private:
    void setCategoryScope(const QString &);
    QString categoryLabel(const QString &categoryId);

    void writeTextElement(const QString &qualifiedName, const QString &value);
    void writeAttribute(const QString &qualifiedName, const QString &value);

    void writeDateElement(const QString &qualifiedName, const QDate &value);
    void writeDateTimeElement(const QString &qualifiedName, const QDateTime &value, bool utc);
    void writeBooleanElement(const QString &qualifiedName, bool value);
    void writeCategoryElements(const QStringList &categories);
    void writeCustomFieldElements(const QMap<QString, QString> &fields);

    void writeBaseAppointmentFields(const QAppointment &);

    QCategoryManager *categoryManager;
};

};

#endif
