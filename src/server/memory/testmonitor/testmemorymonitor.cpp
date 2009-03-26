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

#include "testmemorymonitor.h"
#include "qtopiaserverapplication.h"
#include <QValueSpaceObject>

/*!
  \class TestMemoryMonitor
    \inpublicgroup QtDevToolsModule
  \ingroup QtopiaServer::Task
  \ingroup QtopiaServer::Memory
  \brief The TestMemoryMonitor class provides an instrumented implementation of MemoryMonitor for testing.

  The TestMemoryMonitorTask provides a Qt Extended Server Task.  Qt Extended Server 
  Tasks are documented in full in the QtopiaServerApplication class 
  documentation.

  \table
  \row \o Task Name \o TestMemoryMonitory
  \row \o Interfaces \o MemoryMonitor
  \row \o Services \o None
  \endtable

  The TestMemoryMonitor allows low memory conditions on the device to be 
  simulated.  The TestMemoryMonitor class exposes the following value space 
  items.

  \table
  \header \o Item \o Permissions \o Description
  \row \o \c {/ServerTasks/TestMemoryMonitor/MemoryLevel} \o rw \o Current memory level.  Valid values are "Unknown", "Critical", "VeryLow", "Low" and "Normal".
  \row \o \c {/ServerTasks/TestMemoryMonitor/ChangedTime} \o ro \o The date and time at which the memory level last changed.
  \endtable

  By setting the \c {MemoryLevel} items, test scripts can simulate changes in
  the system's memory pressure.  Only the \c {MemoryLevel} item is writable.
  The \c {ChangedTime} item will be updated accordingly each time the memory
  level changes.

  To ensure that the TestMemoryMonitor task is selected as the MemoryMonitor
  provider, testers should ensure that all other MemoryMonitor providers are
  either disabled, or that the TestMemoryMonitor is manually started at an
  appropriately early time.

  The TestMemoryMonitor task will fail if it is started before the value space
  is initialized.
  
  This class is part of the Qt Extended server and cannot be used by other Qt Extended applications.
 */

/*! \internal */
TestMemoryMonitor::TestMemoryMonitor()
    : m_vso(0),
      m_memstate(MemNormal),
      m_lastDateTime(QDateTime::currentDateTime())
{
    QByteArray vsoPath = QtopiaServerApplication::taskValueSpaceObject("TestMemoryMonitor");
    Q_ASSERT(!vsoPath.isEmpty());

    m_vso = new QValueSpaceObject(vsoPath, this);
    QObject::connect(m_vso,
                     SIGNAL(itemSetValue(QByteArray,QVariant)),
                     this,
                     SLOT(setValue(QByteArray,QVariant)));

    refresh();
}

/*! \internal */
TestMemoryMonitor::MemState TestMemoryMonitor::memoryState() const
{
    return m_memstate;
}

/*! \internal */
unsigned int TestMemoryMonitor::timeInState() const
{
    return m_lastDateTime.secsTo(QDateTime::currentDateTime());
}

/*! \internal */
void TestMemoryMonitor::setValue(const QByteArray &name, const QVariant &value)
{
    if ("/MemoryLevel" == name) {
        QByteArray state = value.toByteArray();
        MemState newState = m_memstate;
        if ("Unknown" == state)
	    newState = MemUnknown;
        else if ("Critical" == state)
	    newState = MemCritical;
        else if ("VeryLow" == state)
	    newState = MemVeryLow;
        else if ("Low" == state)
	    newState = MemLow;
        else if ("Normal" == state)
	    newState = MemNormal;

        if (newState != m_memstate) {
            m_memstate = newState;
            m_lastDateTime = QDateTime::currentDateTime();
            refresh();
            emit memoryStateChanged(m_memstate);
        }
    }
}

/*! \internal */
void TestMemoryMonitor::refresh()
{
    QByteArray state;
    switch(m_memstate) {
        case MemUnknown:
	    state = "Unknown";
	    break;
        case MemCritical:
	    state = "Critical";
	    break;
        case MemVeryLow:
	    state = "VeryLow";
	    break;
        case MemLow:
	    state = "Low";
	    break;
        case MemNormal:
	    state = "Normal";
	    break;
    };

    m_vso->setAttribute("MemoryLevel", state);
    m_vso->setAttribute("ChangedTime", m_lastDateTime);
}

QTOPIA_TASK(TestMemoryMonitor, TestMemoryMonitor);
QTOPIA_TASK_PROVIDES(TestMemoryMonitor, MemoryMonitor);
