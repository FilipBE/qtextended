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

#ifndef E3_NAVIPANE_H
#define E3_NAVIPANE_H

#include <QWidget>

class E3NaviPanePrivate;
class QWSWindow;

typedef QPair<WId, int> IDData;

class E3NaviPane : public QWidget
{
    Q_OBJECT
public:
    E3NaviPane(QWidget *parent=0, Qt::WFlags=0);

    enum Location {
        Beginning = 0, Middle, End, NA
    };

private:
    void setTabs(IDData id, QList<QPair<QString,QIcon> > tabs);
    void setLocationHint(IDData id, QString text, QIcon icon, Location loc = NA);

private slots:
    void activeChanged(QString,QRect,WId);
    void received(const QString &msg, const QByteArray &data);

private:
    E3NaviPanePrivate *d;
    friend class NaviPaneService;
};

#endif
