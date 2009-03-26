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

#ifndef QAPPOINTMENTMODEL_H
#define QAPPOINTMENTMODEL_H

#include <QSet>
#include <QPimModel>
#include <qappointment.h>

#include <qcategorymanager.h>

#include <qpimsource.h>

class QAppointmentModelData;
class QTOPIAPIM_EXPORT QAppointmentModel : public QPimModel
{
    friend class QOccurrenceModel;
    Q_OBJECT

public:
    explicit QAppointmentModel(QObject *parent = 0);
    virtual ~QAppointmentModel();

    enum Field {
        Invalid = -1,

        Description,
        Location,
        Start,
        End,
        AllDay,
        TimeZone,

        Notes,
        Alarm,

        RepeatRule,
        RepeatFrequency,
        RepeatEndDate,
        RepeatWeekFlags,

        Identifier,
        Categories
    };

    /* consider making these just generic added roles */
    enum QAppointmentModelRole {
        LabelRole = Qt::UserRole
    };

    static QString fieldLabel(Field);
    static QIcon fieldIcon(Field k);

    static QString fieldIdentifier(Field);
    static Field identifierField(const QString &);

    int columnCount(const QModelIndex & = QModelIndex()) const;

    // overridden so can change later and provide drag-n-drop (via vcard)
    // later without breaking API compatibility.
    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &, const QVariant &, int);
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &);
    QMap<int,QVariant> itemData(const QModelIndex &) const;

    QVariant headerData(int section, Qt::Orientation orientation,
            int role = Qt::DisplayRole) const;

    QAppointment appointment(const QModelIndex &index) const;
    QAppointment appointment(const QUniqueId &) const;
    QAppointment appointment(int index) const;

    static QVariant appointmentField(const QAppointment &c, QAppointmentModel::Field k);
    static bool setAppointmentField(QAppointment &c, QAppointmentModel::Field k,  const QVariant &);

    bool updateAppointment(const QAppointment& appointment);
    bool removeAppointment(const QAppointment& appointment);
    bool removeAppointment(const QUniqueId& appointment);
    QUniqueId addAppointment(const QAppointment& appointment, const QPimSource & = QPimSource());

    bool removeOccurrence(const QOccurrence& occurrence);
    bool removeOccurrence(const QAppointment& appointment, const QDate &date);
    bool restoreOccurrence(const QUniqueId &, const QDate &);

    bool removeOccurrence(const QUniqueId &id, const QDate &date);
    QUniqueId replaceOccurrence(const QAppointment& appointment, const QOccurrence& replacement, const QDate& date = QDate());
    QUniqueId replaceRemaining(const QAppointment& appointment, const QAppointment& replacement, const QDate& date = QDate());

    // should also be QItemSelection, although isn't that iteratable.
    bool removeList(const QList<QUniqueId> &);

    QUniqueId addRecord(const QByteArray &, const QPimSource &, const QString &format = QString());
    bool updateRecord(const QUniqueId &id, const QByteArray &, const QString &format = QString());
    QByteArray record(const QUniqueId &id, const QString &format = QString()) const;

    bool removeRecord(const QUniqueId &id) { return removeAppointment(id); }

    void setRange(const QDateTime &, const QDateTime &);
    QDateTime rangeStart() const;
    QDateTime rangeEnd() const;

    enum DurationType
    {
        TimedDuration = 0x01,
        AllDayDuration = 0x2,
        AnyDuration = TimedDuration | AllDayDuration,
    };

    void setDurationType(DurationType f);
    DurationType durationType() const;

private:

    static void initMaps();
    static QMap<Field, QString> k2t;
    static QMap<Field, QString> k2i;
    static QMap<QString, Field> i2k;

    QAppointmentModelData *d;
};


class QOccurrenceModelData;
class QTOPIAPIM_EXPORT QOccurrenceModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    QOccurrenceModel(const QDateTime &start, const QDateTime &end, QObject *parent = 0);
    QOccurrenceModel(const QDateTime &start, int count, QObject *parent = 0);

    virtual ~QOccurrenceModel();

    int rowCount(const QModelIndex & = QModelIndex()) const;
    int columnCount(const QModelIndex & = QModelIndex()) const;

    QMimeData * mimeData(const QModelIndexList &) const;
    QStringList mimeTypes() const;

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &, const QVariant &, int);
    bool setItemData(const QModelIndex &, const QMap<int,QVariant> &);
    QMap<int,QVariant> itemData(const QModelIndex &) const;

    int count() const { return rowCount(); }
    bool contains(const QModelIndex &) const;
    bool contains(const QUniqueId &) const;

    QModelIndex parent(const QModelIndex &) const;
    QModelIndex index(int, int = 0, const QModelIndex & = QModelIndex()) const;
    QModelIndex index(const QUniqueId &) const;
    QModelIndex index(const QOccurrence &) const;
    QUniqueId id(const QModelIndex &) const;
    QOccurrence occurrence(const QModelIndex &index) const;
    QOccurrence occurrence(const QUniqueId &, const QDate &) const;
    QOccurrence occurrence(int) const;
    QAppointment appointment(const QUniqueId &id) const;
    QAppointment appointment(const QModelIndex &index) const;
    QAppointment appointment(int index) const;


    void setRange(const QDateTime &, int);
    void setRange(const QDateTime &, const QDateTime &);

    void setDurationType(QAppointmentModel::DurationType f);
    QAppointmentModel::DurationType durationType() const;

    QDateTime rangeStart() const;
    QDateTime rangeEnd() const;

    // TODO extra passthrough functions
    void setCategoryFilter(const QCategoryFilter &);
    QCategoryFilter categoryFilter() const;
    QSet<QPimSource> visibleSources() const;
    void setVisibleSources(const QSet<QPimSource> &);
    QSet<QPimSource> availableSources() const;
    bool sourceExists(const QPimSource &source, const QUniqueId &id) const;
    bool editable(const QModelIndex &index) const;
    bool editable(const QUniqueId &) const;

    // refresh, fetching, completeFetch and fetchCompleted
    // no longer do anything.
    void refresh();
    // will want similar fetch calls for contacts:sim
    bool fetching() const;
    void completeFetch(); // can be expensive
signals:
    void fetchCompleted();

public slots:
    // there is no longer any reason for calling this function
    void rebuildCache() const;

private slots:
    void forwardAppointmentReset();

private:
    QVariant appointmentData(int row, int column) const;
    void init(QAppointmentModel *appointmentModel);

    QOccurrenceModelData *od;
};

#endif
