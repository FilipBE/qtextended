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

#ifndef MODEMINFO_H
#define MODEMINFO_H

#include <QWidget>
#include <QString>

class QTextBrowser;
class ModemInfo : public QWidget
{
    Q_OBJECT
public:
    ModemInfo( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~ModemInfo();

    bool eventFilter( QObject* watched, QEvent *event );

private slots:
    void configValue( const QString& name, const QString& value );
    void init();

private:
    QTextBrowser* infoDisplay;
    QString manufacturer;
    QString model;
    QString revision;
    QString serial;
    QString extraVersion;

    QString format();
};

#endif
