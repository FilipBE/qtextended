#include "oggdecoder.h"
#include "oggplugin.h"


class OggPluginPrivate
{
public:
    QStringList mimeTypes;
    QStringList fileExtensions;
};



OggPlugin::OggPlugin():
    d(new OggPluginPrivate)
{
    d->mimeTypes << "audio/ogg+vorbis";
    d->fileExtensions << "ogg";
}

OggPlugin::~OggPlugin()
{
    delete d;
}

QString OggPlugin::name() const
{
    return QLatin1String("OGG decoder");
}

QString OggPlugin::comment() const
{
    return QString();
}

QStringList OggPlugin::mimeTypes() const
{
    return d->mimeTypes;
}

QStringList OggPlugin::fileExtensions()  const
{
    return d->fileExtensions;
}

double OggPlugin::version() const
{
    return 0.01;
}

bool OggPlugin::canEncode() const
{
    return false;
}

bool OggPlugin::canDecode() const
{
    return true;
}

QMediaEncoder* OggPlugin::encoder(QString const&)
{
    return 0;
}

QMediaDecoder* OggPlugin::decoder(QString const& mimeType)
{
    Q_UNUSED(mimeType);
    return new OggDecoder;
}


QTOPIA_EXPORT_PLUGIN(OggPlugin);


