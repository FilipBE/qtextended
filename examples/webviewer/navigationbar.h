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

#ifndef NAVIGATIONBAR_H
#define NAVIGATIONBAR_H

#include <QWidget>

class QToolButton;

class NavigationBar : public QWidget
{
    Q_OBJECT

public:
    NavigationBar( QWidget* parent = 0);

    void setBack(QAction*);
    void setForward(QAction*);

public slots:
    void labelsChanged(const QString &previous,const QString &next);

private:
    // Called by ctor.
    void init();

    QToolButton *createButton(Qt::ArrowType);

    // The left button can be used as a backwards navigator.
    QToolButton *leftBn;

    // The right button can be used as a forwards navigator.
    QToolButton *rightBn;

    void updateNormalSize();
    int normalSize;
};

#endif
