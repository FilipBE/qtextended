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

#ifndef QVIDEOOUTPUTFACTORY_H
#define QVIDEOOUTPUTFACTORY_H

#include <QObject>
#include <qtopiaglobal.h>
#include "qvideoframe.h"


class QAbstractVideoOutput;
class QScreen;

class QTOPIAVIDEO_EXPORT QVideoOutputFactory
{
public:
    QVideoOutputFactory();
    virtual ~QVideoOutputFactory();

    virtual QVideoFormatList supportedFormats() = 0;
    virtual QVideoFormatList preferredFormats() = 0;

    virtual QAbstractVideoOutput* create( QScreen *screen, QObject *parent = 0 ) = 0;
};

Q_DECLARE_INTERFACE(QVideoOutputFactory, "com.trolltech.qtopia.QVideoOutputFactory/1.0");

#endif

