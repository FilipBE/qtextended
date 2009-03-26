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
#include "piciconengine.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qstyleoption.h>
#include <qfileinfo.h>
#include <QPicture>
#include <qtopialog.h>
#include <QGlobalPixmapCache>
#include <qiconengineplugin.h>
#include <qstringlist.h>
#include <qiodevice.h>

class QtopiaPicIconPlugin : public QIconEnginePluginV2
{
public:
    QStringList keys() const;
    QIconEngineV2 *create(const QString &filename);
};

QStringList QtopiaPicIconPlugin::keys() const
{
    return QStringList() << "pic";
}

QIconEngineV2 *QtopiaPicIconPlugin::create(const QString &file)
{
    QtopiaPicIconEngine *engine = new QtopiaPicIconEngine();
    engine->addFile(file, QSize(), QIcon::Normal, QIcon::On);
    return engine;
}

QTOPIA_EXPORT_QT_PLUGIN(QtopiaPicIconPlugin)


class QtopiaPicIconEnginePrivate : public QSharedData
{
public:
    explicit QtopiaPicIconEnginePrivate()
    {
        picture = 0;
        loaded = false;
        pixmaps = 0;
    }
    ~QtopiaPicIconEnginePrivate()
    {
        delete picture;
        picture = 0;
    }

    QPicture *picture;
    QString filename;
    bool loaded;
    QMap<QString,QPixmap> *pixmaps;
};

static inline QString createKey(const QString &filename, const QSize &size, QIcon::Mode mode, QIcon::State state, const QColor &col)
{
    QString key;
    key.reserve(filename.length() + 32);
    key += filename;
    key += QString::number(size.width());
    key += QLatin1Char('_');
    key += QString::number(size.height());
    key += QLatin1Char('_');
    key += QString::number(mode | (state << 8));
    key += QString::number(int(col.rgba()));

    return key;
}

QtopiaPicIconEngine::QtopiaPicIconEngine()
    : d(new QtopiaPicIconEnginePrivate)
{
}

QtopiaPicIconEngine::QtopiaPicIconEngine(const QtopiaPicIconEngine &other)
    : QIconEngineV2(other), d(new QtopiaPicIconEnginePrivate)
{
    d->filename = other.d->filename;
    if (other.d->pixmaps)
        d->pixmaps = new QMap<QString,QPixmap>(*other.d->pixmaps);
}

QtopiaPicIconEngine::~QtopiaPicIconEngine()
{
}


QSize QtopiaPicIconEngine::actualSize(const QSize &size, QIcon::Mode,
                                 QIcon::State )
{
    return size;
}


QPixmap QtopiaPicIconEngine::pixmap(const QSize &size, QIcon::Mode mode,
                               QIcon::State state)
{
    QString key = createKey(d->filename, size, mode, state,
                            QApplication::palette().color(QPalette::Highlight));
    QPixmap pm;

    // See if we have it in our local cache first.
    if (QPixmapCache::find(key, pm))
        return pm;

    // Try explicitly added pixmaps
    if (d->pixmaps) {
        if (d->pixmaps->contains(key))
            return d->pixmaps->value(key);
    }

    // Perhaps it has already been stored in the global cache.
    bool globalCandidate = false;
    if (size.height() == QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_TabBarIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize)
        || size.height() == QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize)) {
        if (QGlobalPixmapCache::find(key, pm)) {
            qLog(Resource) << "Icon found in global cache" << d->filename;
            // Put in local cache because we will probably use again soon.
            QPixmapCache::insert(key, pm);
            return pm;
        }
        globalCandidate = true;
        qLog(Resource) << "Icon not found in global cache" << d->filename;
    }

    if (!d->loaded) {
        if (!d->picture)
            d->picture = new QPicture;
        if (!d->picture->load(d->filename))
            qWarning() << "Cannot load icon" << d->filename;
        else
            qLog(Resource) << "loaded pic icon" << d->filename;
        d->loaded = true;
    }

    QImage img(size, QImage::Format_ARGB32_Premultiplied);
    img.fill(0x00000000);
    QPainter p(&img);
    QRectF br = d->picture->boundingRect();
    if (br.width() == 0 || br.height() == 0)
        return QPixmap();
    if (br.width() > 0 && br.height() > 0)
        p.scale(qreal(size.width())/br.width(), qreal(size.height())/br.height());
    p.drawPicture(0, 0, *d->picture);
    p.end();
    pm = QPixmap::fromImage(img);
    QStyleOption opt(0);
    opt.palette = QApplication::palette();
    QPixmap generated = QApplication::style()->generatedIconPixmap(mode, pm, &opt);
    if (!generated.isNull())
        pm = generated;

    // We'll only put the standard icon sizes in the global cache because
    // there's a high likelyhood that they'll be used by others.
    if (globalCandidate)
        QGlobalPixmapCache::insert(key, pm);

    // Still worthwhile putting in the local cache since it is very likely
    // to be rendered again
    QPixmapCache::insert(key, pm);

    return pm;
}


void QtopiaPicIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode,
                               QIcon::State state)
{
    QString key = createKey(d->filename, pixmap.size(), mode, state,
                            QApplication::palette().color(QPalette::Highlight));
    if (!d->pixmaps)
        d->pixmaps = new QMap<QString,QPixmap>;
    d->pixmaps->insert(key, pixmap);
}


void QtopiaPicIconEngine::addFile(const QString &fileName, const QSize &,
                             QIcon::Mode, QIcon::State)
{
    if (!fileName.isEmpty()) {
        QString abs = fileName;
        if (fileName.at(0) != QLatin1Char(':'))
            abs = QFileInfo(fileName).absoluteFilePath();
        d->filename = abs;
    }
}

void QtopiaPicIconEngine::paint(QPainter *painter, const QRect &rect,
                           QIcon::Mode mode, QIcon::State state)
{
    painter->drawPixmap(rect, pixmap(rect.size(), mode, state));
}

QString QtopiaPicIconEngine::key() const
{
    return QLatin1String("pic");
}

QIconEngineV2 *QtopiaPicIconEngine::clone() const
{
    return new QtopiaPicIconEngine(*this);
}

bool QtopiaPicIconEngine::read(QDataStream &in)
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

bool QtopiaPicIconEngine::write(QDataStream &out) const
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
