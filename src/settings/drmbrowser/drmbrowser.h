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

#ifndef DRMBROWSER_H
#define DRMBROWSER_H

#include <QWidget>
#include <QMimeType>

class DrmBrowser : public QWidget
{
    Q_OBJECT
public:
    DrmBrowser( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~DrmBrowser();

public slots:
    void setDocument( const QString &fileName );

    void initView();

private:
    bool execService( const QString &fileName, const QMimeType &type, const QString &hint );
};

#endif
