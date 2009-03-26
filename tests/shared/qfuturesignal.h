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

#ifndef QFUTURESIGNAL_H
#define QFUTURESIGNAL_H

#include <QObject>
#include <QList>
#include <QVariant>

class QFutureSignalPrivate;
class QFutureSignalList;
class QFutureSignalListPrivate;

class QFutureSignal
{
    friend class QFutureSignalList;
public:
    QFutureSignal(QObject *sender, const QByteArray& signal);
    QFutureSignal(const QFutureSignal& other);
    ~QFutureSignal();

    QFutureSignal& operator=(const QFutureSignal& other);

    bool wait(int timeout=-1, int expectedCount=1);
    static bool wait(QObject *sender, const QByteArray& signal,
                     int timeout=-1, int expectedCount=1);

    bool expect(int timeout=-1);
    bool expect(int timeout, const QVariant& value);
    bool expect(int timeout, const QVariant& value1, const QVariant& value2);
    bool expect(int timeout, const QList<QVariant>& values);

    static bool expect(QObject *sender, const QByteArray& signal, int timeout=-1);
    static bool expect(QObject *sender, const QByteArray& signal,
                       int timeout, const QVariant& value);
    static bool expect(QObject *sender, const QByteArray& signal,
                       int timeout, const QVariant& value1,
                       const QVariant& value2);
    static bool expect(QObject *sender, const QByteArray& signal,
                       int timeout, const QList<QVariant>& values);

    QList< QList<QVariant> > results() const;
    int resultCount() const;

    void clear();

private:
    QFutureSignalPrivate *d;
};

class QFutureSignalList
{
    friend class QFutureSignalPrivate;
public:
    QFutureSignalList();
    ~QFutureSignalList();

    void addFutureSignal(const QFutureSignal& fs);
    void addFutureSignal(QObject *sender, const QByteArray& signal);

    int count() const;
    QFutureSignal *at(int index) const;

    bool waitAny(int timeout=-1, int expectedCount=1);
    bool waitAll(int timeout=-1, int expectedCount=1);

    void clear();

private:
    QFutureSignalListPrivate *d;

    void checkResults();
};

#endif
