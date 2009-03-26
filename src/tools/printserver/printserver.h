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

#ifndef PRINTSERVER_H
#define PRINTSERVER_H

#include <qtopiaabstractservice.h>
#include <QVariant>
#include <QList>
#include <QPrinter>

class QtopiaApplication;
class QtopiaPrinterInterface;
class QPluginManager;
class PrintServerPrivate;
class QListWidgetItem;

enum PrintJobType { PPK, File, Html };

typedef struct PrintJobInfo {
    PrintJobType type;
    QVariant properties;
};

class PrintServer : public QObject
{
    Q_OBJECT
    friend class PrintService;

public:
    PrintServer(QObject *parent);
    ~PrintServer();

protected:
    void enqueuePPKPrintJob(const QVariant &properties);
    void enqueueFilePrintJob(const QString &fileName);
    void enqueueFilePrintJob(const QString &fileName, const QString &mimeType);
    void enqueueHtmlPrintJob(const QString &html);

private slots:
    void receive(const QString &, const QByteArray &);
    void pluginSelected(QListWidgetItem *);
    void dispatchPrintJob(QtopiaPrinterInterface *);
    void done( bool );
    void cancelJob();

private:
    void selectPrinterPlugin();

private:
    PrintServerPrivate *d;
};

class PrintService : public QtopiaAbstractService
{
    Q_OBJECT
    friend class PrintServer;

private:
    PrintService( PrintServer *parent )
        : QtopiaAbstractService( "Print", parent )
        { this->parent = parent; publishAll(); }
public:
    ~PrintService();

public slots:
    void print(QString);
    void print(QString,QString);
    void printHtml(QString);

private:
    PrintServer *parent;
};

#endif
