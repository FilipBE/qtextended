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

#ifndef LISTPOSITIONBAR_H
#define LISTPOSITIONBAR_H

#include <QWidget>
#include <QString>
#include <QPixmap>

class ListPositionBar : public QWidget
{
    Q_OBJECT;

public:
    ListPositionBar(QWidget *parent = 0);
    ~ListPositionBar();

    void setPosition(int current, int max);

    void setMessage(const QString& formatString);

protected:
    void paintEvent(QPaintEvent *pe);
    void mousePressEvent(QMouseEvent *me);

signals:
    void nextPosition();
    void previousPosition();

protected:
    QString mPosition;
    QString mFormat;
    int mMetric;
    int mCurrent;
    int mMax;
    bool mShowPrev;
    bool mShowNext;
    QPixmap mLeftPixmap;
    QPixmap mRightPixmap;
};

#endif
