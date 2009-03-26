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

//
//  W A R N I N G
//  -------------
//
// This file is part of QtUiTest and is released as a Technology Preview.
// This file and/or the complete System testing solution may change from version to
// version without notice, or even be removed.
//

#include "qmemorysampler.h"

#include <private/qelapsedtimer_p.h>

#include <QFile>
#include <QMap>
#include <QStringList>

/*!
    \internal
    \class QMemorySampler
    \inpublicgroup QtUiTestModule

    QMemorySampler samples and aggregates memory usage information on a Linux
    system.

    Samples are collected from a file, typically /proc/meminfo, and retrieved
    via the value() function.

    For instance, for a /proc/meminfo file with this contents:

    \code
      MemTotal:      2075864 kB
      MemFree:        675052 kB
      Buffers:        160960 kB
    \endcode

    If the file is sampled three times, sampleCount() will return 3,
    keys() will return ("MemTotal", "MemFree", "Buffers"), and
    value("MemTotal", i) will return the value of MemTotal on the i'th sample.
*/

struct QMemorySamplerPrivate
{
    QMemorySamplerPrivate(QString const&);

    QFile meminfoFile;

    QMap< QString, QList<int> > samples;
    QList<int> timestamps;

    QElapsedTimer timer;
};

QMemorySamplerPrivate::QMemorySamplerPrivate(QString const& meminfoPath)
    : meminfoFile(meminfoPath)
{}

/*!
    Constructs a new memory sampler, sampling from \a meminfoPath, which
    should refer to a file of the same format as /proc/meminfo in Linux
    2.4 or 2.6.

    \a parent will be set as this object's parent.
*/
QMemorySampler::QMemorySampler(QString const& meminfoPath, QObject* parent)
    : QObject(parent), d(new QMemorySamplerPrivate(meminfoPath))
{}

/*!
    Returns the number of samples that have been taken since this sampler
    was created or cleared.
*/
int QMemorySampler::sampleCount() const
{ return d->timestamps.count(); }

/*!
    Returns the keys which have been sampled from the meminfo file.

    For instance, for a /proc/meminfo file with this contents:

    \code
      MemTotal:      2075864 kB
      MemFree:        675052 kB
      Buffers:        160960 kB
    \endcode

    keys() will return ("MemTotal", "MemFree", "Buffers").

    This class assumes that the fields available in the meminfo file
    will not change over time.  Therefore, before the first call to
    sample(), this will return an empty list, and after the first
    call to sample(), the list will always be constant.
*/
QStringList QMemorySampler::keys() const
{ return d->samples.keys(); }

/*!
    Returns the value for \a key at the \a n'th sample.

    For instance, for a /proc/meminfo file with this contents:

    \code
      MemTotal:      2075864 kB
      MemFree:        675052 kB
      Buffers:        160960 kB
    \endcode

    If sampled three times, the value of "MemTotal" at the third sample
    can be retrieved using value("MemTotal", 2).
*/
int QMemorySampler::value(QString const& key, int n) const
{
    QMap<QString, QList<int> >::const_iterator val = d->samples.find(key);

    if (val == d->samples.constEnd()) return -1;

    if (n < 0 || n > val->count()) return -1;

    return val->at(n);
}

/*!
    Returns the time at which the \a n'th sample was taken, in milliseconds
    since this sampler was created or cleared.
*/
int QMemorySampler::timestamp(int n) const
{
    if (n < 0 || n > d->timestamps.count()) return -1;

    return d->timestamps.at(n);
}

/*!
    Samples the current state of the meminfo file, and returns the
    sample index.

    After calling this function, sampleCount() will be incremented,
    and the sampled values can be accessed by using the keys(), value()
    and timestamp() functions.
*/
int QMemorySampler::sample()
{
    if (!d->meminfoFile.open(QIODevice::ReadOnly)) return -1;

    QList<QByteArray> lines = d->meminfoFile.readAll().split('\n');
    int timestamp = d->timer.elapsed();

    d->meminfoFile.close();

    bool at_least_one_sample = false;

    foreach (QByteArray line, lines) {
        static QRegExp meminfoRe("^([a-zA-Z]+):\\s+([0-9]+) kB$");
        if (-1 == meminfoRe.indexIn(line)) continue;

        QString key = meminfoRe.cap(1);
        int value = meminfoRe.cap(2).toInt();

        QMap< QString, QList<int> >::iterator iter = d->samples.find(key);

        if (0 != d->timestamps.count() && iter == d->samples.end()) continue;

        if (iter == d->samples.end()) {
            iter = d->samples.insert(key, QList<int>());
        }

        iter->append(value);
        at_least_one_sample = true;
    }

    if (!at_least_one_sample) return -1;

    d->timestamps << timestamp;
    int ret = d->timestamps.count() - 1;
    emit sampled(ret);
    return ret;
}

/*!
    Clears all collected samples.
*/
void QMemorySampler::clear()
{
    d->timestamps.clear();
    d->samples.clear();
    d->timer.start();
}

/*!
    \fn QMemorySampler::sampled(int)

    Emitted whenever a successful sample occurs, with the index of the sample.

    It is possible to connect a QTimer to the sample() slot and use this signal
    to implement sampling at regular intervals.
*/

