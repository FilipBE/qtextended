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

#include <QMediaContent>

#include "medianode.h"
#include "mediaobject.h"
#include "audiooutput.h"
#include "videowidget.h"

#include "backend.h"


Q_EXPORT_PLUGIN2(phonon_qtopiamedia, Phonon::qtopiamedia::Backend);


namespace Phonon
{

namespace qtopiamedia
{

class BackendPrivate
{
public:
};

/*!
    \class Phonon::qtopiamedia::Backend
    \internal
*/

Backend::Backend(QObject* parent, const QVariantList&):
    QObject(parent),
    d(new BackendPrivate)
{
    setProperty("identifier",     QLatin1String("phonon_qtopiamedia"));
    setProperty("backendName",    QLatin1String("QtopiaMedia"));
    setProperty("backendComment", QLatin1String("QtopiaMedia plugin for Phonon"));
    setProperty("backendVersion", QLatin1String("0.1"));
    setProperty("backendWebsite", QLatin1String("http://www.trolltech.com/"));
}

Backend::~Backend()
{
    delete d;
}

QObject* Backend::createObject(Class c, QObject* parent, const QList<QVariant>& args)
{
    Q_UNUSED(args);

    switch (c) {
    case Phonon::BackendInterface::MediaObjectClass:
        return new MediaObject(this, parent);

    case Phonon::BackendInterface::AudioOutputClass:
        return new AudioOutput(this, parent);

    case Phonon::BackendInterface::EffectClass:
        qWarning() << "QtopiaMedia::Backend::createObject(): Effects not supported";
        break;

    case Phonon::BackendInterface::AudioDataOutputClass:
        qWarning() << "QtopiaMedia::Backend::createObject(): AudioDataOutput notsupported";
        break;

    case Phonon::BackendInterface::VideoDataOutputClass:
        qWarning() << "QtopiaMedia::Backend::createObject(): VideoDataOutput not supported";
        break;

    case Phonon::BackendInterface::VideoWidgetClass:
        return new VideoWidget(this, qobject_cast<QWidget*>(parent));

    case Phonon::BackendInterface::VolumeFaderEffectClass:
        qWarning() << "QtopiaMedia::Backend::createObject(): VolumeFaderEffect not supported";
        break;

    case Phonon::BackendInterface::VisualizationClass:  //Fall through
        qWarning() << "QtopiaMedia::Backend::createObject(): Visualization not supported";
        break;
    }

    return 0;
}

QList<int> Backend::objectDescriptionIndexes(Phonon::ObjectDescriptionType type) const
{
    Q_UNUSED(type);
    return QList<int>();
}

QHash<QByteArray, QVariant> Backend::objectDescriptionProperties(Phonon::ObjectDescriptionType type, int index) const
{
    Q_UNUSED(type);
    Q_UNUSED(index);

    return QHash<QByteArray, QVariant>();
}

bool Backend::startConnectionChange(QSet<QObject *>)
{
    return true;
}

bool Backend::connectNodes(QObject* source, QObject* sink)
{
    return qobject_cast<MediaNode*>(source)->connectNode(qobject_cast<MediaNode*>(sink));
}

bool Backend::disconnectNodes(QObject* source, QObject* sink)
{
    return qobject_cast<MediaNode*>(source)->disconnectNode(qobject_cast<MediaNode*>(sink));
}

bool Backend::endConnectionChange(QSet<QObject *>)
{
    return true;
}

QStringList Backend::availableMimeTypes() const
{
    return QMediaContent::supportedMimeTypes();
}

}   // ns qtopiamedia

}   // ns Phonon

