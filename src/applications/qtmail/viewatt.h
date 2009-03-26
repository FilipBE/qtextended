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



#ifndef VIEWATT_H
#define VIEWATT_H

#include <qdialog.h>
#include <qlist.h>
#include <qmap.h>

class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QMailMessagePart;
class QMailMessage;

class ViewAtt : public QDialog
{
    Q_OBJECT

public:
    ViewAtt(QMailMessage *mailIn, bool _inbox, QWidget *parent = 0, Qt::WFlags f = 0);
    bool eventFilter( QObject *, QEvent * );

public slots:
    void accept();
    void reject();

private slots:
    void setInstall(QTableWidgetItem* i);

private:
    void init();
    QTableWidget *listView;
    QMailMessage* mail;
    bool inbox;
    QLabel* label;
    QList<QTableWidgetItem*> on;
    QList<QTableWidgetItem*> off;
    QMap<QTableWidgetItem*,int> attachmentMap; //(tableitem,part index)
};

#endif
