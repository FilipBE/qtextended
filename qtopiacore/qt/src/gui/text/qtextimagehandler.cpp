/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/


#include "qtextimagehandler_p.h"

#include <qapplication.h>
#include <qtextformat.h>
#include <qpainter.h>
#include <qdebug.h>
#include <private/qtextengine_p.h>
#include <qpalette.h>
#include <qtextbrowser.h>
#include <qthread.h>

QT_BEGIN_NAMESPACE

// set by the mime source factory in Qt3Compat
QTextImageHandler::ExternalImageLoaderFunction QTextImageHandler::externalLoader = 0;

static QPixmap getPixmap(QTextDocument *doc, const QTextImageFormat &format)
{
    QPixmap pm;

    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) // auto-detect resources
        name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl::fromEncoded(name.toUtf8());
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Pixmap || data.type() == QVariant::Image) {
        pm = qvariant_cast<QPixmap>(data);
    } else if (data.type() == QVariant::ByteArray) {
        pm.loadFromData(data.toByteArray());
    }

    if (pm.isNull()) {
        QString context;
#ifndef QT_NO_TEXTBROWSER
        QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
        if (browser)
            context = browser->source().toString();
#endif
        QImage img;
        if (QTextImageHandler::externalLoader)
            img = QTextImageHandler::externalLoader(name, context);

        if (img.isNull()) { // try direct loading
            name = format.name(); // remove qrc:/ prefix again
            if (name.isEmpty() || !img.load(name))
                return QPixmap(QLatin1String(":/trolltech/styles/commonstyle/images/file-16.png"));
        }
        pm = QPixmap::fromImage(img);
        doc->addResource(QTextDocument::ImageResource, url, pm);
    }

    return pm;
}

static QSize getPixmapSize(QTextDocument *doc, const QTextImageFormat &format)
{
    QPixmap pm;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        pm = getPixmap(doc, format);
        if (!hasWidth) {
            if (!hasHeight)
                size.setWidth(pm.width());
            else
                size.setWidth(qRound(height * (pm.width() / (qreal) pm.height())));
        }
        if (!hasHeight) {
            if (!hasWidth)
                size.setHeight(pm.height());
            else
                size.setHeight(qRound(width * (pm.height() / (qreal) pm.width())));
        }
    }

    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        extern int qt_defaultDpi();
        if (pm.isNull())
            pm = getPixmap(doc, format);
        if (!pm.isNull())
            scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
    }
    size *= scale;

    return size;
}

static QImage getImage(QTextDocument *doc, const QTextImageFormat &format)
{
    QImage image;

    QString name = format.name();
    if (name.startsWith(QLatin1String(":/"))) // auto-detect resources
        name.prepend(QLatin1String("qrc"));
    QUrl url = QUrl::fromEncoded(name.toUtf8());
    const QVariant data = doc->resource(QTextDocument::ImageResource, url);
    if (data.type() == QVariant::Image) {
        image = qvariant_cast<QImage>(data);
    } else if (data.type() == QVariant::ByteArray) {
        image.loadFromData(data.toByteArray());
    }

    if (image.isNull()) {
        QString context;
#ifndef QT_NO_TEXTBROWSER
        QTextBrowser *browser = qobject_cast<QTextBrowser *>(doc->parent());
        if (browser)
            context = browser->source().toString();
#endif
        if (QTextImageHandler::externalLoader)
            image = QTextImageHandler::externalLoader(name, context);

        if (image.isNull()) { // try direct loading
            name = format.name(); // remove qrc:/ prefix again
            if (name.isEmpty() || !image.load(name))
                return QImage(QLatin1String(":/trolltech/styles/commonstyle/images/file-16.png"));
        }
        doc->addResource(QTextDocument::ImageResource, url, image);
    }

    return image;
}

static QSize getImageSize(QTextDocument *doc, const QTextImageFormat &format)
{
    QImage image;

    const bool hasWidth = format.hasProperty(QTextFormat::ImageWidth);
    const int width = qRound(format.width());
    const bool hasHeight = format.hasProperty(QTextFormat::ImageHeight);
    const int height = qRound(format.height());

    QSize size(width, height);
    if (!hasWidth || !hasHeight) {
        image = getImage(doc, format);
        if (!hasWidth)
            size.setWidth(image.width());
        if (!hasHeight)
            size.setHeight(image.height());
    }

    qreal scale = 1.0;
    QPaintDevice *pdev = doc->documentLayout()->paintDevice();
    if (pdev) {
        extern int qt_defaultDpi();
        if (image.isNull())
            image = getImage(doc, format);
        if (!image.isNull())
            scale = qreal(pdev->logicalDpiY()) / qreal(qt_defaultDpi());
    }
    size *= scale;

    return size;
}

QTextImageHandler::QTextImageHandler(QObject *parent)
    : QObject(parent)
{
}

QSizeF QTextImageHandler::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
    const QTextImageFormat imageFormat = format.toImageFormat();

    if (qApp->thread() != QThread::currentThread())
        return getImageSize(doc, imageFormat);
    return getPixmapSize(doc, imageFormat);
}

void QTextImageHandler::drawObject(QPainter *p, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
    Q_UNUSED(posInDocument)
        const QTextImageFormat imageFormat = format.toImageFormat();

    if (qApp->thread() != QThread::currentThread()) {
        const QImage image = getImage(doc, imageFormat);
        p->drawImage(rect, image, image.rect());
    } else {
        const QPixmap pixmap = getPixmap(doc, imageFormat);
        p->drawPixmap(rect, pixmap, pixmap.rect());
    }
}

QT_END_NAMESPACE
