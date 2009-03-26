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

#ifndef CONTENTSERVER_H
#define CONTENTSERVER_H

#include <QThread>
#include <QStringList>
#include <QSet>
#include <QTimer>

#include <qcontent.h>
#include <qcontentset.h>
#include <qtopiaipcadaptor.h>
#include "qtopiaserverapplication.h"
#include <QtopiaDocumentServer>
#include <QThread>
#include <QMutex>

class ServerInterface;
class AppLoaderPrivate;
class QValueSpaceObject;

class ContentServer : public QThread
{
    Q_OBJECT
public:
    ContentServer( QObject *parent = 0 );
    ~ContentServer();

    virtual void run();

signals:
    void scan(const QString &path, int priority);

public slots:
    void scanAll();

private slots:
    void scanning(bool scanning);

private:
    QtopiaIpcAdaptor *requestQueue;
    QValueSpaceObject *scannerVSObject;
};

// declare ContentServerTask
class ContentServerTask : public SystemShutdownHandler
{
Q_OBJECT
public:
    ContentServerTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

private:
    void doShutdown();

    ContentServer m_server;
};

class DocumentServerTask : public SystemShutdownHandler
{
    Q_OBJECT
public:
    DocumentServerTask();

    virtual bool systemRestart();
    virtual bool systemShutdown();

private:
    QtopiaDocumentServer m_server;
};

#endif
