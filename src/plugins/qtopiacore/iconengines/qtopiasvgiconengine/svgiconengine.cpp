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
#include "svgiconengine.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qsvgrenderer.h>
#include <qpixmapcache.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qstyleoption.h>
#include <qfileinfo.h>
#include <qtopialog.h>
#include <QGlobalPixmapCache>
#include <qiconengineplugin.h>
#include <qstringlist.h>
#include <qiodevice.h>

class QtopiaSvgIconPlugin : public QIconEnginePluginV2
{
public:
    QStringList keys() const;
    QIconEngineV2 *create(const QString &filename);
};

QStringList QtopiaSvgIconPlugin::keys() const
{
    return QStringList() << "svg";
}

QIconEngineV2 *QtopiaSvgIconPlugin::create(const QString &file)
{
    QtopiaSvgIconEngine *engine = new QtopiaSvgIconEngine();
    engine->addFile(file, QSize(), QIcon::Normal, QIcon::On);
    return engine;
}

QTOPIA_EXPORT_QT_PLUGIN(QtopiaSvgIconPlugin)


class QtopiaSvgIconEnginePrivate : public QSharedData
{
public:
    explicit QtopiaSvgIconEnginePrivate()
    {
        render = 0;
        loaded = false;
        pixmaps = 0;
    }
    ~QtopiaSvgIconEnginePrivate()
    {
        delete render;
        render = 0;
    }

    QSvgRenderer *render;
    QString filename;
    bool loaded;
    QMap<QString,QPixmap> *pixmaps;
};

static inline QString createKey(const QString &filename, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    QString key = filename + QString::number(size.width()) + QLatin1String("_")
                    + QString::number(size.height()) + QLatin1String("_")
                    + QString::number(mode) + QString::number(state);

    return key;
}

QtopiaSvgIconEngine::QtopiaSvgIconEngine()
    : d(new QtopiaSvgIconEnginePrivate)
{
}

QtopiaSvgIconEngine::QtopiaSvgIconEngine(const QtopiaSvgIconEngine &other)
    : QIconEngineV2(other), d(new QtopiaSvgIconEnginePrivate)
{
    d->filename = other.d->filename;
    if (other.d->pixmaps)
        d->pixmaps = new QMap<QString,QPixmap>(*other.d->pixmaps);
}

QtopiaSvgIconEngine::~QtopiaSvgIconEngine()
{
}


QSize QtopiaSvgIconEngine::actualSize(const QSize &size, QIcon::Mode,
                                 QIcon::State )
{
    return size;
}


QPixmap QtopiaSvgIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                               QIcon::State state)
{
    QString key = createKey(d->filename, size, mode, state);
    QPixmap pm;

    // Try explicitly added pixmaps first
    if (d->pixmaps) {
        if (d->pixmaps->contains(key))
            return d->pixmaps->value(key);
    }

    // See if we have it in our local cache first.
    if (QPixmapCache::find(key, pm))
        return pm;

    // Perhaps it has already been stored in the global cache.
    bool globalCandidate = false;
    if (size.height() == QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_TabBarIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize)) {
        if (QGlobalPixmapCache::find(key, pm)) {
            qLog(Resource) << "Icon found in global cache" << d->filename;
            return pm;
        }
        globalCandidate = true;
        qLog(Resource) << "Icon not found in global cache" << d->filename;
    }

    if (!d->loaded) {
        if (!d->render)
            d->render = new QSvgRenderer;
        d->render->load(d->filename);
        qLog(Resource) << "loaded svg icon" << d->filename;
        d->loaded = true;
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0x00000000);
    QPainter p(&img);
    d->render->render(&p);
    p.end();
    pm = QPixmap::fromImage(img);
    QStyleOption opt(0);
    opt.palette = QApplication::palette();
    QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
    if (!generated.isNull())
        pm = generated;

    // We'll only put the standard icon sizes in the cache because
    // there's a high likelyhood that they'll be used by others.
    if (globalCandidate) {
        if (QGlobalPixmapCache::insert(key, pm))
            return pm;
    }

    // Still worthwhile putting in the local cache since it is very likely
    // to be rendered again
    QPixmapCache::insert(key, pm);

    return pm;
}


void QtopiaSvgIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                               QIcon::State state)
{
    QString key = createKey(d->filename, pixmap.size(), mode, state);
    if (!d->pixmaps)
        d->pixmaps = new QMap<QString,QPixmap>;
    d->pixmaps->insert(key, pixmap);
}


void QtopiaSvgIconEngine::addFile(const QString &fileName, const QSize &,
                             QIcon::Mode, QIcon::State)
{
    if (!fileName.isEmpty()) {
        QString abs = fileName;
        if (fileName.at(0) != QLatin1Char(':'))
            abs = QFileInfo(fileName).absoluteFilePath();
        d->filename = abs;
    }
}

void QtopiaSvgIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}

QString QtopiaSvgIconEngine::key() const
{
    return QLatin1String("svg");
}

QIconEngineV2 *QtopiaSvgIconEngine::clone() const
{
    return new QtopiaSvgIconEngine(*this);
}

bool QtopiaSvgIconEngine::read(QDataStream &in)
{
    QString fname;
    int num_entries;

    in >> fname;
    d->filename = fname;
    in >> num_entries;
    for (int i=0; i<num_entries; ++i) {
        if (in.atEnd()) {
            if (d->pixmaps)
                d->pixmaps->clear();
            return false;
        }
        QString key;
        QPixmap pixmap;
        in >> pixmap;
        in >> key;
        if (!d->pixmaps)
            d->pixmaps = new QMap<QString,QPixmap>;
        d->pixmaps->insert(key, pixmap);
    }
    return true;
}

bool QtopiaSvgIconEngine::write(QDataStream &out) const
{
    QString fname(d->filename);
    if (fname.at(0) == QLatin1Char(':'))
        fname = QFileInfo(fname).absoluteFilePath();
    out << fname;

    if (d->pixmaps) {
        out << d->pixmaps->size();
        QMapIterator<QString,QPixmap> i(*d->pixmaps);
        while (i.hasNext()) {
            i.next();
            out << i.value();
            out << i.key();
        }
    } else {
        out << int(0);
    }
    return true;
}
