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

#ifndef QVIDEOOUTPUTLOADER_H
#define QVIDEOOUTPUTLOADER_H

#include <QObject>
#include <qtopiaglobal.h>
#include "qvideoframe.h"


class QVideoOutputFactory;
class QAbstractVideoOutput;
class QDirectPainter;
class QScreen;

class QVideoOutputLoaderPrivate;

class QTOPIAVIDEO_EXPORT QVideoOutputLoader : public QObject
{
    Q_OBJECT

public:
    virtual ~QVideoOutputLoader();

    static QVideoOutputLoader* instance();

    void load();
    void unload();
    bool isLoaded() const;

    QAbstractVideoOutput* create( QScreen *screen,
                                  const QVideoFormatList& expectedFormats,
                                  QObject *parent );
    QList<QVideoOutputFactory*> const& videoOutputFactories();

private:
    QVideoOutputLoader( QObject *parent );
    QVideoOutputLoaderPrivate* d;
};


#endif
