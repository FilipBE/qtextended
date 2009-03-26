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

#ifndef PAYLOADMODEL_H
#define PAYLOADMODEL_H

#include <QAbstractItemModel>
#include <QMediaRtpPayload>

class PayloadModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    PayloadModel(QObject *parent = 0);

    void setPayloads(const QList<QMediaRtpPayload> &payloads);
    QList<QMediaRtpPayload> payloads() const;

    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex& index) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    QMediaRtpPayload selectedPayload() const;
    void setSelectedPayload(const QMediaRtpPayload &payload);

private:
    void setSelectedPayloadIndex(int index);

    QList<QMediaRtpPayload> m_payloads;
    int m_selectedPayload;
};

#endif
