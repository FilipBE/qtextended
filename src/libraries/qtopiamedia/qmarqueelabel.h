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

#ifndef QMARQUEELABEL_H
#define QMARQUEELABEL_H

#include <QWidget>
#include <qtopiaglobal.h>

class QMarqueeLabelPrivate;

class QTOPIAMEDIA_EXPORT QMarqueeLabel : public QWidget
{
    Q_OBJECT
public:
    QMarqueeLabel( QWidget* parent = 0 );
    ~QMarqueeLabel();

    void setText( const QString& text );
    QString text () const;
    bool isRotating();
    bool canRotate();

    Qt::Alignment alignment () const;
    void setAlignment ( Qt::Alignment );
    virtual QSize sizeHint () const;

    static bool startRotating( QMarqueeLabel *firstpreference, QMarqueeLabel *secondpreference, QMarqueeLabel *thirdpreference=NULL, int msecPerChar=300);

public slots:
    void startRotate(int msecPerChar);
    void startRotate();
    void stopRotate();

signals:
    void rotateFinished();

protected:
    virtual void resizeEvent( QResizeEvent* event );
    virtual void paintEvent ( QPaintEvent * event );
private slots:
    void changeFrame(int frame);

private:
    QMarqueeLabelPrivate *d;

    void drawOutline(QPainter *painter, const QRect &rect, int flags, const QString &text);
    void generateBitmap();
    QColor invertColor(const QColor& input);
};

#endif
