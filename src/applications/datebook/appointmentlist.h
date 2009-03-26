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

#ifndef APPOINTMENTLIST_H
#define APPOINTMENTLIST_H

#include <qappointment.h>
#include <qappointmentmodel.h>

#include <QListView>

class AppointmentList : public QListView
{
    Q_OBJECT

public:
    AppointmentList(QWidget *parent = 0);
    virtual ~AppointmentList();

    void setFolded(bool f = true);
    void setModel(QAbstractItemModel *model);
    void setMaximumFoldedHeight(int height);
    void setMaximumUnfoldedHeight(int height);
    bool isFolded() const;
    bool provideFoldingOption();

    int visibleRowCount() const;

signals:
    void changeHiddenCount(int hidden);

public slots:
    void recalculateHeight();

private:
    bool folded;
    int maxFoldedHeight;
    int maxUnfoldedHeight;
    int maxRows;
    QOccurrenceModel *occurrenceModel;
};
#endif
