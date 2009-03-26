/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef QDBUSMODEL_H
#define QDBUSMODEL_H

#include <QtCore/qabstractitemmodel.h>
#include <QtDBus/QtDBus>

struct QDBusItem;

QT_FORWARD_DECLARE_CLASS(QDomDocument);
QT_FORWARD_DECLARE_CLASS(QDomElement);


class QDBusModel: public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Type { InterfaceItem, PathItem, MethodItem, SignalItem, PropertyItem };

    QDBusModel(const QString &service, const QDBusConnection &connection);
    ~QDBusModel();

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    Type itemType(const QModelIndex &index) const;
    QString dBusPath(const QModelIndex &index) const;
    QString dBusInterface(const QModelIndex &index) const;
    QString dBusMethodName(const QModelIndex &index) const;

    void refresh(const QModelIndex &index = QModelIndex());

    QModelIndex findObject(const QDBusObjectPath &objectPath);

Q_SIGNALS:
    void busError(const QString &text);

private:
    QDomDocument introspect(const QString &path);
    void addMethods(QDBusItem *parent, const QDomElement &iface);
    void addPath(QDBusItem *parent);

    QString service;
    QDBusConnection c;
    QDBusItem *root;
};

#endif

