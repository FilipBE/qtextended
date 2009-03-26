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

#ifndef QAUDIOMIXER_H
#define QAUDIOMIXER_H

#include <QObject>
#include <qtopiaglobal.h>
#include <QList>
#include <QStringList>


class QAudioMixerPrivate;
class QAudioElementPrivate;


class QTOPIAAUDIO_EXPORT QAudioElement : public QObject
{
    friend class QAudioMixerPrivate;
    Q_OBJECT
public:
    QAudioElement();
    ~QAudioElement();

    QString getName()    const;
    qint32  getIndex()   const;
    bool    isRecord()   const;
    bool    isPlayback() const;
    bool    isOption()   const;
    qint32  getMinimum() const;
    qint32  getMaximum() const;
    qint32  getValue()   const;
    void    setValue(qint32 val);
    bool    isMono()     const;
    QString getOption()  const;
    void    setOption(QString opt);
    bool    isMuted()    const;
    void    setMute(bool val);

    QStringList enumOptions;

private:
    QAudioElementPrivate *d;
};

class QTOPIAAUDIO_EXPORT QAudioMixer : public QObject
{
    Q_OBJECT
public:
    QAudioMixer();
    ~QAudioMixer();

    QList<QAudioElement*>  elements() const;

signals:
    void audioChanged();

private:
    QAudioMixerPrivate *d;
};

#endif
