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

#ifndef WAVPLUGIN_H
#define WAVPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QMediaCodecPlugin>

#include <qtopiaglobal.h>

class WavPluginPrivate;

class WavPlugin :
    public QObject,
    public QMediaCodecPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMediaCodecPlugin)

public:
    WavPlugin();
    ~WavPlugin();

    QString name() const;
    QString comment() const;
    QStringList mimeTypes() const;
    QStringList fileExtensions() const;

    double version() const;

    bool canEncode() const;
    bool canDecode() const;

    QMediaEncoder* encoder(QString const& mimeType);
    QMediaDecoder* decoder(QString const& mimeType);

private:
    WavPluginPrivate*  d;
};

#endif
