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

#include "payloadmodel.h"

PayloadModel::PayloadModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_selectedPayload(0)
{
}

void PayloadModel::setPayloads(const QList<QMediaRtpPayload> &payloads)
{
    m_payloads = payloads;

    emit reset();
}

QList<QMediaRtpPayload> PayloadModel::payloads() const
{
    return m_payloads;
}

int PayloadModel::columnCount(const QModelIndex &parent) const
{
    return !parent.isValid()
        ? 4
        : 0;
}

int PayloadModel::rowCount (const QModelIndex &parent) const
{
    return !parent.isValid()
        ? m_payloads.count()
        : 0;
}

QModelIndex PayloadModel::index(int row, int column, const QModelIndex &parent) const
{
    return !parent.isValid() && row >= 0 && row < m_payloads.count() && column >= 0 && column < 4
        ? createIndex( row, column )
        : QModelIndex();
}

QModelIndex PayloadModel::parent (const QModelIndex &index) const
{
    Q_UNUSED( index );

    return QModelIndex();
}

Qt::ItemFlags PayloadModel::flags(const QModelIndex &index) const
{
    return index.column() == 0
        ? QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable
        : QAbstractItemModel::flags(index);
}

QVariant PayloadModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        if (role == Qt::DisplayRole) {
            switch (index.column()) {
            case 0:
                return m_payloads.at(index.row()).id();
            case 1:
                return m_payloads.at(index.row()).encodingName();
            case 2:
                return m_payloads.at(index.row()).clockRate();
            case 3:
                return m_payloads.at(index.row()).channels();
            }
        } else if (role == Qt::CheckStateRole && index.column() == 0) {
            return index.row() == m_selectedPayload
                    ? Qt::Checked
                    : Qt::Unchecked;
        }
    }

    return QVariant();
}

bool PayloadModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value);

    if (role == Qt::CheckStateRole && index.column() == 0 && index.row() != m_selectedPayload) {
        setSelectedPayloadIndex(index.row());

        return true;
    }

    return false;
}

QVariant PayloadModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("ID");
        case 1:
            return tr("Encoding Name");
        case 2:
            return tr("Clock Rate");
        case 3:
            return tr("Channels");
        }
    }
    return QVariant();
}

void PayloadModel::setSelectedPayloadIndex(int index)
{
    if (index >= 0 && index < m_payloads.count()) {
        int oldSelectedPayload = m_selectedPayload;

        m_selectedPayload = index;

        emit dataChanged(createIndex(oldSelectedPayload, 0), createIndex(oldSelectedPayload, 0));
        emit dataChanged(createIndex(m_selectedPayload, 0), createIndex(m_selectedPayload, 0));
    }
}

QMediaRtpPayload PayloadModel::selectedPayload() const
{
    return m_payloads.value(m_selectedPayload);
}

void PayloadModel::setSelectedPayload(const QMediaRtpPayload &payload)
{
    for (int i = 0; i < m_payloads.count(); ++i) {
        if (payload.isEquivalent(m_payloads.at(i))) {
            setSelectedPayloadIndex(i);
            break;
        }
    }
}
