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
#ifndef QTOPIADOCUMENTSERVER_H
#define QTOPIADOCUMENTSERVER_H
#include <QObject>
#include <qtopiaglobal.h>

class QtopiaDocumentServerPrivate;

class QTOPIA_EXPORT QtopiaDocumentServer : public QObject
{
    Q_OBJECT
public:
    QtopiaDocumentServer( QObject *parent = 0 );

    virtual ~QtopiaDocumentServer();

    void shutdown();

signals:
    void shutdownComplete();

private slots:
    void threadFinished();

private:
    QtopiaDocumentServerPrivate *d;
};

#endif
