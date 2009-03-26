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

#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QIcon>
#include <QMailMessageSet>
#include <QPair>
#include <QString>


class FolderModel : public QMailMessageSetModel
{
    Q_OBJECT

public:
    enum Roles
    {
        FolderIconRole = Qt::DecorationRole,
        FolderStatusRole = QMailMessageSetModel::SubclassUserRole,
        FolderStatusDetailRole,
        FolderIdRole
    };

    FolderModel(QObject *parent = 0);
    ~FolderModel();

    static QString excessIndicator();

    using QMailMessageSetModel::data;
    virtual QVariant data(QMailMessageSet *item, int role, int column) const;

protected slots:
    void processUpdatedItems();

protected:
    typedef QPair<QString, QString> StatusText;

    void scheduleUpdate(QMailMessageSet *item);

    virtual void appended(QMailMessageSet *item);
    virtual void updated(QMailMessageSet *item);
    virtual void removed(QMailMessageSet *item);

    virtual QIcon itemIcon(QMailMessageSet *item) const;
    virtual QString itemStatus(QMailMessageSet *item) const;
    virtual QString itemStatusDetail(QMailMessageSet *item) const;

    virtual StatusText itemStatusText(QMailMessageSet *item) const;
    virtual StatusText folderStatusText(QMailFolderMessageSet *item) const;
    virtual StatusText accountStatusText(QMailAccountMessageSet *item) const;
    virtual StatusText filterStatusText(QMailFilterMessageSet *item) const;

    enum SubTotalType { Unread, New, Unsent };

    static QString describeFolderCount(int total, int subTotal, SubTotalType type = Unread);

    QMap<QMailMessageSet*, StatusText> statusMap;

    QList<QMailMessageSet*> updatedItems;
};


#endif

