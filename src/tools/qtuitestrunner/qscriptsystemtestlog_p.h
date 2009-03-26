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

#ifndef QSCRIPTSYSTEMTESTLOG_P_H
#define QSCRIPTSYSTEMTESTLOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class QScriptSystemTestLogPrivate;

class QScriptSystemTestLog : public QObject
{
Q_OBJECT

public:
    QScriptSystemTestLog(QObject* =0);
    virtual ~QScriptSystemTestLog();

public slots:
    void addMessages(QList<QByteArray> const&);

    // QList<QByteArray> is not automatically converted by QScriptEngine;
    // this overload allows us to not have to register it
    inline void addMessages(QStringList const& list)
    {
        QList<QByteArray> bytelist;
        foreach (QString const& str, list) bytelist << str.toLocal8Bit();
        addMessages(bytelist);
    }

    void setBufferSize(int);

    void        setFormat(QByteArray const&);
    QByteArray  format() const;
    int         count() const;
    QVariantMap message(int) const;

    void clear();

private:
    QScriptSystemTestLogPrivate* d;
};

#endif

