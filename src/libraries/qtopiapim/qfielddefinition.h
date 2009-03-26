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

#ifndef FIELDDEFINITIONS_H
#define FIELDDEFINITIONS_H

#include <QString>
#include <QList>
#include <QPimRecord>
#include <QIcon>
#include <QSharedData>
#include <QtopiaServiceDescription>

class QFieldDefinition;
class QFieldDefinitionData;
class QFieldActionData;
class QAppointment;
class QContact;
class QTask;
class QTOPIAPIM_EXPORT QFieldDefinition
{
    friend class QFieldDefinitionData;
public:
    QFieldDefinition();
    QFieldDefinition(const QFieldDefinition &other);
    virtual ~QFieldDefinition();

    QString id() const;
    QString label() const;
    QIcon icon() const;
    QString iconString() const;
    QString inputHint() const;

    QStringList editActions() const;
    QStringList browseActions() const;

    bool hasTag(const QString &tag) const;
    QStringList tags() const;

    QFieldDefinition &operator=(const QFieldDefinition &def);

protected:
    QStringList attributeKeys() const;
    QVariant attribute(const QString &key) const;

private:
    QSharedDataPointer<QFieldDefinitionData> d;
};

class QTOPIAPIM_EXPORT QContactFieldDefinition : public QFieldDefinition
{
public:
    QContactFieldDefinition();
    QContactFieldDefinition(const QString &);
    ~QContactFieldDefinition();

    QVariant value(const QContact &) const;
    void setValue(QContact &, const QVariant &) const;

    QString provider() const;

    static QStringList fields(const QString &tag);

    static QString actionLabel(const QString &);
    static QString actionIconString(const QString &);
    static QIcon actionIcon(const QString &action) { return QIcon(actionIconString(action)); }

    static QtopiaServiceDescription actionDescription(const QString &);

    static QtopiaServiceRequest actionRequest(const QString &, const QContact &, const QString &value);

private:
    static QFieldDefinition field(const QString &id);
    static void init();
};

class QTOPIAPIM_EXPORT QTaskFieldDefinition : public QFieldDefinition
{
public:
    QTaskFieldDefinition();
    QTaskFieldDefinition(const QString &);
    ~QTaskFieldDefinition();

    QVariant value(const QTask &) const;
    void setValue(QTask &, const QVariant &) const;

    static QStringList fields(const QString &);

    static QString actionLabel(const QString &);
    static QString actionIconString(const QString &);
    static QIcon actionIcon(const QString &action) { return QIcon(actionIconString(action)); }

    static QtopiaServiceDescription actionDescription(const QString &);

    static QtopiaServiceRequest actionRequest(const QString &, const QTask &, const QString &value);

private:
    static QFieldDefinition field(const QString &id);
    static void init();
};

class QTOPIAPIM_EXPORT QAppointmentFieldDefinition : public QFieldDefinition
{
public:
    QAppointmentFieldDefinition();
    QAppointmentFieldDefinition(const QString &);
    ~QAppointmentFieldDefinition();

    QVariant value(const QAppointment &) const;
    void setValue(QAppointment &, const QVariant &) const;

    static QStringList fields(const QString &);

    static QString actionLabel(const QString &);
    static QString actionIconString(const QString &);
    static QIcon actionIcon(const QString &action) { return QIcon(actionIconString(action)); }
    static QtopiaServiceDescription actionDescription(const QString &);
    static QtopiaServiceRequest actionRequest(const QString &, const QAppointment &, const QString &value);

private:
    static QFieldDefinition field(const QString &id);
    static void init();
};

#endif
