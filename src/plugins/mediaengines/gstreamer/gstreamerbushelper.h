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

#ifndef GSTREAMERBUSHELPER_H
#define GSTREAMERBUSHELPER_H

#include <QObject>

#include <gstreamermessage.h>
#include <gst/gst.h>

namespace gstreamer
{

class BusHelperPrivate;

class BusHelper : public QObject
{
    Q_OBJECT
    friend class BusHelperPrivate;

public:
    BusHelper(GstBus* bus, QObject* parent = 0);
    ~BusHelper();

signals:
    void message(Message const& message);

private:
    BusHelperPrivate*   d;
};

}   // ns gstreamer

#endif
