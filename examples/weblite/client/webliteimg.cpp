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

#include "webliteimg.h"
#include <QImageReader>
#include <QTimer>
#include <QThread>
#include <QLabel>
#include <QDebug>

class WebLiteImgPrivate : public QObject
{
    Q_OBJECT
    public:
        QSize scale;
        QPointer<QLabel> label;
        QImage img;
        QPixmap pxm;
        bool autoLoad;
        WebLiteImgPrivate(QObject* o) : QObject(o)
                ,scale(QSize(-1,-1)),autoLoad(true)
        {

        }
    public slots:
        void decode ();
        void decodeDone (const QImage &);

    signals:
        void decoded ();
        void error();
};
void WebLiteImgPrivate::decodeDone (const QImage & i)
{
    img = i;
    sender ()->deleteLater ();
    emit decoded ();

    if (label)
    {
        pxm = QPixmap::fromImage(img);
        label->setPixmap(pxm);
    }

    abort ();
}

void WebLiteImgPrivate::decode ()
{

    if (autoLoad)
    {
        WebLiteClient* client = (WebLiteClient*)sender ();
        QString fn (client->filename());
        if (fn.length()){
            QImageReader reader(fn);
            bool read = true;
            if (scale.width() >0 && scale.height() > 0)
            {
                QSize s = reader.size();
                if (s.width() > scale.width() || s.height() > scale.height())
                {
                    s.scale(scale,Qt::KeepAspectRatio);
                    reader.setScaledSize(s);
                }
            }
            if (read)
                img = reader.read();
            emit decoded ();
        }
        else
        {
            emit error ();
        }
    }
}

WebLiteImg::WebLiteImg(QObject* o) : WebLiteClient(o)
{
    d = new WebLiteImgPrivate(this);
    d->scale = QSize(0,0);
    connect (this, SIGNAL(loaded()), d, SLOT(decode()));
    connect (d, SIGNAL(decoded()), this, SIGNAL(decoded()));
    connect (d, SIGNAL(error()), this, SIGNAL(error()));
}
WebLiteImg::~WebLiteImg()
{
}

void WebLiteImg::setScale (const QSize & s)
{
    d->scale = s;
}
QSize WebLiteImg::scale () const
{
    return d->scale;
}

QImage WebLiteImg::image () const
{
    return d->img;
}

QPixmap WebLiteImg::pixmap () const
{
    if (d->pxm.isNull())
        d->pxm = QPixmap::fromImage(d->img);

    return d->pxm;
}

void WebLiteImg::setLabel (QLabel* lbl)
{
    d->label = lbl;
}
QLabel* WebLiteImg::label ()
{
    return d->label;
}
void WebLiteImg::setAutoLoad (bool al)
{
    d->autoLoad = al;
}

bool WebLiteImg::autoLoad() const
{
    return d->autoLoad;
}

#include "webliteimg.moc"
