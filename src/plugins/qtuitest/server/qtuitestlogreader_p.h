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

#ifndef QTUITESTLOGREADER_P_H
#define QTUITESTLOGREADER_P_H

#include <QObject>

class QtUiTestLogReaderPrivate;

class QtUiTestLogReader : public QObject
{
Q_OBJECT
public:
    QtUiTestLogReader(QObject* =0);
    virtual ~QtUiTestLogReader();

    QString errorString() const;

    void start(QStringList const&);

    bool isActive() const;

signals:
    void error(QString const&);
    void log(QStringList const&);
    void finished();

private:
    QtUiTestLogReaderPrivate* d;
    friend class QtUiTestLogReaderPrivate;

    Q_PRIVATE_SLOT(d, void _q_onReadyRead());
    Q_PRIVATE_SLOT(d, void _q_onFinished());
    Q_PRIVATE_SLOT(d, void _q_onError());
};

#endif

