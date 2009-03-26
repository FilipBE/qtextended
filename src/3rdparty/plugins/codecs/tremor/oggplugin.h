#ifndef __QTOPIA_OGGPLUGIN_H
#define __QTOPIA_OGGPLUGIN_H

#include <QObject>
#include <QStringList>

#include <QMediaCodecPlugin>

#include <qtopiaglobal.h>



class OggPluginPrivate;

class OggPlugin :
    public QObject,
    public QMediaCodecPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMediaCodecPlugin)

public:
    OggPlugin();
    ~OggPlugin();

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
    OggPluginPrivate*  d;
};


#endif  // __QTOPIA_OGGPLUGIN_H
