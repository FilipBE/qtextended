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

#ifndef QTOPIATABWIDGET_H
#define QTOPIATABWIDGET_H

#include <QTabWidget>
#include <QMap>

class QScrollArea;
class QDelayedScrollArea;
class QtopiaTabWidget : public QTabWidget
{
    Q_OBJECT
public:
    
    QtopiaTabWidget(QWidget *parent = 0);
    ~QtopiaTabWidget();

    // not virtual in parent...
    int addTab(const QString &label);
    int addTab(const QIcon &icon, const QString &label);

signals:
    void prepareTab(int index, QScrollArea *parent);

private slots:
    void layoutTab(int);

private:
    QWidget *widget(int index) const;
    // hiding these functions.
    int addTab(QWidget *child, const QString &label);
    int addTab(QWidget *child, const QIcon &icon, const QString &label);
    int insertTab(int index, QWidget *widget, const QString &label);
    int insertTab(int index, QWidget *widget, const QIcon &icon, const QString &label);

    QMap<int, QDelayedScrollArea *> unpreparedTabs;
};

#endif
