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

#include <QTimer>

#include <qtopianamespace.h>
#include <qtopialog.h>

#include <timidity.h>

#include "mididecoder.h"
#include "midiplugin.h"


class MidiPlugin::MidiPluginPrivate : public QObject
{
    Q_OBJECT

public:
    MidiPluginPrivate();
    ~MidiPluginPrivate();

    bool        initialized;
    int         active;
    QStringList mimeTypes;
    QStringList fileExtensions;
    QTimer*     cleanupTimer;

public slots:
    void init();
    void cleanup();
    void unload();
};

MidiPlugin::MidiPluginPrivate::MidiPluginPrivate():
    initialized(false),
    active(0)
{
    mimeTypes << "audio/midi" << "audio/x-midi";
    fileExtensions << "mid" << "midi";

    cleanupTimer = new QTimer(this);
    cleanupTimer->setSingleShot(true);
    cleanupTimer->setInterval(30000);       // XXX: magic number

    connect(cleanupTimer, SIGNAL(timeout()), SLOT(unload()));
}

MidiPlugin::MidiPluginPrivate::~MidiPluginPrivate()
{
}

void MidiPlugin::MidiPluginPrivate::init()
{
    if (active++ > 0)
        return;

    if (initialized) {
        cleanupTimer->stop();
        return;
    }

    // Load .cfg
    foreach (QString configPath, Qtopia::installPaths()) {
        configPath += QLatin1String("etc/timidity/timidity.cfg");

        qLog(Media) << "MidiDecoder::MidiDecoder(); searching for config -" << configPath;

        if (QFileInfo(configPath).exists()) {
            qLog(Media) << "MidiDecoder::MidiDecoder(); found timidity.cfg";

            if (mid_init(configPath.toLocal8Bit().data()) == -1)
                qLog(Media) << "MidiDecoder::MidiDecoder(); Invalid config file";

            initialized = true;
            break;
        }
    }
}

void MidiPlugin::MidiPluginPrivate::cleanup()
{
    if (--active == 0)
        cleanupTimer->start();
}

void MidiPlugin::MidiPluginPrivate::unload()
{
    mid_exit();
    initialized = false;
}


MidiPlugin::MidiPlugin():
    d(new MidiPluginPrivate)
{
}

MidiPlugin::~MidiPlugin()
{
    d->cleanup();

    delete d;
}


QString MidiPlugin::name() const
{
    return QLatin1String("Midi decoder");
}

QString MidiPlugin::comment() const
{
    return QString();
}

QStringList MidiPlugin::mimeTypes() const
{
    return d->mimeTypes;
}

QStringList MidiPlugin::fileExtensions()  const
{
    return d->fileExtensions;
}

double MidiPlugin::version() const
{
    return 0.01;
}

bool MidiPlugin::canEncode() const
{
    return false;
}

bool MidiPlugin::canDecode() const
{
    return true;
}


QMediaEncoder* MidiPlugin::encoder(QString const&)
{
    return 0;
}

QMediaDecoder* MidiPlugin::decoder(QString const& mimeType)
{
    d->init();

    QMediaDecoder*  decoder = new MidiDecoder;
    connect(decoder, SIGNAL(destroyed(QObject*)), d, SLOT(cleanup()));

    return decoder;

    Q_UNUSED(mimeType);
}

QTOPIA_EXPORT_PLUGIN(MidiPlugin);

#include "midiplugin.moc"

