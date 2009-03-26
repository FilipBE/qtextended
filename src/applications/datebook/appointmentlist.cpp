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

#include "appointmentlist.h"

AppointmentList::AppointmentList(QWidget *parent)
: QListView(parent)
{
    maxFoldedHeight = 0;
    maxUnfoldedHeight = 0;
    maxRows = 0;
    folded = false;
    occurrenceModel = 0;
    setUniformItemSizes(true);
    setFrameShape(QFrame::NoFrame);
}

AppointmentList::~AppointmentList()
{
}

void AppointmentList::setFolded(bool f)
{
    folded = f;
    recalculateHeight();
}

void AppointmentList::recalculateHeight()
{
    int rows = model()->rowCount();

    if (rows == 0) {
        hide();
        emit changeHiddenCount(0);
    } else {
        int itemHeight = sizeHintForRow(0);
        maxRows = qMax(1, maxFoldedHeight / itemHeight);

        if (folded && rows > maxRows) {
            //  Show 1 less than technically possible; save room for "more" label
            maxRows--;
            setFixedHeight(maxRows * itemHeight);
            emit changeHiddenCount(rows - maxRows);
        } else {
            int h = rows * itemHeight;
            int unfoldedrows = maxUnfoldedHeight / itemHeight;

            if (parentWidget()) {
                int parentRows = parentWidget()->height() / itemHeight;
                if (unfoldedrows > 0) {
                    if (parentRows < unfoldedrows)
                        unfoldedrows = parentRows;
                } else
                    unfoldedrows = parentRows;
            }
            if (unfoldedrows > 0)
                h = qMin(h, itemHeight * unfoldedrows);

            setFixedHeight(h);
            emit changeHiddenCount(0);
        }

        if (folded)
            setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        else
            setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        show();
    }
}

void AppointmentList::setModel(QAbstractItemModel *absModel)
{
    QOccurrenceModel *model = qobject_cast<QOccurrenceModel*>(absModel);
    if (!model)
        return;

    occurrenceModel = model;
    connect(model, SIGNAL(modelReset()), this, SLOT(recalculateHeight()));
    QListView::setModel(model);
}

int AppointmentList::visibleRowCount() const
{
    if (folded)
        return qMin(model()->rowCount(), maxRows);
    else
        return model()->rowCount();
}

void AppointmentList::setMaximumFoldedHeight(int height)
{
    maxFoldedHeight = height;
    recalculateHeight();
}

void AppointmentList::setMaximumUnfoldedHeight(int height)
{
    maxUnfoldedHeight = height;
    recalculateHeight();
}

bool AppointmentList::isFolded() const
{
    return folded;
}

bool AppointmentList::provideFoldingOption()
{
    int rows = model()->rowCount();
    recalculateHeight();

    if ( rows > maxRows ) {
        return true;
    } else {
        return false;
    }
}


