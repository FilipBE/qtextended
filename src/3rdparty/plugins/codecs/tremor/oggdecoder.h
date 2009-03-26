#ifndef __QTOPIA_CRUXUS_OGGCODER_H
#define __QTOPIA_CRUXUS_OGGCODER_H


#include <qtopiamedia/media.h>
#include <QMediaDecoder>


#define OGG_BUFFER_SIZE 4096

extern "C"
{
#include "ivorbiscodec.h"
#include "ivorbisfile.h"
}

/**
     callbacks from vorbisfile
     */

extern "C" {
    extern size_t  fread_func  (void *ptr,size_t size,size_t nmemb, void *stream);
    extern int     fseek_func  (void *stream, ogg_int64_t offset, int whence);
    extern int     fclose_func (void *stream);
    extern long    ftell_func  (void *stream);
}

class QMediaDevice;

class OggDecoderPrivate;

class OggDecoder : public QMediaDecoder
{
    Q_OBJECT

public:
    OggDecoder();
    ~OggDecoder();

    QMediaDevice::Info const& dataType() const;

    bool connectToInput(QMediaDevice* input);
    void disconnectFromInput(QMediaDevice* input);

    void start();
    void stop();
    void pause();

    quint64 length();
    bool seek(qint64 ms);

    void setVolume(int volume);
    int volume();

    void setMuted(bool mute);
    bool isMuted();

signals:
    void playerStateChanged(QtopiaMedia::State);
    void positionChanged(quint32);
    void lengthChanged(quint32);
    void volumeChanged(int);
    void volumeMuted(bool);

public:
    unsigned char *input_data;
    unsigned char *output_data;
    unsigned char *output_pos;
    int           buffered;
    int           offset;
    int           input_length;
    int           output_length;
    int           output_ptr;
    bool          resync;
    bool          header;
    quint32       duration;
    quint32       current;
    quint32       update;

private:
    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

    OggDecoderPrivate* d;
};


#endif  // __QTOPIA_CRUXUS_OGGCODER_H
