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

#include "systemsuspend.h"

#include <QUsbManager>
#include <QUsbGadget>

class UsbSuspend : public SystemSuspendHandler
{
    public:
        UsbSuspend();
        ~UsbSuspend();

        virtual bool canSuspend() const;
        virtual bool suspend();
        virtual bool wake();

    private:
        QUsbManager *m_manager;
        QList<QUsbGadget *> m_gadgets;
};

QTOPIA_TASK(UsbSuspend, UsbSuspend)
QTOPIA_TASK_PROVIDES(UsbSuspend, SystemSuspendHandler)

UsbSuspend::UsbSuspend()
{
    m_manager = new QUsbManager(this);
    connect(m_manager, SIGNAL(deactivateCompleted()), this, SIGNAL(operationCompleted()));
    connect(m_manager, SIGNAL(deactivateAborted()), this, SIGNAL(operationCompleted()));
}

UsbSuspend::~UsbSuspend()
{
}

bool UsbSuspend::canSuspend() const
{
    return true;
}

bool UsbSuspend::suspend()
{
    m_gadgets = m_manager->activeGadgets();

    if (!m_gadgets.isEmpty()) {
        m_manager->deactivateGadgets();

        return false;
    } else {
        return true;
    }
}

bool UsbSuspend::wake()
{
    if (!m_gadgets.isEmpty()) {
        foreach (QUsbGadget *gadget, m_gadgets) {
            gadget->activate();

            delete gadget;
        }
    }

    return true;
}

