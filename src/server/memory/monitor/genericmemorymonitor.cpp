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

#include "genericmemorymonitor.h"
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QString>
#include <sys/types.h>
#include <unistd.h>
#include <QValueSpaceItem>
#include <QtopiaIpcEnvelope>

static const int MIN_MEM_LIMIT = 10000;

/*!
  \class GenericMemoryMonitorTask
    \inpublicgroup QtBaseModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::Memory
  \brief The GenericMemoryMonitorTask class implements a simple page-fault
         driven memory monitor task.

  The GenericMemoryMonitorTask provides a Qt Extended Server Task.  Qt Extended Server
  Tasks are documented in full in the QtopiaServerApplication class
  documentation.

  \table
  \row \o Task Name \o GenericMemoryMonitory
  \row \o Interfaces \o MemoryMonitor
  \row \o Services \o None
  \endtable

  The GenericMemoryMonitorTask uses a simple page-fault rate driven algorithm to
  approximate the memory "pressure" on the device.

  The GenericMemoryMonitorTask task exports the following informational value
  space items.

  \table
  \header \o Item \o Description
  \row \o \c {/ServerTasks/GenericMemoryMonitorTask/MemoryLevel} \o Set to the current memory level.  Either "Unknown", "Critical", "VeryLow", "Low" or "Normal".
  \row \o \c {/ServerTasks/GenericMemoryMonitorTask/ChangedTime} \o The date and time at which the memory level last changed.
  \endtable

  As polling is used to sample memory usage information from the /c {/proc}
  filesystem, the GenericMemoryMonitorTask should generally be replaced by
  system integrators with a more efficient, system specific mechanism.

  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*!
  \internal
 */
GenericMemoryMonitorTask::GenericMemoryMonitorTask()
    : m_low_threshhold(60),
      m_verylow_threshhold(120),
      m_critical_threshhold(250),
      m_percent_threshhold(0),
      m_memory_available(-1),
      m_percent_available(-1),
      m_samples(5),
      m_long_interval(10000),
      m_short_interval(1000),
      m_normal_count(0),
      m_critical_count(0),
      m_verylow_count(0),
      m_low_count(0),
      m_vmStatType(VMUnknown),
      m_useLongInterval(true),
      m_count(0),
      m_previousFaults(0),
      m_memoryState(MemUnknown),
      m_vso(0)
{
    qLog(OOM) << "GenericMemoryMonitorTask(): reading oom.config";
    QSettings cfg("Trolltech","oom");
    cfg.beginGroup("values");
    m_low_threshhold = cfg.value(QString("low"),60).toInt();
    qLog(OOM) << "  low threshhold = " << m_low_threshhold;
    m_verylow_threshhold = cfg.value(QString("verylow"),120).toInt();
    qLog(OOM) << "  verylow threshhold = " << m_verylow_threshhold;
    m_critical_threshhold = cfg.value(QString("critical"),250).toInt();
    qLog(OOM) << "  critical threshhold = " << m_critical_threshhold;
    m_samples = cfg.value(QString("samples"),5).toInt();
    qLog(OOM) << "  samples = " << m_samples;
    m_percent_threshhold = cfg.value(QString("percent"),20).toInt();
    qLog(OOM) << "  percent = " << m_percent_threshhold;
    m_long_interval = cfg.value(QString("long"),10000).toInt();
    qLog(OOM) << "  long = " << m_long_interval;
    m_short_interval = cfg.value(QString("short"),1000).toInt();
    qLog(OOM) << "  short = " << m_short_interval;
    cfg.endGroup();

    m_pageFaults.fill(0,m_samples);
    m_memStates.fill((int)MemUnknown,m_samples);

    QFile vmstat("/proc/vmstat"); // kernel 2.6+
    if (vmstat.exists() && vmstat.open(QIODevice::ReadOnly)) {
        qLog(QtopiaServer) << "Detected Linux 2.6.x kernel";
        m_vmStatType  = VMLinux_2_6;
        vmstat.close();
    }
    else {
        qLog(QtopiaServer) << "Detected Linux 2.4.x kernel";
        m_vmStatType = VMLinux_2_4;
    }

    if (m_vmStatType != VMUnknown) {
        m_vmMonitor = new QTimer(this);
        connect(m_vmMonitor,SIGNAL(timeout()),this,SLOT(memoryMonitor()));
        restartTimer();
    }
}

/*!
  Resets the timer to run the memory monitor every long
  interval or every short interval.

  If the percentage of memory available is greater than the
  percent threshhold value read from oom.conf, then run the
  memory monitor every long interval. Otherwise run it every
  short interval. The long and short interval values are
  also read from oom.conf.
 */
void
GenericMemoryMonitorTask::restartTimer()
{
    if (m_percent_available > m_percent_threshhold) {
        m_vmMonitor->start(m_long_interval);
        if (!m_useLongInterval)
            qLog(OOM) << "Using long interval:" << m_long_interval;
        m_useLongInterval = true;
    }
    else {
        m_vmMonitor->start(m_short_interval);
        if (m_useLongInterval)
            qLog(OOM) << "Using short interval:" << m_short_interval;
        m_useLongInterval = false;
    }
}

/*!
  This function reads some values in /proc/meminfo and uses
  them to estimate how much memory is available both as an
  absolute byte count and a percentage of the total.
 */
void
GenericMemoryMonitorTask::computeMemoryValues()
{
    m_memory_available = -1;
    m_percent_available = -1;

    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) {
        qLog(OOM) << "Could not open /proc/meminfo";
        return;
    }

    QTextStream meminfo(&file);
    QString line;
    quint64 total=0, memfree=0, buffers=0, cached=0;
    bool ok = false;
    static QRegExp kernel24HeaderLine("\\s+total:\\s+used:\\s+free:\\s+shared:\\s+buffers:\\s+cached:");

    line = meminfo.readLine();
    if(kernel24HeaderLine.indexIn(line) > -1)
    {
        static QRegExp kernel24DataLine("Mem:\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\d+)");
        line = meminfo.readLine();
        if(kernel24DataLine.indexIn(line) > -1)
        {
            total = kernel24DataLine.cap(1).toULongLong(&ok);
            memfree = kernel24DataLine.cap(3).toULongLong();
            buffers = kernel24DataLine.cap(5).toULongLong();
            cached = kernel24DataLine.cap(6).toULongLong();
        }
    }
    else
    {
        static QRegExp kernel26MemTotalLine("MemTotal:\\s+(\\d+)\\s+kB");
        static QRegExp kernel26MemFreeLine("MemFree:\\s+(\\d+)\\s+kB");
        static QRegExp kernel26BuffersLine("Buffers:\\s+(\\d+)\\s+kB");
        static QRegExp kernel26CachedLine("Cached:\\s+(\\d+)\\s+kB");
        while (!meminfo.atEnd()) {
            if (kernel26MemTotalLine.indexIn(line) > -1)
                total = kernel26MemTotalLine.cap(1).toULongLong(&ok);
            else if (kernel26MemFreeLine.indexIn(line) > -1)
                memfree = kernel26MemFreeLine.cap(1).toULongLong();
            else if (kernel26BuffersLine.indexIn(line) > -1)
                buffers = kernel26BuffersLine.cap(1).toULongLong();
            else if (kernel26CachedLine.indexIn(line) > -1) {
                cached = kernel26CachedLine.cap(1).toULongLong();
                break; //last entry to read
            }
            line = meminfo.readLine();
        }
    }

    if (!ok)
        qWarning() << "Meminfo cannot monitor available memory";
    m_memory_available = buffers + cached + memfree;
    m_percent_available = total ? (100 * m_memory_available / total) : 0;

    qLog(OOM) << "  MemTotal:        " << total;
    qLog(OOM) << "  MemFree:         " << memfree;
    qLog(OOM) << "  Buffers:         " << buffers;
    qLog(OOM) << "  Cached:          " << cached;
    qLog(OOM) << "  memory available:" << m_percent_available << "\%";
}

/*!
  This slot gets called every time the memory monitor timer
  runs out. It implements an algorithm for estimating the
  system's level of available memory state. The level of
  available memory can be \c Normal, \c Low, \c VeryLow, or
  \c Critical. The algorithm does not act to recover memory,
  if it is low, very low, or critical, but whenever a change
  in the level is detected a signal is emitted to notify the
  \l {LowMemoryTask} {low memory task} of the change.
 */
void
GenericMemoryMonitorTask::memoryMonitor()
{
    computeMemoryValues();
    computePageFaultValues();
    computeMemoryState();
    restartTimer();
}

/*!
  This function is called by the \l {memoryMonitor()} each
  time it runs to read the system's record of major page
  faults and perform some computations with the value. It
  keeps track of a history of page fault data.

  First, it reads the raw page fault data from the /proc
  directory. The location and format of the data depend on
  which linux version is running.

  Then it subtracts the previous page fault value from the
  current one to get the number of page faults in the last
  sample interval. There are two sample interval lengths.
  When the system appears to have plenty of memory, samples
  are taken less often than when the system appears to be
  running out of memory. We have chosen the long interval
  as the base interval, so if we are sampling with the short
  interval, the number of page faults in the interval is
  normalized to the long interval. The interval lengths are
  read from the configuration file oom.conf. The number of
  page faults for the interval is stored in an array.

  Then the memory state for the sample interval is computed.
  This is done by comparing the normalized page fault count
  to the three threshhold values for the \c critical,
  \c verylow, and \c low page fault threshholds. These were
  obtained from the configuration file oom.conf. If the
  number of page faults in the current interval is greater
  than the \c critical page fault threshhold, the memory
  state for the interval is \c critical, else if the number
  of page faults in the current interval is greater than the
  \c verylow page fault threshhold, the memory state for the
  current interval is \c verylow, else if the number of page
  faults in the current interval is greater than the \c low
  page fault threshhold, the memory state for the current
  interval is \c low. Otherwise, the memory state is normal.

  There are n sample periods in a cycle. The number n is
  also configuarbale and is read from oom.conf, where it
  is called \c samples. Once an entire cycle of samples has
  been completed, every sample period in the cycle has a
  memory state computed as described above. These are kept
  in an array that is updated for each sample.

  The function also keeps track of the total number of
  memory states of each kind that occur during a sample
  cycle. For example, if there are 10 samples in a cycle
  and we are gradually running out of memory, we might have
  seen three \c normal samples, 2 \c low samples, and 5
  \c verylow samples.
 */
void
GenericMemoryMonitorTask::computePageFaultValues()
{
    QString majfaults;
    if (m_vmStatType == VMLinux_2_6) {
        QFile vmstat("/proc/vmstat"); //kernel 2.6+
        if (vmstat.open(QIODevice::ReadOnly)) {
            QTextStream t(&vmstat);
            QString nLine;
            nLine = t.readLine();
            while (!nLine.isNull()) {
                //only interested in major page faults
                if (nLine.contains("pgmajfault"))  {
                    majfaults = nLine.mid(nLine.indexOf(' '));
                    break;
                }
                nLine = t.readLine();
            }
            vmstat.close();
        }

    }
    else if (m_vmStatType == VMLinux_2_4) {
        /*
          XXX - Can this be right? !!!!
         */
        QFile vmstat("/proc/"+QString::number(::getpid())+"/stat");
        if (vmstat.open(QIODevice::ReadOnly)) {
            QTextStream t(&vmstat);
            for (int i=0; i<12; i++)
                t >> majfaults;
            vmstat.close();
        }
    }

    bool ok;
    long current = majfaults.toInt(&ok);
    long difference = current - m_previousFaults;

    qLog(OOM) << "  previous faults: " << m_previousFaults;
    qLog(OOM) << "  current faults:  " << current;
    qLog(OOM) << "  new faults:      " << difference;

    /*
      First time through only.
     */
    if (!m_previousFaults) {
        m_previousFaults = current;
        return;
    }

    /*
      Corrupted data.
     */
    if (!ok)
        return;

    m_previousFaults = current;

    /*
      If we are running the monitor on the short interval,
      normalize the number of page faults so that all the
      values are scaled for the long interval.
     */
    if (!m_useLongInterval)
        difference = difference * m_long_interval / m_short_interval;

    /*
      Determine the memory state for this sample interval.
     */
    MemState nextState = MemNormal;
    if (difference > m_critical_threshhold)
        nextState = MemCritical;
    else if (difference > m_verylow_threshhold)
        nextState = MemVeryLow;
    else if (difference > m_low_threshhold)
        nextState = MemLow;

    /*
      If the previous memory state for this sample interval
      was different, adjust the totals for the number of
      sample intervals of the previous memory state and the
      current memory state. These are total number of samples
      of each memory state in the current sample cycle.
     */
    if (nextState != m_memStates[m_count]) {
        switch (m_memStates[m_count]) {
          case MemCritical:
              --m_critical_count;
              break;
          case MemVeryLow:
              --m_verylow_count;
              break;
          case MemLow:
              --m_low_count;
              break;
          case MemNormal:
              --m_normal_count;
              break;
          case MemUnknown:
              break;
        };
        switch (nextState) {
          case MemCritical:
              ++m_critical_count;
              break;
          case MemVeryLow:
              ++m_verylow_count;
              break;
          case MemLow:
              ++m_low_count;
              break;
          case MemNormal:
              ++m_normal_count;
              break;
          case MemUnknown:
              break;
        };
        m_memStates[m_count] = nextState;
    }

    /*
      Store the number of major page faults that occurred in
      this sample interval.
     */
    m_pageFaults[m_count] = difference;
    m_count = (++m_count) % m_samples;
}

/*!
  Evaluate the current page fault data and set the memory
  state accordingly. If the memory state changes, report the
  new memory state by emitting the memoryStateChanged() signal.
  The \l {LowMemoryTask} receives this signal and handles it,
  if the new state requires some action be taken to recover
  memory.
 */
void
GenericMemoryMonitorTask::computeMemoryState()
{
    int sum = 0;
    for (int i=0; i < m_samples; i++)
        sum += m_pageFaults[i];

    int avg = sum/m_samples;

    /*
      The threshhold values used here are read from oom.conf
      They are untested.
     */

    qLog(OOM) << "average faults:" << avg;
    MemState newState = MemNormal;
    if (avg > m_critical_threshhold) // (2.6,250) (2.4,50)
        newState = MemCritical;
    else if (avg > m_verylow_threshhold) // (2.6,120) (2.4,25)
        newState = MemVeryLow;
    else if (avg > m_low_threshhold) // (2.6,60) (2.4,20)
        newState = MemLow;

    if (newState == m_memoryState)
        return;

    m_memoryState = newState;
    m_lastChanged = QDateTime::currentDateTime();

    if (!m_vso) {
        QByteArray t = "GenericMemoryMonitorTask";
        QByteArray p = QtopiaServerApplication::taskValueSpaceObject(t);
        if (!p.isNull())
            m_vso = new QValueSpaceObject(p,this);
    }

    QByteArray stateName = "Unknown";
    if (m_vso) {
        m_vso->setAttribute("ChangedTime",m_lastChanged);
        switch (m_memoryState) {
            case MemUnknown: stateName = "Unknown"; break;
            case MemCritical: stateName = "Critical"; break;
            case MemVeryLow: stateName = "VeryLow"; break;
            case MemLow: stateName = "Low"; break;
            case MemNormal: stateName = "Normal"; break;
        };
        m_vso->setAttribute("MemoryLevel",stateName);
    }
    emit memoryStateChanged(m_memoryState);
}

/*!
  Returns the current memory state, ie Critical, Very Low,
  Low, or Normal.
 */
MemoryMonitor::MemState
GenericMemoryMonitorTask::memoryState() const
{
    return m_memoryState;
}

/*!
  Return the time in seconds since the last memory state change.
 */
unsigned int GenericMemoryMonitorTask::timeInState() const
{
    return m_lastChanged.secsTo(QDateTime::currentDateTime());
}

QTOPIA_TASK(GenericMemoryMonitor, GenericMemoryMonitorTask);
QTOPIA_TASK_PROVIDES(GenericMemoryMonitor, MemoryMonitor);
