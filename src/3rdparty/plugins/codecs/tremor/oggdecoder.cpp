#include <QTimer>
#include <QFileInfo>

#include <qendian.h>
#include <qtopianamespace.h>
#include <qtopialog.h>

#include "oggdecoder.h"

#include <errno.h>
#include <stdlib.h>


size_t fread_func(void *ptr, size_t size, size_t nmemb, void *stream) {
    QMediaDevice *p = (QMediaDevice*)stream;
    return p->read((char*)ptr, size*nmemb);
}

int fseek_func(void *stream, ogg_int64_t offset, int whence) {
    QMediaDevice *p = (QMediaDevice*)stream;

    bool ret;

    if (whence==SEEK_SET) {
        //qWarning("fseek() SET %ld pos=%ld",(long)offset,(long)p->pos());
        ret = p->seek(offset);
        if(!ret) return -1;
        return 0;
    }
    if (whence==SEEK_CUR) {
        //qWarning("fseek() CUR %ld pos=%ld",(long)(p->pos()+offset),(long)p->pos());
        ret = p->seek(p->pos()+offset);
        if(!ret) return -1;
        return 0;
    }
    if (whence==SEEK_END) {
        //qWarning("fseek() END %ld pos=%ld",(long)p->dataType().dataSize,(long)p->pos());
        if(p->dataType().dataSize <= 0) return -1;
        ret = p->seek(p->dataType().dataSize);
        if(!ret) {
            //qWarning("FAILED seek EOF");
            return -1;
        }
        return 0;
    }
    ret = p->seek(offset);
    if(!ret) {
        //qWarning("FAILED fseek() to %ld",(long)offset);
        return -1;
    } else {
        //qWarning("fseek() to %ld",(long)offset);
        return 0;
    }
    return -1;
}

int fclose_func (void * /*stream*/) {
    return 0;
}

long ftell_func  (void *stream) {
    QMediaDevice *p = (QMediaDevice*)stream;
    //qWarning("ftell_func() %ld",(long)p->pos());
    return (long)p->pos();
}


// {{{ OggDecoderPrivate
class OggDecoderPrivate
{
public:
    bool                initialized;
    bool                muted;
    int                 volume;
    quint32             length;
    quint32             position;
    QMediaDevice*       inputDevice;
    QtopiaMedia::State  state;
    QMediaDevice::Info  outputInfo;

    OggVorbis_File      vf;
};
// }}}

// {{{ OggDecoder
OggDecoder::OggDecoder():
    d(new OggDecoderPrivate)
{
    // init
    d->initialized = false;
    d->muted = false;
    d->volume = 50;
    d->length = 0;
    d->position = 0;
    d->state = QtopiaMedia::Stopped;
    d->outputInfo.type = QMediaDevice::Info::PCM;

    input_data = (unsigned char *)malloc(OGG_BUFFER_SIZE*8);
    output_data = (unsigned char *)malloc(OGG_BUFFER_SIZE*8);
    output_pos = output_data;
    buffered=0;
    offset=0;
    input_length=0;
    output_length=0;
    resync=false;
    header=false;
    duration=0;
    update=0;
    current=0;
}

OggDecoder::~OggDecoder()
{
    delete input_data;
    delete output_data;

    ov_clear(&d->vf);

    delete d;
}

QMediaDevice::Info const& OggDecoder::dataType() const
{
        return d->outputInfo;
}

bool OggDecoder::connectToInput(QMediaDevice* input)
{
    if (input->dataType().type != QMediaDevice::Info::Raw) {
        return false;
    }

    d->inputDevice = input;

    return true;
}

void OggDecoder::disconnectFromInput(QMediaDevice* input)
{
    Q_UNUSED(input);
    d->inputDevice = 0;
}

void OggDecoder::start()
{
    if (!d->initialized)
    {
        if (QIODevice::open(QIODevice::ReadWrite | QIODevice::Unbuffered))
        {
            vorbis_info *vi=NULL;
            ov_callbacks callbacks;

            callbacks.read_func = fread_func;
            callbacks.seek_func = fseek_func;
            callbacks.close_func = fclose_func;
            callbacks.tell_func = ftell_func;

            if(ov_open_callbacks(d->inputDevice,&d->vf,NULL,0,callbacks) < 0) {
                return;
            }

            //char **ptr1=ov_comment(&d->vf,-1)->user_comments;
            //while(*ptr1) {
            //  qWarning("%s",*ptr1);
            //  ++ptr1;
            //}

            if(d->length == 0) {
                vi=ov_info(&d->vf, -1);

                d->outputInfo.type = QMediaDevice::Info::PCM;
                d->outputInfo.frequency = (int)vi->rate;
                d->outputInfo.bitsPerSample = 16;
                d->outputInfo.channels = (int)vi->channels;
                d->outputInfo.volume = d->muted ? 0 : d->volume;

                //qWarning("ov_raw_total() %ld",(long)ov_raw_total(&d->vf,-1));
                //qWarning("ov_pcm_total() %ld",(long)ov_pcm_total(&d->vf,-1));
                //qWarning("ov_time_total() %ld",(long)ov_time_total(&d->vf,-1));

                d->length = ov_time_total(&d->vf,-1);
                duration = d->length;
                emit lengthChanged(d->length);
            }

            d->initialized = true;
        }
    }

    if (d->initialized)
    {
        if (d->state == QtopiaMedia::Stopped)
            seek(0);

        d->state = QtopiaMedia::Playing;

        emit readyRead();
        emit playerStateChanged(d->state);
    }
}

void OggDecoder::stop()
{
    emit playerStateChanged(d->state = QtopiaMedia::Stopped);
    seek(0);
}

void OggDecoder::pause()
{
    emit playerStateChanged(d->state = QtopiaMedia::Paused);
}

quint64 OggDecoder::length()
{
    return d->length;
}

bool OggDecoder::seek(qint64 ms)
{
    qint64 rawPos = (d->inputDevice->dataType().dataSize/d->length)*ms;
    //qWarning("seek to %d, %d ms", (int) rawPos, (int)ms);
    int ret = ov_time_seek(&d->vf,(ogg_int64_t) ms);
    if(ret < 0) return false;

    d->vf.offset=(ogg_int64_t)rawPos;

    update=0;
    current=ms;
    input_length=0;
    output_length=0;
    output_pos=output_data;

    emit positionChanged(d->position = ms);

    return true;
}

void OggDecoder::setVolume(int volume)
{
    d->volume = qMin(qMax(volume, 0), 100);

    if (!d->muted)
        d->outputInfo.volume = d->volume;

    emit volumeChanged(d->volume);
}

int OggDecoder::volume()
{
    return d->volume;
}

void OggDecoder::setMuted(bool mute)
{
    d->outputInfo.volume = mute ? 0 : d->volume;

    emit volumeMuted(d->muted = mute);
}

bool OggDecoder::isMuted()
{
    return d->muted;
}

//private:
qint64 OggDecoder::readData(char *data, qint64 maxlen)
{
    if(d->state == QtopiaMedia::Playing) {
        if((int)update >= (int)(d->outputInfo.frequency*d->outputInfo.channels)) {
            update=0;
            current=current+1000; // 1 second update
            duration=d->length-current;
            if((int)duration < 0) duration=0;
            d->position = current;
            emit positionChanged(d->position);
        }
        //If we have enough decoded data just output it and return
        if(output_length >= (int)maxlen) {
            memcpy(data,output_pos,(int)maxlen);
            output_pos=output_pos+(int)maxlen;
            output_length=output_length-(int)maxlen;
            update=update+maxlen/2;
            return maxlen;
        } else if(output_length > 0) {
            qint32 l=output_length;
            memcpy(data,output_pos,output_length);
            update=update+output_length/2;
            output_length=0;
            return l;
        }
        output_pos = output_data;

        qint64 len=0;
        int    current_section;
        while(output_length <(int)maxlen) {
            len=(qint64)ov_read(&d->vf,output_data+output_length,
                    OGG_BUFFER_SIZE,&current_section);
            if(len == 0) {
                d->state = QtopiaMedia::Stopped;
                emit playerStateChanged(d->state);
                break;
            }
            output_length=output_length+len;
        }

        if(output_length >= (int)maxlen) {
            output_pos = output_data+(int)maxlen;
            memcpy(data,output_data,(int)maxlen);
            output_length=output_length-(int)maxlen;
            update=update+maxlen/2;
            return maxlen;
        } else {
            output_pos = output_data;
            memcpy(data,output_data,(int)len);
            output_length=0;
            update=update+len/2;
            return len;
        }
    }
    return 0;
}

qint64 OggDecoder::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);

    return 0;
}

// }}}


