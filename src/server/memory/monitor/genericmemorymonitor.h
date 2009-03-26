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

#ifndef GENERICMEMORYMONITOR_H
#define GENERICMEMORYMONITOR_H

#include "memorymonitor.h"
#include <QVector>
#include <QDateTime>
#include <QValueSpaceObject>

class QTimer;

class GenericMemoryMonitorTask : public MemoryMonitor
{
    Q_OBJECT
  public:
    GenericMemoryMonitorTask();

    virtual MemState memoryState() const;
    virtual unsigned int timeInState() const;

  private slots:
    void memoryMonitor();

  private:
    enum VM_Model {
        VMUnknown,
        VMLinux_2_4,
        VMLinux_2_6
    };

  private:
    void computeMemoryValues();
    void computePageFaultValues();
    void computeMemoryState();
    void restartTimer();

  private:
    int                 m_low_threshhold;
    int                 m_verylow_threshhold;
    int                 m_critical_threshhold;
    int                 m_percent_threshhold;
    int                 m_memory_available;
    int                 m_percent_available;
    int                 m_samples;
    int                 m_long_interval;
    int                 m_short_interval;
    int                 m_normal_count;
    int                 m_critical_count;
    int                 m_verylow_count;
    int                 m_low_count;

    VM_Model            m_vmStatType;
    QTimer*             m_vmMonitor;
    bool                m_useLongInterval;
    short               m_count;
    long                m_previousFaults;
    QVector<int>        m_pageFaults;
    QVector<int>        m_memStates;
    MemState            m_memoryState;
    QDateTime           m_lastChanged;
    QValueSpaceObject*  m_vso;
};

#endif
