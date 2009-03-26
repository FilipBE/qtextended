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

#ifndef GREENPHONEAUDIOPLUGIN_H
#define GREENPHONEAUDIOPLUGIN_H

#include <QAudioStatePlugin>

class GreenphoneAudioPluginPrivate;

class GreenphoneAudioPlugin : public QAudioStatePlugin
{
    Q_OBJECT
    friend class GreenphoneAudioPluginPrivate;

public:
    GreenphoneAudioPlugin(QObject *parent = 0);
    ~GreenphoneAudioPlugin();

    QList<QAudioState *> statesProvided() const;

private:
    GreenphoneAudioPluginPrivate *m_data;
};

#endif
