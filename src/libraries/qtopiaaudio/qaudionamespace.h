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

#ifndef QAUDIONAMESPACE_H
#define QAUDIONAMESPACE_H

#include <QFlags>

#include <qtopiaglobal.h>

class QString;
#ifndef Q_QDOC
// syncqtopia header QAudio
namespace QAudio
{
#else
class QAudio
{
public:
#endif
    enum AudioCapability { None = 0x0, InputOnly = 0x01, OutputOnly = 0x02, InputAndOutput = 0x04 };
    Q_DECLARE_FLAGS(AudioCapabilities, AudioCapability)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAudio::AudioCapabilities)

#endif
