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

#include <alsa/asoundlib.h>
#include <stdlib.h>


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
        pvolume    = false;
        pswitch    = false;
        volume     = false;
        cvolume    = false;
        cswitch    = false;
        joinedSw   = false;
        joinedVol  = false;
        enumerated = false;
    }

    ~QAudioElementPrivate()
    {
    }

    void setValue(int lval, int rval)
    {
        snd_mixer_selem_channel_id_t channel;

        if(pvolume|volume) {
            for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                if(snd_mixer_selem_has_playback_channel(element,(snd_mixer_selem_channel_id_t)i) > 0) {
                    channel = (snd_mixer_selem_channel_id_t)i;
                    snd_mixer_selem_set_playback_volume(element, channel, (lval+rval)/2);
                }
            }
        }
        if(cvolume) {
            for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                if(snd_mixer_selem_has_capture_channel(element,(snd_mixer_selem_channel_id_t)i) > 0) {
                    channel = (snd_mixer_selem_channel_id_t)i;
                    snd_mixer_selem_set_capture_volume(element, channel, (lval+rval)/2);
                }
            }
        }
        if(volume) {
            for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                snd_mixer_selem_set_playback_volume(element, (snd_mixer_selem_channel_id_t)0, (lval+rval)/2);
            }
        }
    }

    void setMute(bool val)
    {
        snd_mixer_selem_channel_id_t channel;

        if(pswitch) {
            for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                channel = (snd_mixer_selem_channel_id_t)i;
                snd_mixer_selem_set_playback_switch(element, channel, !val);
            }
        }
        if(cswitch) {
            for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                channel = (snd_mixer_selem_channel_id_t)i;
                snd_mixer_selem_set_capture_switch(element, channel, !val);
            }
        }
    }

    void setEnum(QString name)
    {
        if(!enumerated) return;

        unsigned int sel;
        char itemname[40];
        for(int i=0; !(int)snd_mixer_selem_get_enum_item(element, (snd_mixer_selem_channel_id_t)i, &sel); i++) {
            snd_mixer_selem_get_enum_item_name(element,sel, sizeof(itemname)-1,itemname);
            QString itemEntry(itemname);
            if(itemEntry.compare(name)) {
                int itms = snd_mixer_selem_get_enum_items(element);
                for(int j=0; j< itms; j++) {
                    snd_mixer_selem_get_enum_item_name(element, j, sizeof(itemname)-1,itemname);
                    QString itemEntryA(itemname);
                    if(!itemEntryA.compare(name)) {
                        currentOption.clear();
                        currentOption.append(name);
                        snd_mixer_selem_set_enum_item( element, (snd_mixer_selem_channel_id_t)0, j);
                    }
                }
            }
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
    QString          currentOption;
    bool             pvolume;
    bool             pswitch;
    bool             volume;
    bool             cvolume;
    bool             cswitch;
    bool             joinedSw;
    bool             joinedVol;
    bool             enumerated;

    snd_mixer_elem_t* element;
};

/*!
  \class QAudioElement
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule
  \brief The QAudioElement class manages an element of the audio mixer.

  This class provides functions to get and set the audio mixer value of the element.

  \sa QAudioMixer
*/

/*!
   Construct a new QAudioElement object
*/
QAudioElement::QAudioElement()
{
    d = new QAudioElementPrivate();
}

/*!
  Destroy this audio mixer element.
*/
QAudioElement::~QAudioElement()
{
    delete d;
}

/*!
  Return a QString pointer to the name of the element.
*/
QString QAudioElement::getName() const
{
    return d->name;
}

/*!
  Return the index number of the element.
*/
qint32 QAudioElement::getIndex() const
{
    return d->idx;
}
/*!
  Returns true if element is a capture control.
*/
bool QAudioElement::isRecord() const
{
    return (d->cvolume);
}
/*!
  Returns true if element is a playback control.
*/
bool QAudioElement::isPlayback() const
{
    return (d->pvolume || d->volume);
}
/*!
  Returns true if element is an enum control.
*/
bool QAudioElement::isOption() const
{
    return (d->enumerated);
}
/*!
  Returns the minimum value that the control can be set too.
*/
qint32 QAudioElement::getMinimum() const
{
    return d->pmin;
}
/*!
  Returns the maximum value that the control can be set too.
*/
qint32 QAudioElement::getMaximum() const
{
    return d->pmax;
}
/*!
  Returns the value that the control element is currently set as.
*/
qint32 QAudioElement::getValue() const
{
    return ((d->rvalue + d->lvalue)/2);
}

/*!
  Sets the value of the control element to \a val.
*/
void QAudioElement::setValue(qint32 val)
{
    d->rvalue = val;
    d->lvalue = val;
    d->setValue(val, val);
}

/*!
  Returns true if mixer element is only a single channel, not stereo.
*/
bool QAudioElement::isMono() const
{
    return d->mono;
}

/*!
  Returns the currently set option for an enum type mixer element.
*/
QString QAudioElement::getOption() const
{
    return d->currentOption;
}

/*!
  Sets the mixer elements enum to value \a opt.
*/
void QAudioElement::setOption(QString opt)
{
    d->currentOption.clear();
    QTextStream(&d->currentOption) << opt;
    d->setEnum(opt);
}

/*!
  Returns true if mixer element is muted.
*/
bool QAudioElement::isMuted() const
{
    return d->muted;
}

/*!
  Sets the audio mixer element mute state to \a val.
*/
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
        available = true;
        snd_ctl_card_info_alloca(&hw_info);
        if ((snd_ctl_open (&ctl_handle, "default", 0)) < 0) {
            qWarning("QAudioMixer() Default control of Mixer open failed");
            available = false;
            return;
        }
        if ((snd_ctl_card_info (ctl_handle, hw_info)) < 0) {
            qWarning("QAudioMixer() Default control of Mixer get hwinfo failed");
            available = false;
            return;
        }
        snd_ctl_close (ctl_handle);

        if ((snd_mixer_open(&handle, 0)) < 0) {
            qWarning("QAudioMixer() Default Mixer open failed");
            available = false;
            return;
        }
        if ((handle != NULL) && (snd_mixer_attach(handle, "default") < 0)) {
            qWarning("QAudioMixer() attach to Mixer failed");
            snd_mixer_close(handle);
            handle = NULL;
            available = false;
            return;
        }
        int options = 0;
        if (snd_mixer_selem_register(handle,(snd_mixer_selem_regopt*)options,NULL) < 0)  {
            qWarning("QAudioMixer() Mixer register failed");
            snd_mixer_close(handle);
            handle = NULL;
            available = false;
            return;
        }

        snd_mixer_set_callback(handle, mixer_event);
        snd_mixer_set_callback_private(handle, this);

        if (snd_mixer_load(handle) < 0) {
            qWarning("QAudioMixer() Mixer load failed");
            snd_mixer_close(handle);
            handle = NULL;
            available = false;
            return;
        }
        int count = snd_mixer_poll_descriptors_count(handle);
        pollfd  *pfds = new pollfd[count];
        snd_mixer_poll_descriptors(handle, pfds, count);
        for(int i = 0;i<count;++i) {
            if ((pfds[i].events & POLLIN) != 0) {
                notifier = new QSocketNotifier(pfds[i].fd, QSocketNotifier::Read);
                connect(notifier,SIGNAL(activated(int)),SLOT(update()));
                break;
            }
        }
        connect(this, SIGNAL(audioChanged()), parent,SIGNAL(audioChanged()));
    }

    ~QAudioMixerPrivate()
    {
        snd_mixer_free(handle);
        snd_mixer_detach(handle,"default");
        snd_mixer_close(handle);
    }
    QList<QAudioElement*>      elements;

signals:
    void audioChanged();

private:
    void addElement(snd_mixer_elem_t *elem, QString nam, int i);
    void setMute(snd_mixer_elem_t *elem, int lvalue, int rvalue);
    void setRange(snd_mixer_elem_t *elem, int min, int max);
    void setValue(snd_mixer_elem_t *elem, int lvalue, int rvalue);
    void setMono(snd_mixer_elem_t *elem, bool val);
    void setEntry(snd_mixer_elem_t *elem, QString nam, int idx);
    void addEntry(snd_mixer_elem_t *elem, QString nam);

    static int mixer_event(snd_mixer_t *mixer, unsigned int mask,snd_mixer_elem_t *elem);
    static int melem_event(snd_mixer_elem_t *elem, unsigned int mask);

    snd_ctl_card_info_t*       hw_info;
    snd_ctl_t*                 ctl_handle;
    snd_mixer_selem_id_t*      sid;
    snd_mixer_t*               handle;
    QSocketNotifier            *notifier;

    bool                       available;

private slots:
    void update();
};

int QAudioMixerPrivate::mixer_event(snd_mixer_t *mixer, unsigned int mask,snd_mixer_elem_t *elem)
{
    if (mask & SND_CTL_EVENT_MASK_ADD) {
        snd_mixer_selem_id_t *ssid;
        snd_mixer_selem_id_alloca(&ssid);
        snd_mixer_selem_get_id(elem, ssid);

        QAudioMixerPrivate *p = (QAudioMixerPrivate*)snd_mixer_get_callback_private(mixer);
        p->addElement(elem,snd_mixer_selem_id_get_name(ssid),snd_mixer_selem_id_get_index(ssid));

        snd_mixer_elem_set_callback(elem, melem_event);
        snd_mixer_elem_set_callback_private(elem, p);
    }
    return 0;
}

int QAudioMixerPrivate::melem_event(snd_mixer_elem_t *elem, unsigned int mask)
{
    long                         tempvol=0,pmin=0,pmax=0,pvol=0;
    snd_mixer_selem_channel_id_t channel;

    snd_mixer_selem_id_t *ssid;
    snd_mixer_selem_id_alloca(&ssid);
    snd_mixer_selem_get_id(elem, ssid);

    QAudioElement   *e;
    QAudioMixerPrivate *p=(QAudioMixerPrivate*)snd_mixer_elem_get_callback_private(elem);

    if (mask & SND_CTL_EVENT_MASK_VALUE) {
        for(int i = 0; i < p->elements.size(); ++i) {
            e = p->elements.at(i);
            if(e->d->element == elem) {
                if(snd_mixer_selem_has_playback_switch(elem) > 0) {
                    e->d->pswitch = true;
                    if(snd_mixer_selem_has_playback_volume_joined(elem) > 0) e->d->joinedVol = true;
                    if(snd_mixer_selem_has_playback_switch_joined(elem) > 0) e->d->joinedSw = true;
                }
                if(snd_mixer_selem_has_capture_switch(elem)  > 0) e->d->cswitch = true;
                if(snd_mixer_selem_has_common_volume(elem)   > 0) {
                    e->d->volume  = true;
                    e->d->pvolume = false;
                    e->d->cvolume = false;
                    snd_mixer_selem_get_playback_volume_range(elem, &pmin, &pmax);
                    p->setRange(elem, (int)pmin, (int)pmax);
                    pvol = 0;
                    for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                        channel = (snd_mixer_selem_channel_id_t)i;
                        snd_mixer_selem_get_playback_volume(elem, channel, &tempvol);
                        if(tempvol > pvol) pvol = tempvol;
                    }
                    p->setValue(elem, (int)pvol, (int)pvol);
                } else {
                    if(snd_mixer_selem_has_playback_volume(elem) > 0) {
                        e->d->pvolume = true;
                        snd_mixer_selem_get_playback_volume_range(elem, &pmin, &pmax);
                        p->setRange(elem, (int)pmin, (int)pmax);
                        pvol = 0;
                        for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                            channel = (snd_mixer_selem_channel_id_t)i;
                            snd_mixer_selem_get_playback_volume(elem, channel, &tempvol);
                            if(tempvol > pvol) pvol = tempvol;
                        }
                        p->setValue(elem, (int)pvol, (int)pvol);
                    }
                    if(snd_mixer_selem_has_capture_volume(elem)  > 0) {
                        e->d->cvolume = true;
                        snd_mixer_selem_get_capture_volume_range(elem, &pmin, &pmax);
                        p->setRange(elem, (int)pmin, (int)pmax);
                        pvol = 0;
                        for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                            channel = (snd_mixer_selem_channel_id_t)i;
                            snd_mixer_selem_get_capture_volume(elem, channel, &tempvol);
                            if(tempvol > pvol) pvol = tempvol;
                        }
                        p->setValue(elem, (int)pvol, (int)pvol);
                    }
                }

                if(snd_mixer_selem_is_enumerated(elem) > 0) {
                    e->d->enumerated = true;
                    char itemname[40];
                    int itms = snd_mixer_selem_get_enum_items(elem);
                    unsigned int sel;
                    for(int i=0; i< itms; i++) {
                        snd_mixer_selem_get_enum_item_name(elem, i, sizeof(itemname)-1,itemname);
                        QString itemEntry(itemname);
                        p->addEntry(elem, itemEntry);
                    }
                    for(int i=0; !(int)snd_mixer_selem_get_enum_item(elem, (snd_mixer_selem_channel_id_t)i, &sel); i++) {
                        snd_mixer_selem_get_enum_item_name(elem,sel, sizeof(itemname)-1,itemname);
                        p->setEntry(elem,QString(itemname),sel);
                    }
                }

                int val = 0, sw = 0;
                snd_mixer_selem_channel_id_t channel;
                for(int i = -1; i <= (int) SND_MIXER_SCHN_LAST; i++) {
                    if(snd_mixer_selem_has_playback_channel(elem,(snd_mixer_selem_channel_id_t)i) > 0) {
                        channel = (snd_mixer_selem_channel_id_t)i;
                        snd_mixer_selem_get_playback_switch(elem, channel, &val);
                        sw = sw | val;
                        val = 0;
                    }
                    if(snd_mixer_selem_has_capture_channel(elem,(snd_mixer_selem_channel_id_t)i) > 0) {
                        channel = (snd_mixer_selem_channel_id_t)i;
                        snd_mixer_selem_get_capture_switch(elem, channel, &val);
                        sw = sw | val;
                        val = 0;
                    }
                }
                if(e->d->pswitch||e->d->cswitch)
                    p->setMute(elem,!sw,!sw);

                if(snd_mixer_selem_is_playback_mono(elem)) p->setMono(elem,true);
                else p->setMono(elem,false);
            }
        }
    }
    return 0;
}

void QAudioMixerPrivate::update()
{
    notifier->setEnabled(false);
    int err = snd_mixer_wait(handle,1000);
    err = snd_mixer_handle_events(handle);
    if(err > 0)
        notifier->setEnabled(true);
    emit audioChanged();
}

void QAudioMixerPrivate::addElement(snd_mixer_elem_t *elem, QString nam, int i)
{
    QAudioElement *newElement = new QAudioElement();
    newElement->d->element = elem;
    newElement->d->name.append(nam);
    newElement->d->idx = i;

    snd_mixer_selem_id_t *ssid;
    snd_mixer_selem_id_alloca(&ssid);
    snd_mixer_selem_get_id(elem, ssid);

    elements.append(newElement);
}

void QAudioMixerPrivate::setMute(snd_mixer_elem_t *elem, int lvalue, int rvalue)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->muted = lvalue && rvalue;
        }
    }
}

void QAudioMixerPrivate::setValue(snd_mixer_elem_t *elem, int lvalue, int rvalue)
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

void QAudioMixerPrivate::setEntry(snd_mixer_elem_t *elem, QString nam, int /*idx*/)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->currentOption.clear();
            QTextStream(&e->d->currentOption)<<nam;
        }
    }
}

void QAudioMixerPrivate::addEntry(snd_mixer_elem_t *elem, QString nam)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            if(!e->enumOptions.contains(nam))
                e->enumOptions.append(nam);
        }
    }
}

void QAudioMixerPrivate::setRange(snd_mixer_elem_t *elem, int min, int max)
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

void QAudioMixerPrivate::setMono(snd_mixer_elem_t *elem, bool val)
{
    QAudioElement   *e;

    for(int i = 0; i < elements.size(); ++i) {
        e = elements.at(i);
        if(e->d->element == elem) {
            e->d->mono = val;
        }
    }
}

/*!
  \class QAudioMixer
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
    \inpublicgroup QtMediaModule
    \inpublicgroup QtBluetoothModule

  \brief The QAudioMixer class provides a way of accessing the audio systems mixer controls.

  This class provides both set and get functionality.

  A typical implementation follows:

  \code
  QAudioMixer    *mixer;

  mixer = new QAudioMixer();
  connect(mixer, SIGNAL(audioChanged()), this, SLOT(updateStatus()));
  QList<QAudioElement*> e = mixer->elements();
  current = e.at(0);
  updateStatus();
  \endcode

  audioChanged() is emitted when changes to the mixer settings are made.

  see signals audioChanged()

  \sa QAudioElement
  \ingroup multimedia
*/

/*!
  Construct a new QAudioMixer object
*/
QAudioMixer::QAudioMixer()
{
    d = new QAudioMixerPrivate(this);
}

/*!
  Destroy this audio mixer.
*/
QAudioMixer::~QAudioMixer()
{
    delete d;
}
/*!
  Return a pointer to a QList of QAudioElement pointers
*/
QList<QAudioElement*> QAudioMixer::elements() const
{
    return d->elements;
}

/*!
    \fn QAudioMixer::audioChanged()
      This signal is emitted when audio elements are changed.
*/

#include "qaudiomixer_alsa.moc"
