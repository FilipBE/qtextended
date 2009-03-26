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

#ifndef PICTUREFLOWVIEW_H
#define PICTUREFLOWVIEW_H
#include "pictureflow.h"
#include <QModelIndex>

class PictureFlowViewPrivate;
class QAbstractItemModel;
class QItemSelectionModel;
class QAbstractItemDelegate;

class PictureFlowView : public PictureFlow
{
    Q_OBJECT
    Q_PROPERTY(int modelRole READ modelRole WRITE setModelRole)

public:
    PictureFlowView(QWidget* parent = 0);
    virtual ~PictureFlowView();

    void setModel (QAbstractItemModel * model);
    QAbstractItemModel *model() const;

    virtual QModelIndex currentModelIndex();

    void setModelRole(int role);
    int modelRole();

    void setSelectionModel(QItemSelectionModel *selectionModel);
    QItemSelectionModel* selectionModel();

    void setItemDelegate(QAbstractItemDelegate *delegate);

signals:
    void activated(const QModelIndex &index);
    void currentChanged(const QModelIndex &index);

protected:
    virtual void hideEvent(QHideEvent*);
    virtual void showEvent(QShowEvent*);

private:
    friend class PictureFlowViewPrivate;
    PictureFlowViewPrivate *d;
};

#endif
