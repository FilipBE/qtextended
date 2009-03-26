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

#include <qtopialog.h>
#include <qsocketnotifier.h>

#include "qaudiomixer.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>


class QAudioElementPrivate : public QObject
{
    Q_OBJECT
    friend class QAudioMixerPrivate;
public:
    QAudioElementPrivate()
    {
        lvalue     = 0;
        rvalue     = 0;
        pmin       = 0;
        pmax       = 0;
        idx        = 0;
        muted      = false;
        mono       = false;
    }

    ~QAudioElementPrivate()
    {
    }

    void setValue(int lval, int rval)
    {
        unsigned int leftright;
        int mixerHandle = ::open( "/dev/mixer", O_RDWR|O_NONBLOCK );
        if ( mixerHandle >= 0 ) {
            leftright = (lval << 8) | rval;
            ioctl(mixerHandle, (SOUND_MIXER_WRITE_VOLUME+element-1), &leftright);
            close(mixerHandle);
        }
    }

    void setMute(bool val)
    {
        unsigned int leftright;

        int mixerHandle = ::open( "/dev/mixer", O_RDWR|O_NONBLOCK );
        if ( mixerHandle >= 0 ) {
            if(val) {
                leftright = 0;
                ioctl(mixerHandle, (SOUND_MIXER_WRITE_VOLUME+element-1), &leftright);
            } else {
                leftright = (lvalue << 8) | rvalue;
                ioctl(mixerHandle, (SOUND_MIXER_WRITE_VOLUME+element-1), &leftright);
            }
            close(mixerHandle);
        }
    }

    QAudioElement*   e;

    QString          name;
    qint32           idx;
    bool             muted;
    bool             mono;
    qint32           lvalue;
    qint32           rvalue;
    qint32           pmin;
    qint32           pmax;

    int              element;
};

QAudioElement::QAudioElement()
{
    d = new QAudioElementPrivate();
}

QAudioElement::~QAudioElement()
{
    delete d;
}

QString QAudioElement::getName() const
{
    return d->name;
}

qint32 QAudioElement::getIndex() const
{
    return d->idx;
}

bool QAudioElement::isRecord() const
{
    return false;
}

bool QAudioElement::isPlayback() const
{
    return true;
}

bool QAudioElement::isOption() const
{
    return false;
}

qint32 QAudioElement::getMinimum() const
{
    return d->pmin;
}

qint32 QAudioElement::getMaximum() const
{
    return d->pmax;
}

qint32 QAudioElement::getValue() const
{
    return ((d->rvalue + d->lvalue)/2);
}

void QAudioElement::setValue(qint32 val)
{
    d->rvalue = val;
    d->lvalue = val;
    d->setValue(val, val);
}

bool QAudioElement::isMono() const
{
    return d->mono;
}

QString QAudioElement::getOption() const
{
    return NULL;
}

void QAudioElement::setOption(QString /*opt*/)
{
}

bool QAudioElement::isMuted() const
{
    return d->muted;
}

void QAudioElement::setMute(bool val)
{
    d->muted = val;
    d->setMute(val);
}


class QAudioMixerPrivate : public QObject
{
friend class QAudioElementPrivate;
friend class QAudioElement;

    Q_OBJECT
public:
    QAudioMixerPrivate(QObject *parent)
    {
        unsigned int leftright;
        int left, right;

        available = true;
        mixerIdx = 0;
        makeMap();
        int mixerHandle = ::open( "/dev/mixer", O_RDWR|O_NONBLOCK );
        if ( mixerHandle >= 0 ) {
            for(int i = 0; i < SOUND_MIXER_NRDEVICES; i++) {
                if(ioctl(mixerHandle,MIXER_READ(i), &leftright) != -1) {
                    left  = (leftright & 0xff00) >> 8;
                    right = (leftright & 0x00ff);
                    addElement(i+1,map[i+1], 0);
                    setRange(i+1, 0, 100);
                    setValue(i+1, left, right);
                    mixerIdx++;
                }
            }
            close( mixerHandle );
            mixerHandle = 0;
        } else {
            qWarning( "access to audio mixer failed" );
            available = false;
        }
        connect(this, SIGNAL(audioChanged()), parent,SIGNAL(audioChanged()));
    }

    ~QAudioMixerPrivate()
    {
    }
    QList<QAudioElement*>      elements;

signals:
    void audioChanged();

private:
    void addElement(int elem, QString nam, int i);
    void setMute(int elem, int lvalue, int rvalue);
    void setRange(int elem, int min, int max);
    void setValue(int elem, int lvalue, int rvalue);
    void setMono(int elem, bool val);

    void makeMap();

    bool                       available;
    int                        mixerIdx;

    QMap<int, QString> map;

    private slots:
    void update();
};

void QAudioMixerPrivate::update()
{
    emit audioChanged();
}

void QAudioMixerPrivate::makeMap()
{
    map[ 1] = "Master Volume";
    map[ 2] = "Bass";
    map[ 3] = "Treble";
    map[ 4] = "Synth";
    map[ 5] = "PCM Volume";
    map[ 6] = "Speaker";
    map[ 7] = "Line";
    map[ 8] = "Mic";
    map[ 9] = "CD";
    map[10] = "Rec Monitor";
    map[11] = "Alt PCM Volume";
    map[12] = "Rec Level";
    map[13] = "Input Gain";
    map[14] = "Output Gain";
    map[15] = "Line1";
    map[16] = "Line2";
    map[17] = "Line3";
    map[18] = "Digital1";
    map[19] = "Digital2";
    map[20] = "Digital3";
    map[21] = "Phone In";
    map[22] = "Phone Out";
    map[23] = "TV audio in";
    map[24] = "Radio In";
    map[25] = "Monitor Volume";
}

void QAudioMixerPrivate::addElement(int elem, QString nam, int i)
{
    QAudioElement *newElement = new QAudioElement();
    newElement->d->element = elem;
    newElement->d->name.append(nam);
    newElement->d->idx = i;

    elements.append(newElement);
}

void QAudioMixerPrivate::setMute(int elem, int lvalue, int rvalue)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->muted = lvalue && rvalue;
        }
    }
}

void QAudioMixerPrivate::setValue(int elem, int lvalue, int rvalue)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->lvalue = lvalue;
            e->d->rvalue = rvalue;
            e->setValue((rvalue+lvalue)/2);
        }
    }
}

void QAudioMixerPrivate::setRange(int elem, int min, int max)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->pmin = min;
            e->d->pmax = max;
        }
    }
}

void QAudioMixerPrivate::setMono(int elem, bool val)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->mono = val;
        }
    }
}

QAudioMixer::QAudioMixer()
{
    d = new QAudioMixerPrivate(this);
}

QAudioMixer::~QAudioMixer()
{
    delete d;
}

QList<QAudioElement*> QAudioMixer::elements() const
{
    return d->elements;
}
#include "qaudiomixer_oss.moc"
