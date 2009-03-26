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

#ifndef SMILTRANSFER_H
#define SMILTRANSFER_H

#include <QObject>
#include <qtopiaglobal.h>

class QIODevice;
class SmilTransferServerPrivate;
class SmilElement;

class QTOPIASMIL_EXPORT SmilDataSource
{
public:
    explicit SmilDataSource(const QString &t=QString());
    virtual ~SmilDataSource();

    virtual void setDevice(QIODevice *d);
    QIODevice *device() const;

    void setMimeType(const QString &t);
    const QString &mimeType() const;

private:
    QString type;
    QIODevice *dev;
};

class SmilTransferServer : public QObject
{
    Q_OBJECT
public:
    explicit SmilTransferServer(QObject *parent);
    ~SmilTransferServer();

    void requestData(SmilElement *e, const QString &src);
    void endData(SmilElement *e, const QString &src);

signals:
    void transferRequested(SmilDataSource *s, const QString &src);
    void transferCancelled(SmilDataSource *s, const QString &src);

private:
    void requestTransfer(SmilDataSource *e, const QString &src);
    void endTransfer(SmilDataSource *e, const QString &src);

private:
    QString base;
    SmilTransferServerPrivate *d;
    friend class SmilDataStore;
};

#endif
