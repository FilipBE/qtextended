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

#ifndef SIMINFO_H
#define SIMINFO_H

#include <QWidget>
#include <qtopiaphone/qsmsreader.h>
#include <qtopiaphone/qphonebook.h>

class GraphData;
class Graph;
class GraphLegend;
class QLabel;
class QSimInfo;

class SimInfo : public QWidget
{
    Q_OBJECT
public:
    SimInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~SimInfo();

private slots:
    void updateData();
    void limits( const QString& store, const QPhoneBookLimits& value );
    void init();

private:
    QSMSReader *sms;
    QPhoneBook *pb;
    GraphData *smsData;
    QLabel *header;
    Graph *smsGraph;
    GraphLegend *smsLegend;
    GraphData *pbData;
    Graph *pbGraph;
    GraphLegend *pbLegend;
    QSimInfo *simInf;
    int pbused;
    int pbtotal;
};

#endif
